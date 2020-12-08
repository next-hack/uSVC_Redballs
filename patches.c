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
 * patches.h/c: all the patches (sounds) used in the game.
 */
#include "main.h" 
const char patch00[]  =
{
	0, PC_ENV_VOL,0,
	0, PC_ENV_SPEED, 16,
	0, PC_WAVE,0,
	4, PC_ENV_VOL, 0xff,
	10, PC_ENV_SPEED, -4,
	0x50, PC_NOTE_CUT,0,
	0,PATCH_END
};
const char patch_play_specified_sps[]  =
{
	0,PC_SAMPLE_RATE_SPECIFY, 1,
	57,PATCH_END
};
const char teleportPatch[]  =
{
	0,PC_ENV_VOL, 0,
	35,PC_ENV_VOL, 255,
	0,PC_WAVE, 9,
	0, PC_PITCH, 40,
	0, PC_TREMOLO_LEVEL, 255,
	0, PC_TREMOLO_RATE, 2,
	90, PC_ENV_SPEED, -20,
	0,PATCH_END
};
const char medipackPatch[] =
{
	0, PC_ENV_VOL, 255,
	0, PC_WAVE, 0,
	0, PC_PITCH, 50,
	0, PC_TREMOLO_RATE, 40,
	0, PC_TREMOLO_LEVEL, 255,
	1, PC_LOOP_START, 25,
	0, PC_NOTE_UP, 1,
	1, PC_LOOP_END, 0,
	1, PC_NOTE_CUT, 0,
	0, PATCH_END 
};
const char offSidePatch[] =
{
	0, PC_ENV_VOL, 255,
	0, PC_WAVE, 0,
	0, PC_PITCH, 75,
	0, PC_TREMOLO_RATE, 40,
	0, PC_TREMOLO_LEVEL, 255,
	1, PC_LOOP_START, 25,
	0, PC_NOTE_DOWN, 1,
	1, PC_LOOP_END, 0,
	1, PC_NOTE_CUT, 0,
	0, PATCH_END
};
const char enterMenuPatch[] =
{
	0, PC_ENV_VOL, 255,
	0, PC_WAVE, 0,
	0, PC_PITCH, 60,
	5, PC_NOTE_UP, 12,
	5, PC_NOTE_CUT, 0,
	0, PATCH_END
};
const char exitMenuPatch[] =
{
	0, PC_ENV_VOL, 255,
	0, PC_WAVE, 0,
	0, PC_PITCH, 72,
	5, PC_NOTE_DOWN, 12,
	5, PC_NOTE_CUT, 0,
	0, PATCH_END
};
const char moveMenuPatch[] =
{
	0, PC_ENV_VOL, 255,
	0, PC_WAVE, 0,
	0, PC_PITCH, 68,
	5, PC_ENV_VOL, 0,
	2, PC_ENV_VOL, 255,
	5, PC_NOTE_CUT, 0,
	0, PATCH_END
};
const  patch_t patches[]  =
{
	{0, 0, (uint8_t*) patch00, 256, 256},
	{0, 0, (uint8_t*) patch00, 256, 256},
	{0, 0, (uint8_t*) patch00, 256, 256},
	{0, 0, (uint8_t*) patch00, 256, 256},
	{2, DEFAULT_SOUND_NUMBER + 0, (uint8_t*) patch_play_specified_sps, AK47_NUM_ELEMENTS - 1, AK47_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 1, (uint8_t*) patch_play_specified_sps, BAZOOKA_NUM_ELEMENTS - 1, BAZOOKA_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 2, (uint8_t*) patch_play_specified_sps, 0, BLOWTORCH_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 3, (uint8_t*) patch_play_specified_sps, BOING_NUM_ELEMENTS - 1, BOING_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 4, (uint8_t*) patch_play_specified_sps, 0, DRILL_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 5, (uint8_t*) patch_play_specified_sps, EXPLOSION_NUM_ELEMENTS - 1, EXPLOSION_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 6, (uint8_t*) patch_play_specified_sps, 0, FUSE_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 7, (uint8_t*) patch_play_specified_sps, GRENADEBOUNCE_NUM_ELEMENTS - 1, GRENADEBOUNCE_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 8, (uint8_t*) patch_play_specified_sps, MINEBOUNCE_NUM_ELEMENTS - 1, MINEBOUNCE_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 9, (uint8_t*) patch_play_specified_sps, RELOAD_NUM_ELEMENTS - 1, RELOAD_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 10, (uint8_t*) patch_play_specified_sps, SHOTGUN_NUM_ELEMENTS - 1, SHOTGUN_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 11, (uint8_t*) patch_play_specified_sps, WATER_NUM_ELEMENTS - 1, WATER_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 12, (uint8_t*) patch_play_specified_sps, POP_NUM_ELEMENTS - 1, POP_NUM_ELEMENTS},
	{2, DEFAULT_SOUND_NUMBER + 13, (uint8_t*) patch_play_specified_sps, THROWGRENADE_NUM_ELEMENTS - 1, THROWGRENADE_NUM_ELEMENTS},
	{0, -1, (uint8_t*) teleportPatch, 0, 256},	
	{0, -1, (uint8_t*) medipackPatch, 0, 256},			
	{0, -1, (uint8_t*) offSidePatch, 0, 256},
	{0, -1, (uint8_t*) enterMenuPatch, 0, 256},
	{0, -1, (uint8_t*) exitMenuPatch, 0, 256},
	{0, -1, (uint8_t*) moveMenuPatch, 0, 256},
	
//	{2, (int8_t*) soundData1, (uint8_t*) patch01, 0, SAMPLES_SOUNDDATA1},
//	{0, (int8_t*) soundData2, (uint8_t*) patch02, 0, SAMPLES_SOUNDDATA2}
};
int getNumberOfPatches()
{
	return sizeof(patches) / sizeof(patch_t);
}