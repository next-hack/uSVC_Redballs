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
 *
 * paletteRemapData.h/c: this file is (mostly) generated automatically by the uChip Game MapEditor
 * It permits to use the palette/color remapping feautre of 4BPP TILEMODE2, to create the 
 * background shades, water effect, and to change the color of the clouds's shadows.
 */
#ifndef PALETTEREMAPDATA_H_
#define PALETTEREMAPDATA_H_

#include <stdint.h>
// Copy here all the level color data declarations.
#define TITLESCREEN_NUMBER_OF_PALETTES 2
#define TITLESCREEN_COLOR_REMAP_ROWS 756
#define TITLESCREEN_NUMBER_OF_REMAP_CHECKPOINTS 23
#define LEVEL1_NUMBER_OF_PALETTES 2
#define LEVEL1_COLOR_REMAP_ROWS 756
#define LEVEL1_NUMBER_OF_REMAP_CHECKPOINTS 23 
#define LEVEL2_NUMBER_OF_PALETTES 2
#define LEVEL2_COLOR_REMAP_ROWS 756
#define LEVEL2_NUMBER_OF_REMAP_CHECKPOINTS 23
#define LEVEL3_NUMBER_OF_PALETTES 2
#define LEVEL3_COLOR_REMAP_ROWS 756
#define LEVEL3_NUMBER_OF_REMAP_CHECKPOINTS 23
extern const uint16_t titleScreen_checkpointData[TITLESCREEN_NUMBER_OF_REMAP_CHECKPOINTS * 16 *TITLESCREEN_NUMBER_OF_PALETTES];
extern const uint8_t titleScreen_rowPaletteIndexes[TITLESCREEN_COLOR_REMAP_ROWS];
extern const uint8_t titleScreen_colorToChangeIndex[TITLESCREEN_COLOR_REMAP_ROWS];
extern const uint8_t titleScreen_newColorTable[TITLESCREEN_COLOR_REMAP_ROWS];
extern const uint16_t level1_explosionColors[16];
extern const uint16_t level1_checkpointData[LEVEL1_NUMBER_OF_REMAP_CHECKPOINTS * 16 * 2];
extern const uint8_t level1_rowPaletteIndexes[LEVEL1_COLOR_REMAP_ROWS];
extern const uint8_t level1_colorToChangeIndex[LEVEL1_COLOR_REMAP_ROWS];
extern const uint8_t level1_newColorTable[LEVEL1_COLOR_REMAP_ROWS];
extern const int16_t level1_paletteRowRemap[];
extern const uint16_t level2_checkpointData[LEVEL1_NUMBER_OF_REMAP_CHECKPOINTS * 16 * LEVEL1_NUMBER_OF_PALETTES];
extern const uint8_t level2_rowPaletteIndexes[LEVEL2_COLOR_REMAP_ROWS];
extern const uint8_t level2_colorToChangeIndex[LEVEL2_COLOR_REMAP_ROWS];
extern const uint8_t level2_newColorTable[LEVEL2_COLOR_REMAP_ROWS];
extern const uint16_t level2_explosionColors[16];
extern const uint16_t level3_checkpointData[LEVEL3_NUMBER_OF_REMAP_CHECKPOINTS * 16 *LEVEL3_NUMBER_OF_PALETTES];
extern const uint8_t level3_rowPaletteIndexes[LEVEL3_COLOR_REMAP_ROWS];
extern const uint8_t level3_colorToChangeIndex[LEVEL3_COLOR_REMAP_ROWS];
extern const uint8_t level3_newColorTable[LEVEL3_COLOR_REMAP_ROWS];
extern const uint16_t level3_explosionColors[16];

#endif /* PALETTEREMAPDATA_H_ */