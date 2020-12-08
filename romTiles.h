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
 * romTiles.h/c: this file is (mostly) generated automatically by the uChip Game MapEditor
 * All the tile graphics data for all the level are defined here.
 */
#ifndef ROMTILES_H_
#define ROMTILES_H_
#include <stdint.h>
#define MAXTILEINDEX 97
#define RAMTILES (MAXTILEINDEX) // 
#define TITLE_SCREEN_TILES 142
#define TILESIZEX 8
#define TILESIZEY 8
extern const uint8_t titleScreen_tileData[TITLE_SCREEN_TILES][TILESIZEX * TILESIZEY / 2 ];
extern const uint8_t level1_tileData[MAXTILEINDEX][TILESIZEX * TILESIZEY/2 ];
extern const uint8_t level2_tileData[MAXTILEINDEX][TILESIZEX * TILESIZEY/2 ];
extern const uint8_t level3_tileData[MAXTILEINDEX][TILESIZEX * TILESIZEY / 2 ];
#endif /* ROMTILES_H_ */