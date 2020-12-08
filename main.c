/*
 *  RedBalls (not in the UrbanDictionary definintion!): 
 *  A clone of the popular Worms game, runnin on uChip Simple VGA Console
 *  a simple Cortex M0+ console with 32 kB of RAM!
 *
 *  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
 *
 *  This file is part of next-hack's RedBalls.
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program  is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *	
 *  tl;dr 
 *  Do whatever you want, this program is free! Though we won't 
 *  reject donations https://next-hack.com/index.php/donate/ :) 
 *
 */
#include "main.h"
#define NO_USB 0			// 1 for testing
#if RAMTILES > MAX_TILES 
	#error you must adjust the number of MAX_TILES in VGA.h
#endif
#include <stdlib.h>
uint8_t currentLevel = 0;
static uint16_t g_frame = 0;		
uint16_t getFrameTicks(void)
{
	return g_frame;
}
void randomize(void)
{
	srand(millis());
}
 int main(void) 
{
	// NOTE! The videoData structure must be correctly initialized BEFORE Calling initUsvc!
	videoData.pPaletteIdx = (uint8_t*) level1_rowPaletteIndexes;
	videoData.pNewColorChangeTable = (uint8_t*) level1_newColorTable;
	videoData.pNewColorChangeIndexTable = (uint8_t*) level1_colorToChangeIndex;
	videoData.pPaletteRemappingRowOffsets = (int16_t*) level1_paletteRowRemap;
	// Only if you use row remapping videoData.ptrRowRemapTable = (uint8_t*) rowRemapTable; where rowRemapTable is 208-bytes array (in flash or ram) of uint8_t.
	// Only if you use per-line xScroll: videoData.ptrRowXScroll = xScrollTable; where xScrollTable is an array stored in flash o ram.	
	videoData.ptrTileXScroll = xScrollTileTable;
	memset(xScrollTileTable, 0, sizeof(xScrollTileTable));
	initUsvc(patches);
	setLed(1);
	uint32_t lastTime = millis();
	int testLed = 0;
#if !NO_USB	
	usbSetKeyboardInstallationCompleteCallback(randomize);
#endif
	uint16_t keyBuffer[6] = {0, 0, 0, 0, 0, 0};
	gameInit();
	while (1)
	{
		uint32_t timeNow = millis();
		waitForVerticalBlank();
		soundEngine();			// called here to be sure it will be called at an almost constant position		
		if (timeNow - lastTime > 1000UL)
		{
			setLed(testLed);
			testLed = 1 - testLed;
			lastTime = timeNow;
		}
		g_frame++;
		removeAllSprites(1);			// remove the sprites and redraw the screen (updating vram).
		gameLoop(keyBuffer, 0);
		drawSprites();
		#if DEBUG_TIME
			gLine = getCurrentScanLineNumber();
		#endif
		// finally, redraw the fixed section
		#if USE_SECTION != NO_FIXED_SECTION
			drawFixedSection();
		#endif
		// now, we leave the remaining time for the AI and USB
		gameLoop(NULL, 1);
		do
		{
			#if !NO_USB
			usbHostTask();
			if (usbHidBootKeyboardIsInstalled())
			{
				usbKeyboardPoll();
				usbGetCurrentAsciiKeyboardStateEx(keyBuffer);
			}
			if (usbHidGenericGamepadIsInstalled())
			{
				gamePadState_t gps;
				usbHidGenericGamepadPoll();
				// gamepad to keyboard conversion
				getCurrentGamepadState(&gps);
				//
				keyBuffer[0] = 0;
				uint32_t buttons = gps.buttons;
				// if R2 is pressed, then move the camera
				if (buttons & GP_BUTTON_R2)
				{
					uint8_t k = 0;
					if (gps.axes[1] == gps.XYZRxMinimum)
						keyBuffer[k++] = USB_KEY_UP << 8;
					else if (gps.axes[1] == gps.XYZRxMaximum)
						keyBuffer[k++] = USB_KEY_DOWN << 8;
					if (gps.axes[0] == gps.XYZRxMinimum)
						keyBuffer[k++] = USB_KEY_LEFT << 8;
					else if (gps.axes[0] == gps.XYZRxMaximum)
						keyBuffer[k++] = USB_KEY_RIGHT << 8;
					//
				}
				else
				{
					uint8_t k = 0;
					if (gps.axes[1] == gps.XYZRxMinimum)
						keyBuffer[k++] = 'w';
					else if (gps.axes[1] == gps.XYZRxMaximum)
						keyBuffer[k++] = 's';
					if (gps.axes[0] == gps.XYZRxMinimum)
						keyBuffer[k++] = 'a';
					else if (gps.axes[0] == gps.XYZRxMaximum)
						keyBuffer[k++] = 'd';	
					uint8_t key = 0;	
					switch (buttons)
					{
						case GP_BUTTON_1:
							key = 'y';
							break;
						case GP_BUTTON_2:
							key = 'f';
							break;
						case GP_BUTTON_3:
							key = 'g';
							break;
						case GP_BUTTON_4:	// ESC
							key = ASCII_ESCAPE;
							break;
						case GP_BUTTON_L2:
							// change grenade timing
							key = 'z';
							break;
						case GP_BUTTON_L1:
							// weapon up
							key = '+';
							break;
						case GP_BUTTON_R1:
							// weapon down
							key = '-';
							break;
					}
					keyBuffer[k] = key;		
				}	
			}			
			#endif
		} while (videoData.currentLineNumber < 523);
	}
}