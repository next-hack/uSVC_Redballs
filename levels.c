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
 * levels.c/h: this defines the levels of the game, and also a function to switch from one level to the other.
 * This might require modifications on each game, and it is not essential for the USVC kernel.
 *
 */
#ifndef LEVELS_H_
#define LEVELS_H_ 
#include "main.h"
const customLevelData_t customLevelData[] =
{
	{
		//.pCustomDataHandler =  customDataHandler,
		.paletteRowRemap = level1_paletteRowRemap,
		.rowPaletteIndexes = titleScreen_rowPaletteIndexes,
		.colorToChangeIndex = titleScreen_colorToChangeIndex,
		.newColorTable = titleScreen_newColorTable,
		.checkpointData = titleScreen_checkpointData,
	},
	{
		//.pCustomDataHandler =  customDataHandler,
		.paletteRowRemap = level1_paletteRowRemap,
		.rowPaletteIndexes = level1_rowPaletteIndexes,
		.colorToChangeIndex = level1_colorToChangeIndex,
		.newColorTable = level1_newColorTable,
		.checkpointData = level1_checkpointData,
		.explosionColors = level1_explosionColors,
		.terminalVelocity = REGULAR_TERMINAL_VELOCITY,
		.gravity = REGULAR_GRAVITY,
		.playerXbounce = REGULAR_PLAYER_X_BOUNCE,
		.playerYbounce = REGULAR_PLAYER_Y_BOUNCE,
		.playerXfriction = REGULAR_PLAYER_X_FRICTION,
		.minSpeedToRollOver = REGULAR_MIN_SPEED_TO_ROLL_OVER,		
	},
	{
		//.pCustomDataHandler =  customDataHandler,
		.paletteRowRemap = level1_paletteRowRemap, // common
		.rowPaletteIndexes = level2_rowPaletteIndexes,
		.colorToChangeIndex = level2_colorToChangeIndex,
		.newColorTable = level2_newColorTable,
		.checkpointData = level2_checkpointData,
		.explosionColors = level2_explosionColors,
		.terminalVelocity = REGULAR_TERMINAL_VELOCITY,
		.gravity = REGULAR_GRAVITY,
		.playerXbounce = ICE_PLAYER_X_BOUNCE,
		.playerYbounce = ICE_PLAYER_Y_BOUNCE,
		.playerXfriction = ICE_PLAYER_X_FRICTION,
		.minSpeedToRollOver = ICE_MIN_SPEED_TO_ROLL_OVER,		
	},
	{
		//.pCustomDataHandler =  customDataHandler,
		.paletteRowRemap = level1_paletteRowRemap, // common
		.rowPaletteIndexes = level3_rowPaletteIndexes,
 		.colorToChangeIndex = level3_colorToChangeIndex,
 		.newColorTable = level3_newColorTable,
 		.checkpointData = level3_checkpointData,
 		.explosionColors = level3_explosionColors,
		.terminalVelocity = REGULAR_TERMINAL_VELOCITY,
		.gravity = REGULAR_GRAVITY,
		.playerXbounce = REGULAR_PLAYER_X_BOUNCE,
		.playerYbounce = REGULAR_PLAYER_Y_BOUNCE,
		.playerXfriction = REGULAR_PLAYER_X_FRICTION,
		.minSpeedToRollOver = REGULAR_MIN_SPEED_TO_ROLL_OVER,
	}	
};
const level_t levels[] =
{
	{
		.pGameMap = (uint16_t*) titleScreenMap,
		.pMetaTiles = titleScreenMapMetaTiles,
		.mapSizeX = MAPSIZEX_TITLESCREEN,
		.mapSizeY = MAPSIZEY_TITLESCREEN,
		.pTiles = &titleScreen_tileData[0][0], // note change this if you are using a different tile set for each level!
		.numberOfTilesToCopyInRam = TITLE_SCREEN_TILES, // note change this if you are using a different tile set for each level!
		.pixelSizeX = TILE_SIZE_X * MAPSIZEX_TITLESCREEN * 2,
		.pixelSizeY = TILE_SIZE_Y * MAPSIZEY_TITLESCREEN * 2,
		.tileSizeX = TILE_SIZE_X * 2,
		.tileSizeY = TILE_SIZE_Y * 2,
		.useMetaTiles = 1,
		.customData = &customLevelData[0],
	},
	{
		.mapSizeX = MAX_MAP_SIZE_X,
		.mapSizeY = MAX_MAP_SIZE_Y,
		.pTiles = &level1_tileData[0][0], // note change this if you are using a different tile set for each level!
		.numberOfTilesToCopyInRam = RAMTILES, // note change this if you are using a different tile set for each level!
		.pixelSizeX = TILE_SIZE_X * MAX_MAP_SIZE_X,
		.pixelSizeY = TILE_SIZE_Y * MAX_MAP_SIZE_Y,
		.tileSizeX = TILE_SIZE_X,
		.tileSizeY = TILE_SIZE_Y,
		.customData = &customLevelData[1],
	},
	{
		.mapSizeX = MAX_MAP_SIZE_X,
		.mapSizeY = MAX_MAP_SIZE_Y,
		.pTiles = &level2_tileData[0][0], // note change this if you are using a different tile set for each level!
		.numberOfTilesToCopyInRam = RAMTILES, // note change this if you are using a different tile set for each level!
		.pixelSizeX = TILE_SIZE_X * MAX_MAP_SIZE_X,
		.pixelSizeY = TILE_SIZE_Y * MAX_MAP_SIZE_Y,
		.tileSizeX = TILE_SIZE_X,
		.tileSizeY = TILE_SIZE_Y,
		.customData = &customLevelData[2],
	},
	{
		.mapSizeX = MAX_MAP_SIZE_X,
		.mapSizeY = MAX_MAP_SIZE_Y,
		.pTiles = &level3_tileData[0][0], // note change this if you are using a different tile set for each level!
		.numberOfTilesToCopyInRam = RAMTILES, // note change this if you are using a different tile set for each level!
		.pixelSizeX = TILE_SIZE_X * MAX_MAP_SIZE_X,
		.pixelSizeY = TILE_SIZE_Y * MAX_MAP_SIZE_Y,
		.tileSizeX = TILE_SIZE_X,
		.tileSizeY = TILE_SIZE_Y,
		.customData = &customLevelData[3],
	},	
	
};

