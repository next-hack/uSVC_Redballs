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
 * soundData.h/c: these files defines the actual PCM samples. 
 * I used  WAVToCode (Copyright (C) 2018-2020 C. J. Seymour)
 * to create the arrays. 
 */
#ifndef SOUNDDATA_H_
#define SOUNDDATA_H_
#include <stdint.h>
#define AK47_NUM_ELEMENTS 1268
#define AK47_SPS 30000
#define BAZOOKA_NUM_ELEMENTS 3387
#define BAZOOKA_SPS 6000
#define BLOWTORCH_NUM_ELEMENTS 2955
#define BLOWTORCH_SPS 8000
#define BOING_NUM_ELEMENTS 4418
#define BOING_SPS 8000
#define DRILL_NUM_ELEMENTS 798
#define DRILL_SPS 14000
#define EXPLOSION_NUM_ELEMENTS 3889
#define EXPLOSION_SPS 8000
#define FUSE_NUM_ELEMENTS 1970
#define FUSE_SPS 30000
#define RELOAD_NUM_ELEMENTS 4609
#define RELOAD_SPS 8000
#define SHOTGUN_NUM_ELEMENTS 3801
#define SHOTGUN_SPS 8000
#define WATER_NUM_ELEMENTS 2714
#define WATER_SPS 6000
#define MINEBOUNCE_NUM_ELEMENTS 275
#define MINEBOUNCE_SPS 8000
#define GRENADEBOUNCE_NUM_ELEMENTS 25
#define GRENADEBOUNCE_SPS 30000
#define POP_NUM_ELEMENTS 1374
#define POP_SPS 12000
#define THROWGRENADE_NUM_ELEMENTS 1161
#define THROWGRENADE_SPS 15000
extern const char throwGrenade_data[];
extern const char pop_data[];
extern const char ak47_data[];
extern const char bazooka_data[];
extern const char blowtorch_data[];
extern const char boing_data[];
extern const char drill_data[];
extern const char explosion_data[];
extern const char fuse_data[];
extern const char reload_data[];
extern const char shotgun_data[];
extern const char water_data[];
extern const char mineBounce_data[];
extern const char grenadeBounce_data[];
extern const uint8_t midTrack[];




#endif /* SOUNDDATA_H_ */