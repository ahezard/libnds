#include <nds/disc_io.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <nds/system.h>
#include <nds/arm9/cache.h>

//---------------------------------------------------------------------------------
bool nand_Startup() {
//---------------------------------------------------------------------------------
	if (!isSDAcessible()) return false;

	return true;
}

//---------------------------------------------------------------------------------
bool nand_IsInserted() {
//---------------------------------------------------------------------------------
	if (!isSDAcessible()) return false;

	return true;
}

//---------------------------------------------------------------------------------
bool nand_ReadSectors(sec_t sector, sec_t numSectors,void* buffer) {
//---------------------------------------------------------------------------------
	if (!isSDAcessible()) return false;
	FifoMessage msg;

	DC_FlushRange(buffer,numSectors * 512);

	msg.type = SDMMC_NAND_READ_SECTORS;
	msg.sdParams.startsector = sector;
	msg.sdParams.numsectors = numSectors;
	msg.sdParams.buffer = buffer;

	fifoSendDatamsg(FIFO_SDMMCDSI, sizeof(msg), (u8*)&msg);

	fifoWaitValue32(FIFO_SDMMCDSI);

	int result = fifoGetValue32(FIFO_SDMMCDSI);

	return result == 0;
}

//---------------------------------------------------------------------------------
bool nand_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer) {
//---------------------------------------------------------------------------------
	if (!isSDAcessible()) return false;
	FifoMessage msg;

	DC_FlushRange(buffer,numSectors * 512);

	msg.type = SDMMC_NAND_WRITE_SECTORS;
	msg.sdParams.startsector = sector;
	msg.sdParams.numsectors = numSectors;
	msg.sdParams.buffer = (void*)buffer;

	fifoSendDatamsg(FIFO_SDMMCDSI, sizeof(msg), (u8*)&msg);

	fifoWaitValue32(FIFO_SDMMCDSI);

	int result = fifoGetValue32(FIFO_SDMMCDSI);

	return result == 0;
}


//---------------------------------------------------------------------------------
bool nand_ClearStatus() {
//---------------------------------------------------------------------------------
	if (!isSDAcessible()) return false;
	return true;
}

//---------------------------------------------------------------------------------
bool nand_Shutdown() {
//---------------------------------------------------------------------------------
	if (!isSDAcessible()) return false;
	return true;
}

/*const DISC_INTERFACE __io_dsisd = {
	DEVICE_TYPE_DSI_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&sdio_Startup,
	(FN_MEDIUM_ISINSERTED)&sdio_IsInserted,
	(FN_MEDIUM_READSECTORS)&sdio_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&sdio_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&sdio_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&sdio_Shutdown
};
*/