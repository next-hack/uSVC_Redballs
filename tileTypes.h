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
 * tileTypes.h: this file is (mostly) generated automatically by the uChip Game MapEditor
 */
#ifndef TILETYPES_H_
#define TILETYPES_H_

#include <stdint.h>
#define NUMTILES 97
#define TILE_EMPTY 0
#define TILE_STARTFILL 1
#define TILE_NEUTRAL 2
#define TILE_STOPFILL 3
extern const int8_t tileProps[NUMTILES];



#endif /* TILETYPES_H_ */