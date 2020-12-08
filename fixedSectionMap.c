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
#include "fixedSectionMap.h"
uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][8] __attribute__ ((aligned (4)));
uint8_t fixedSectionMap[FIXED_SECTION_MAPSIZEX * FIXED_SECTION_MAPSIZEY]  __attribute__ ((aligned (4)));
const uint8_t fixedSectionRomMap[FIXED_SECTION_MAPSIZEX * FIXED_SECTION_MAPSIZEY] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, FIXED_SECTION_TILE_TEXT_OFFSET, FIXED_SECTION_TILE_TEXT_OFFSET + 1, FIXED_SECTION_TILE_TEXT_OFFSET + 2, FIXED_SECTION_TILE_TEXT_OFFSET + 3,
	FIXED_SECTION_TILE_TEXT_OFFSET + 4, FIXED_SECTION_TILE_TEXT_OFFSET + 5, FIXED_SECTION_TILE_TEXT_OFFSET + 6, FIXED_SECTION_TILE_TEXT_OFFSET + 7, FIXED_SECTION_TILE_TEXT_OFFSET + 8, FIXED_SECTION_TILE_TEXT_OFFSET + 9, FIXED_SECTION_TILE_TEXT_OFFSET + 10, FIXED_SECTION_TILE_TEXT_OFFSET + 11,
	FIXED_SECTION_TILE_TEXT_OFFSET + 12, FIXED_SECTION_TILE_TEXT_OFFSET + 13, FIXED_SECTION_TILE_TEXT_OFFSET + 14, FIXED_SECTION_TILE_TEXT_OFFSET + 15, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x000B, 0x000C, 0x0005, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0004, 0x0006, 0x0005,
	0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0004, 0x0007, 0x0005, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0004,
	0x000D, 0x000E, 0x0005, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002,
	0x0003, 0x0001, 0x0001, 0x0001, 0x0001, 0x0004, 0x0008, 0x0005,
	0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0004, 0x0009, 0x0005, 0x0001, 0x0001, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0004
};