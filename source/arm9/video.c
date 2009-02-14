/*---------------------------------------------------------------------------------

	Copyright (C) 2005 - 2009
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)
		Mike Parks (BigRedPimp)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.
	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:
 
	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.
	
---------------------------------------------------------------------------------*/

#include <nds/ndstypes.h>
#include <nds/arm9/video.h>


//---------------------------------------------------------------------------------
u32 vramSetMainBanks(VRAM_A_TYPE a, VRAM_B_TYPE b, VRAM_C_TYPE c, VRAM_D_TYPE d) {
//---------------------------------------------------------------------------------
	uint32 vramTemp = VRAM_CR;

	VRAM_A_CR = VRAM_ENABLE | a;
	VRAM_B_CR = VRAM_ENABLE | b;
	VRAM_C_CR = VRAM_ENABLE | c;
	VRAM_D_CR = VRAM_ENABLE | d;

	return vramTemp;
}



//---------------------------------------------------------------------------------
void vramRestoreMainBanks(u32 vramTemp) {
//---------------------------------------------------------------------------------
  VRAM_CR = vramTemp;
}


//---------------------------------------------------------------------------------
void vramSetBankA(VRAM_A_TYPE a) {
//---------------------------------------------------------------------------------
	VRAM_A_CR = VRAM_ENABLE | a;
}


//---------------------------------------------------------------------------------
void vramSetBankB(VRAM_B_TYPE b) {
//---------------------------------------------------------------------------------
	VRAM_B_CR = VRAM_ENABLE | b;
}


//---------------------------------------------------------------------------------
void vramSetBankC(VRAM_C_TYPE c) {
//---------------------------------------------------------------------------------
	VRAM_C_CR = VRAM_ENABLE | c;
}


//---------------------------------------------------------------------------------
void vramSetBankD(VRAM_D_TYPE d) {
//---------------------------------------------------------------------------------
	VRAM_D_CR = VRAM_ENABLE | d;
}


//---------------------------------------------------------------------------------
void vramSetBankE(VRAM_E_TYPE e) {
//---------------------------------------------------------------------------------
	VRAM_E_CR = VRAM_ENABLE | e;
}


//---------------------------------------------------------------------------------
void vramSetBankF(VRAM_F_TYPE f) {
//---------------------------------------------------------------------------------
	VRAM_F_CR = VRAM_ENABLE | f;
}


//---------------------------------------------------------------------------------
void vramSetBankG(VRAM_G_TYPE g) {
//---------------------------------------------------------------------------------
	VRAM_G_CR = VRAM_ENABLE | g;
}


//---------------------------------------------------------------------------------
void vramSetBankH(VRAM_H_TYPE h) {
//---------------------------------------------------------------------------------
	VRAM_H_CR = VRAM_ENABLE | h;
}


//---------------------------------------------------------------------------------
void vramSetBankI(VRAM_I_TYPE i) {
//---------------------------------------------------------------------------------
	VRAM_I_CR = VRAM_ENABLE | i;
}

//---------------------------------------------------------------------------------
void setBrightness( int screen, int level) {
//---------------------------------------------------------------------------------
	int mode = 1<<14;

	if ( level < 0){
		level = -level;
		mode = 2<<14;
	}
	
	if (level>16) level =16;

	if (screen & 1) REG_MASTER_BRIGHT=(mode | level); 
	if (screen & 2) REG_MASTER_BRIGHT=(mode | level);
}

