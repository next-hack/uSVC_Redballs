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
 * fixedSectionMap.h/c: this file is (mostly) generated automatically by the uChip Game MapEditor
 * Specifies how the tiles should  be placed in the fixed section.
 */
#ifndef FIXEDSECTIONMAP_H_
#define FIXEDSECTIONMAP_H_
#include <stdint.h>
#define FIXED_SECTION_MAPSIZEX 40
#define FIXED_SECTION_MAPSIZEY 3
#define MAX_FIXED_SECTION_TEXT 16	// maximum length of text we can copy 
#define FIXED_SECTION_FIXED_TILES 11	// number of tiles that will always be used.
#define FIXED_SECTION_WEAPON_TILES 4
#define FIXED_SECTION_TILE_EMPTY 1
#define FIXED_SECTION_TILE_FULL 10
#define FIXED_SECTION_TILE_ONE_LEFT 11
#define FIRING_FORCE_BAR_LENGTH 10
#define FIRST_FIRING_FORCE_BAR_TILE 3
#define FIXED_SECTION_POWER_TILE (FIXED_SECTION_FIXED_TILES + FIXED_SECTION_WEAPON_TILES)
#define FIXED_SECTION_WIND_TILE (FIXED_SECTION_POWER_TILE + 1)
#define FIXED_SECTION_HEALTH_RED_TILE (FIXED_SECTION_WIND_TILE + 1)
#define FIXED_SECTION_HEALTH_BLUE_TILE (FIXED_SECTION_HEALTH_RED_TILE + 1)
#define FIXED_SECTION_HEALTH_YELLOW_TILE (FIXED_SECTION_HEALTH_BLUE_TILE + 1)
#define FIXED_SECTION_HEALTH_GRAY_TILE (FIXED_SECTION_HEALTH_YELLOW_TILE + 1)
#define FIXED_SECTION_TILE_TEXT_OFFSET (FIXED_SECTION_HEALTH_GRAY_TILE + 1)
#define FIRST_LEFT_WIND_BAR_TILE 3
#define FIRST_RIGHT_WIND_BAR_TILE 9
#define WIND_BAR_LENGTH 4
#define FIXED_SECTION_TILE_ONE_RIGHT 18
#define HEALTH_BAR_LENGTH 10
#define FIRING_FORCE_Y 1
#define WIND_FORCE_Y 2
#define MAX_FIXED_SECTION_TILES (FIXED_SECTION_TILE_TEXT_OFFSET + MAX_FIXED_SECTION_TEXT)
extern uint8_t fixedSectionMap[FIXED_SECTION_MAPSIZEX * FIXED_SECTION_MAPSIZEY]  __attribute__ ((aligned (4)));
extern const uint8_t fixedSectionRomMap[FIXED_SECTION_MAPSIZEX * FIXED_SECTION_MAPSIZEY];
extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][8] __attribute__ ((aligned (4)));



#endif /* FIXEDSECTIONMAP_H_ */