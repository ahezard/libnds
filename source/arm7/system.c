/*---------------------------------------------------------------------------------

  Copyright (C) 2005 - 2010
    Michael Noland (joat)
	Jason Rogers (Dovoto)
	Dave Murphy (WinterMute)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1.  The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.
  2.  Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.
  3.  This notice may not be removed or altered from any source
      distribution.

---------------------------------------------------------------------------------*/
#include <nds/ndstypes.h>
#include <nds/system.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <nds/interrupts.h>
#include <nds/bios.h>
#include <nds/card.h>
#include <nds/arm7/clock.h>
#include <nds/arm7/sdmmc.h>
#include <nds/arm7/i2c.h>

bool sleepIsEnabled = true;

bool __dsimode = false;

//---------------------------------------------------------------------------------
void powerValueHandler(u32 value, void* user_data) {
//---------------------------------------------------------------------------------
	u32 temp;
	u32 ie_save;
	int battery, backlight, power;

	switch(value & 0xFFFF0000) {
		//power control
	case PM_REQ_LED:
		ledBlink(value);
		break;
	case PM_REQ_ON:
		temp = readPowerManagement(PM_CONTROL_REG);
		writePowerManagement(PM_CONTROL_REG, temp | (value & 0xFFFF));
		break;
	case PM_REQ_OFF:
		temp = readPowerManagement(PM_CONTROL_REG) & (~(value & 0xFFFF));
		writePowerManagement(PM_CONTROL_REG, temp);
		break;

	case PM_REQ_SLEEP:
			
		ie_save = REG_IE;
		// Turn the speaker down.
		if (REG_POWERCNT & 1) swiChangeSoundBias(0,0x400);
		// Save current power state.
		power = readPowerManagement(PM_CONTROL_REG);
		// Set sleep LED.
		writePowerManagement(PM_CONTROL_REG, PM_LED_CONTROL(1));
		// Register for the lid interrupt.
		REG_IE = IRQ_LID;

		// Power down till we get our interrupt.
		swiSleep(); //waits for PM (lid open) interrupt

		//100ms
		swiDelay(838000);
		
		// Restore the interrupt state.
		REG_IE = ie_save;

		// Restore power state.
		writePowerManagement(PM_CONTROL_REG, power);

		// Turn the speaker up.
		if (REG_POWERCNT & 1) swiChangeSoundBias(1,0x400); 

		// update clock tracking
		resyncClock();
		break;

	case PM_REQ_SLEEP_DISABLE:
		sleepIsEnabled = false;
		break;

	case PM_REQ_SLEEP_ENABLE:
		sleepIsEnabled = true;
		break;
	case PM_REQ_BATTERY:
		if (!__dsimode) {
			battery = (readPowerManagement(PM_BATTERY_REG) & 1)?3:15;
			backlight = readPowerManagement(PM_BACKLIGHT_LEVEL);
			if (backlight & (1<<6)) battery += (backlight & (1<<3))<<4;
		} else {
			battery = i2cReadRegister(I2C_PM,I2CREGPM_BATTERY);
		}
		fifoSendValue32(FIFO_PM, battery);
		break;
	case PM_DSI_HACK:
		__dsimode = true;
		__touch_dsimode = true;
		break;
	case PM_DSI_TOUCHDSMODE:
		__touch_dsimode = false;
		break;
	}
}

//---------------------------------------------------------------------------------
void sdmmcDsiValueHandler(u32 value, void* user_data) {
//---------------------------------------------------------------------------------
    int result = 0;

    int oldIME = enterCriticalSection();

    switch(value) {

    case SDMMC_HAVE_SD:
        result = sdmmc_read16(REG_SDSTATUS0);
        break;

    case SDMMC_SD_START:
        if (sdmmc_read16(REG_SDSTATUS0) == 0) {
            result = 1;
        } else {
            sdmmc_controller_init();
            result = sdmmc_sdcard_init();
        }
        break;
		
	case SDMMC_NAND_START:
		// disable nand start by default until no$gba handle it correctly
        if (sdmmc_read16(REG_SDSTATUS0) == 0) {
            result = 1;
        } else {
            sdmmc_controller_init();
            result = sdmmc_nand_init();
        }
        break;

    case SDMMC_SD_IS_INSERTED:
        result = sdmmc_cardinserted();
        break;

    case SDMMC_SD_STOP:
        break;
		
	case DSI_RESET_SLOT_1:
		result = dsi_resetSlot1();
        break;

	case DSI_LOCK_SCFG_ARM7:
		result = dsi_lockScfgARM7();
        break;

	case DSI_SWITCH_TO_DS_MODE:
		result = dsi_switchToDsMode();
        break;
    }

    leaveCriticalSection(oldIME);

    fifoSendValue32(FIFO_SDMMCDSI, result);
}