void changeLevel(uint16_t levelNumber)
{
	// This operation might take a while. To avoid displaying rubbish we firstly clear a tile, then we clear VRAM
	memset(&tiles[0], 0, sizeof(tiles[0]));
	for (int i = 0; i < VRAMX * VRAMY; i++)
		vram[i] = (uint32_t) &tiles[0];
	// now the screen is black
	currentLevel = levelNumber;
	videoData.ramTiles = levels[levelNumber].numberOfTilesToCopyInRam;
	#if (GFX_MODE != TILE_MODE2) 
		memcpy(&tiles[0], levels[levelNumber].pTiles, TILE_SIZE_X * TILE_SIZE_Y * videoData.ramTiles );
	#else
		memcpy(&tiles[0], levels[levelNumber].pTiles, TILE_SIZE_X * TILE_SIZE_Y / 2 * videoData.ramTiles);
	#endif
#if LEVELS_CAN_SPECIFY_CUSTOM_ADDITIONAL_DATA && LEVELS_CAN_SPECIFY_CUSTOM_FUNCTION_HANDLER
	if (levels[levelNumber].customDataHasHandlerFunction && levels[levelNumber].customData != NULL)
	{
		((customDataHandler_t*)levels[levelNumber].customData)(levelNumber);		
	}
#endif
}
void changeLevelEx(uint16_t levelNumber, uint32_t reservedWorkTiles)
{
	changeLevel(levelNumber);
	videoData.ramTiles += reservedWorkTiles;
}
int getNumberOfLevels()
{
	return sizeof(levels)/sizeof(level_t);
}
#endif /* LEVELS_H_ */