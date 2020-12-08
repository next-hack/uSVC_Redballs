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
#ifndef PATCHES_H
#define PATCHES_H
#include "usvc_kernel/audio.h"
extern const  patch_t patches[]; 
int getNumberOfPatches();
enum
{
	PATCH_MACHINE_GUN = 4,
	PATCH_BAZOOKA,
	PATCH_BLOWTORCH,
	PATCH_BOING,
	PATCH_DRILL,
	PATCH_EXPLOSION,
	PATCH_FUSE,
	PATCH_GRENADE_BOUNCE,
	PATCH_MINE_BOUNCE,
	PATCH_RELOAD,
	PATCH_SHOTGUN,
	PATCH_WATER,
	PATCH_POP,
	PATCH_THROWGRENADE,
	PATCH_TELEPORT,
	PATCH_MEDIPACK,
	PATCH_OFFSIDE,
	PATCH_ENTER_MENU,
	PATCH_BACK_MENU,
	PATCH_MOVE_MENU,
	
};
#endif
