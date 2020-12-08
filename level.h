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
#ifndef LEVEL_H_
#define LEVEL_H_
#include <stdint.h>
// Configuration switches
#define NUMBER_OF_LEVELS 1
#define LEVELS_CAN_HAVE_ROM_MAP 1
#define LEVELS_CAN_USE_META_TILE 1
#define LEVELS_CAN_DEFINE_INITIAL_SPRITE_POSITION 0
#define LEVELS_CAN_SPECIFY_CUSTOM_ADDITIONAL_DATA 1
#define LEVELS_CAN_SPECIFY_CUSTOM_FUNCTION_HANDLER 1
//
typedef void customDataHandler_t(int);
typedef struct
{
#if LEVELS_CAN_SPECIFY_CUSTOM_FUNCTION_HANDLER
	customDataHandler_t *pCustomDataHandler;		// the function that will be called to deal with the customData. Since handlers might be different, we will have to specify it every time.
#endif
	// change the following fields according to your requests
	const int16_t *paletteRowRemap;
	const uint8_t *rowPaletteIndexes;
	const uint8_t *colorToChangeIndex;
	const uint8_t *newColorTable;
	const uint16_t *checkpointData;
	const uint16_t *explosionColors;
	const int32_t terminalVelocity;
	const uint16_t gravity;
	const uint16_t playerXbounce;
	const uint16_t playerYbounce;
	const uint16_t playerXfriction;
	const int16_t minSpeedToRollOver;
} customLevelData_t;
extern const customLevelData_t customLevelData[];
//
typedef struct
{
#if LEVELS_CAN_HAVE_ROM_MAP
	const uint16_t *pGameMap;		// the level map
#endif
#if LEVELS_CAN_USE_META_TILE
	const uint16_t *pMetaTiles;	// the 2x2 metaTiles
#endif
#if LEVELS_CAN_DEFINE_INITIAL_SPRITE_POSITION	
	const spritePos_t * pInitialSpritePositions;
#endif
	const uint8_t *pTiles;		// the actual tiles used. This is copied into RAM when switching level
#if LEVELS_CAN_SPECIFY_CUSTOM_ADDITIONAL_DATA
	const void * customData;
#endif	
	uint16_t numberOfTilesToCopyInRam;		// the number of tiles to be copied in ram.
#if LEVELS_CAN_DEFINE_INITIAL_SPRITE_POSITION
	uint16_t numberOfSpritePositions;		// the size of the initialSpritePositions
#endif
	uint16_t mapSizeX;		// in terms of metatiles (or tiles if not using metaTiles)
	uint16_t mapSizeY;		// in terms of metatiles (or tiles if not using metatiles)
	int16_t pixelSizeX;		// horizontal size in terms of pixel (precalculated to save time)
	int16_t pixelSizeY;		// horizontal size in terms of pixel (precalculated to save time)
	int8_t tileSizeX;		// horizontal metaTile size in terms of pixel (precalculated to save time)
	int8_t tileSizeY;		// horizontal metaTile size in terms of pixel (precalculated to save time)
#if LEVELS_CAN_USE_META_TILE 
	uint8_t useMetaTiles;	// 
#endif
#if LEVELS_CAN_SPECIFY_CUSTOM_ADDITIONAL_DATA && LEVELS_CAN_SPECIFY_CUSTOM_FUNCTION_HANDLER
	uint8_t customDataHasHandlerFunction;
#endif
} level_t;
extern const level_t levels[];
extern uint8_t currentLevel;
void changeLevel(uint16_t levelNumber);
void changeLevelEx(uint16_t levelNumber, uint32_t reservedWorkTiles);
int getNumberOfLevels(void);
#endif /* LEVEL_H_ */