//---------------------------------------------------------------------------------
void systemSleep(void) {
//---------------------------------------------------------------------------------
	if(!sleepIsEnabled) return;
	//puts arm9 to sleep which then notifies arm7 above (which causes arm7 to sleep)
	fifoSendValue32(FIFO_SYSTEM, PM_REQ_SLEEP);
}

//---------------------------------------------------------------------------------
int sleepEnabled(void) {
//---------------------------------------------------------------------------------
	return sleepIsEnabled;
}

//---------------------------------------------------------------------------------
u32 dsi_resetSlot1() {
//---------------------------------------------------------------------------------
	int backup=REG_SCFG_EXT;
	REG_SCFG_EXT = 0x82050100;

	// Power Off Slot
	while((REG_SCFG_MC&0x0C) !=  0x0C); // wait until state<>3
	if((REG_SCFG_MC&0x0C) != 0x08) return 1; // exit if state<>2      
	
	REG_SCFG_MC = 0x0C; // set state=3 
	while((REG_SCFG_MC&0x0C) !=  0x00); // wait until state=0

	// Power On Slot
	while((REG_SCFG_MC&0x0C) !=  0x0C); // wait until state<>3
	if((REG_SCFG_MC&0x0C) != 0x00) return 1; //  exit if state<>0
	
	REG_SCFG_MC = 0x04; // set state=1
	while((REG_SCFG_MC&0x0C) != 0x04); // wait until state=1
	
	REG_SCFG_MC = 0x08; // set state=2      
	while((REG_SCFG_MC&0x0C) != 0x08); // wait until state=2
	
	REG_ROMCTRL = 0x20000000; // set ROMCTRL=20000000h
	
	while((REG_ROMCTRL&0x8000000) != 0x8000000); // wait until ROMCTRL.bit31=1

	REG_SCFG_EXT=backup;
	
	return 0;
}

u32 dsi_lockScfgARM7() {
	// REG_SCFG_EXT.bit31=0 lock SCFG
	REG_SCFG_EXT &= ~0x80000000;	
	return 0;
}

u32 dsi_switchToDsMode() {
	// REG_SCFG_ROM (ARM7)
	// 0x703 : NTR (DS)
	// 0x701 : TWL (DSI)
	REG_SCFG_ROM = 0x703;

	// REG_SCFG_CLK (ARM7)
	// 0x0180 : NTR (DS)
	// 0x0187 : TWL (DSI)
	REG_SCFG_CLK = 0x0180;
	
	// REG_SCFG_EXT (ARM7)
	REG_SCFG_EXT = 0x12A00000;
	
	return 0;
}

void sdmmcMsgHandler(int bytes, void *user_data);
void sdmmcValueHandler(u32 value, void* user_data);
void firmwareMsgHandler(int bytes, void *user_data);

//---------------------------------------------------------------------------------
void installSystemFIFO(void) {
//---------------------------------------------------------------------------------

	fifoSetValue32Handler(FIFO_PM, powerValueHandler, 0);
	fifoSetValue32Handler(FIFO_SDMMCDSI, sdmmcDsiValueHandler, 0);
	fifoSetDatamsgHandler(FIFO_SDMMCDSI, sdmmcMsgHandler, 0);
	fifoSetDatamsgHandler(FIFO_FIRMWARE, firmwareMsgHandler, 0);
	
}


