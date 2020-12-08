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
 * titleScreenMap.h: this file is (mostly) generated automatically by the uChip Game MapEditor
 */
#ifndef TITLESCREENMAP_H_
#define TITLESCREENMAP_H_
#include <stdint.h>
#define MAPSIZEX_TITLESCREEN 20
#define MAPSIZEY_TITLESCREEN 4
#define NUMBER_OF_METATILES_TITLESCREEN 43
extern const uint16_t titleScreenMap[MAPSIZEY_TITLESCREEN * MAPSIZEX_TITLESCREEN];
extern const uint16_t titleScreenMapMetaTiles[4 * NUMBER_OF_METATILES_TITLESCREEN];
#endif /* TITLESCREENMAP_H_ */