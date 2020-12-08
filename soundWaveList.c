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
 * soundWaveList.h/c: these files defines an array of PCM samples, specifying for each one with its original sampling frequency.
 */
#include "main.h"
// PUT HERE ALL YOUR SOUNDWAVES. 
const soundWave_t soundWaves[] = 
{ 
	{
		.length = 256,
		.wData = (int8_t *)  sineWave,
		.sps = 30000
	},
	{
		.length = 256,
		.wData = (int8_t *)  sawToothWave,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  triangleWave,
		.sps = 30000
	},
	{
		.length = 256,
		.wData = (int8_t *)  squareWave25,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  squareWave50,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  squareWave75,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  sineDistoWave1,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  sineDistoWave2,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  sineDistoWave3,
		.sps = 30000
	},	
	{
		.length = 256,
		.wData = (int8_t *)  squareWave50Filtered,
		.sps = 30000
	},	
	{
		.length = AK47_NUM_ELEMENTS,
		.wData =(int8_t *) ak47_data,
		.sps = AK47_SPS
	},
	{
		.length = BAZOOKA_NUM_ELEMENTS,
		.wData =(int8_t *) bazooka_data,
		.sps = BAZOOKA_SPS
	},
	{
		.length = BLOWTORCH_NUM_ELEMENTS,
		.wData =(int8_t *) blowtorch_data,
		.sps = BLOWTORCH_SPS
	},
	{
		.length = BOING_NUM_ELEMENTS,
		.wData =(int8_t *) boing_data,
		.sps = BOING_SPS
	},
	{
		.length = DRILL_NUM_ELEMENTS,
		.wData =(int8_t *) drill_data,
		.sps = DRILL_SPS
	},
	{
		.length = EXPLOSION_NUM_ELEMENTS,
		.wData =(int8_t *) explosion_data,
		.sps = EXPLOSION_SPS
	},	
	{
		.length = FUSE_NUM_ELEMENTS,
		.wData =(int8_t *) fuse_data,
		.sps = FUSE_SPS
	},
	{
		.length = GRENADEBOUNCE_NUM_ELEMENTS,
		.wData =(int8_t *) grenadeBounce_data,
		.sps = FUSE_SPS
	},
	{
		.length = MINEBOUNCE_NUM_ELEMENTS,
		.wData =(int8_t *) mineBounce_data,
		.sps = MINEBOUNCE_SPS
	},	
	{
		.length = RELOAD_NUM_ELEMENTS,
		.wData =(int8_t *) reload_data,
		.sps = RELOAD_SPS
	},	
	{
		.length = SHOTGUN_NUM_ELEMENTS,
		.wData =(int8_t *) shotgun_data,
		.sps = SHOTGUN_SPS
	},
	{
		.length = WATER_NUM_ELEMENTS,
		.wData =(int8_t *) water_data,
		.sps = WATER_SPS
	},	
	{
		.length = POP_NUM_ELEMENTS,
		.wData =(int8_t *) pop_data,
		.sps = POP_SPS
	},	
	{
		.length = THROWGRENADE_NUM_ELEMENTS,
		.wData =(int8_t *) throwGrenade_data,
		.sps = THROWGRENADE_SPS
	},	
};

int getNumberOfSoundWaves()
{
	return sizeof(soundWaves) / sizeof (soundWave_t);
}