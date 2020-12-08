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
 * game.c/h. All the game logic occurs here. 
 * We did not split in several dozens of files: all that belongs strictly to the game, will be here.
 * Warning: major code cleanup for better stability and robustness should/will be done!
  * The game is not complete yet, further improvements will be made!
 */
#ifndef GAME_H_
#define GAME_H_
#include "usvc_config.h"
#define ASCII_ESCAPE 0x1b
#define MAX_DESTROYED_TILES 256
#define MAX_MAP_SIZE_X 88 // 72
#define MAX_MAP_SIZE_Y 42 //52
#define WATER_SPRITE_WIDTH 64
#define MAX_TEAMS 4
#define MAX_TEAM_UNITS 4
#define MAX_PLAYER_UNITS 8
#define TEAM_CPU 0x80
#define TEAM_CPU_DUMB (TEAM_CPU | 0)
#define TEAM_CPU_MEDIUM (TEAM_CPU | 1)
#define TEAM_CPU_SMART (TEAM_CPU | 2)
#define PLAYER_FACING_LEFT 0
#define PLAYER_FACING_RIGHT 0x80
#define FRACTIONAL_PART 16
#define PLAYER_STATE_IDLE 1
#define PLAYER_STATE_IDLE_LEFT (PLAYER_STATE_IDLE | PLAYER_FACING_LEFT)
#define PLAYER_STATE_IDLE_RIGHT (PLAYER_STATE_IDLE | PLAYER_FACING_RIGHT)
#define PLAYER_STATE_WALKING 2
#define PLAYER_STATE_WALKING_LEFT (PLAYER_STATE_WALKING | PLAYER_FACING_LEFT)
#define PLAYER_STATE_WALKING_RIGHT (PLAYER_STATE_WALKING | PLAYER_FACING_RIGHT)
#define PLAYER_STATE_JUMPING 4
#define PLAYER_STATE_JUMPING_LEFT (PLAYER_STATE_JUMPING | PLAYER_FACING_LEFT)
#define PLAYER_STATE_JUMPING_RIGHT (PLAYER_STATE_JUMPING | PLAYER_FACING_RIGHT)
#define PLAYER_STATE_FALLING 8
#define PLAYER_STATE_FALLING_LEFT (PLAYER_STATE_FALLING | PLAYER_FACING_LEFT)
#define PLAYER_STATE_FALLING_RIGHT (PLAYER_STATE_FALLING | PLAYER_FACING_RIGHT)
#define PLAYER_STATE_DROWNING 16
#define PLAYER_STATE_DROWNING_LEFT (PLAYER_STATE_DROWNING | PLAYER_FACING_LEFT)
#define PLAYER_STATE_DROWNING_RIGHT (PLAYER_STATE_DROWNING | PLAYER_FACING_RIGHT)
#define PLAYER_STATE_FIRING 32
#define PLAYER_STATE_FIRING_LEFT (PLAYER_STATE_FIRING | PLAYER_FACING_LEFT)
#define PLAYER_STATE_FIRING_RIGHT (PLAYER_STATE_FIRING | PLAYER_FACING_RIGHT)
#define PLAYER_STATE_DEAD 64
//
#define PROJECTILE_MAXSPEED_FRACTIONAL_PART 8
//
#define BAZOOKA_PRE_FIRE_ANIMATION 3
#define BAZOOKA_POST_FIRE_ANIMATION 6
//
#define AIM_POINTER_DISTANCE 20
//
#define WEAPON_NONE 0
#define WEAPON_BAZOOKA 1
#define WEAPON_GRENADE 2
#define WEAPON_DYNAMITE 3
#define WEAPON_CLUSTER_BOMB 4
#define WEAPON_MINE 5
#define WEAPON_SHOTGUN 6
#define WEAPON_AIRSTRIKE 7
#define WEAPON_TELEPORT 8
#define WEAPON_DEVASTATOR 9
#define WEAPON_SKIP_TURN 10
#define WEAPON_SURRENDER 11
#define WEAPON_BLOWTORCH 12
#define WEAPON_MACHINEGUN 13
#define WEAPON_DRILL 14
#define WEAPON_GIRDERS 15

//
#define MAX_INGAME_PROJECTILES 8
#define MAX_STATIC_UNITS 10
//
#define PROJECTILE_NONE 0
#define PROJECTILE_ROCKET 1
#define PROJECTILE_EXPLOSION 2
#define PROJECTILE_DUMMY_DEATH 3
#define PROJECTILE_DEATH_EXPLOSION 4
#define PROJECTILE_GRENADE 5
#define PROJECTILE_DYNAMITE 6
#define PROJECTILE_CLUSTER_BOMB 7
#define PROJECTILE_BOMBLET 8
#define PROJECTILE_INACTIVE_MINE 9
#define PROJECTILE_ACTIVE_MINE 10
#define PROJECTILE_EXPLODING_MINE 11
#define PROJECTILE_SHOTGUN_ROUND 12
#define PROJECTILE_SMOKE16 13
#define PROJECTILE_AIRSTRIKE_ROCKET 14
#define PROJECTILE_TELEPORT 15			// damage around the player.
#define PROJECTILE_BLOWTORCH 16
#define PROJECTILE_MACHINEGUN_ROUND 17
#define PROJECTILE_DRILL 18
#define PROJECTILE_AMMO_CRATE 19
#define PROJECTILE_MEDIPACK 20
#define PROJECTILE_EXPLODING_CRATE 21
#define PROJECTILE_EXPLODING_MEDIPACK 22
#define PROJECTILE_BLOWTORCH_UP_RIGHT 23
#define PROJECTILE_BLOWTORCH_DOWN_RIGHT 24
#define PROJECTILE_BLOWTORCH_UP_LEFT 25
#define PROJECTILE_BLOWTORCH_DOWN_LEFT 26

//
#define PROJECTILE_STATE_NOT_CREATED 0
#define PROJECTILE_STATE_CREATED 1
#define PROJECTILE_STATE_FLYING 3
//
#define PROJECTILE_FLAGS_SUBJECTED_TO_WIND 1
#define PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY 2
#define PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN 4
#define PROJECTILE_FLAGS_PROXIMITY_SETOFF 8
#define PROJECTILE_FLAGS_BOUNCES 16
#define PROJECTILE_FLAGS_FIXED_LAUNCH_SPEED 32
#define PROJECTILE_FLAGS_HAS_DIRECTIONS 64
#define PROJECTILE_FLAGS_TIME_SETOFF 128
#define PROJECTILE_FLAGS_IMMEDIATE_SETOFF 256
#define PROJECTILE_FLAGS_END_OF_ANIM_SETOFF 512
#define PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION 1024
#define PROJECTILE_FLAGS_CONFIGURABLE_SET_OFF_TIME 2048
#define PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE 4096
#define PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES 8192   // i.e like dynamite
#define PROJECTILE_FLAGS_BECOMES_STATIC 16384		// for mines
#define PROJECTILE_FLAGS_INVISIBLE 32768 // invisible
#define PROJECTILE_FLAGS_NO_SELF_DAMAGE 65536
#define PROJECTILE_FLAGS_NO_SELF_PROXIMITY_SET_OFF 131072	// for rockets, and shotgun or machinegun rounds
//
#define CLOUDS_WIND_SPEED_FACTOR 512		// at maximum wind speed (128), the clouds move at 1 pixel per frame
#define WIND_SPEED_FACTOR 0x10
#define PROJECTILE_TYPE_EFFECT 0x80			// this is just an effect, so it won't harm anybody.
#define PROJECTILE_TYPE_MASK (~PROJECTILE_TYPE_EFFECT)
//
#define WEAPON_FLAGS_FIXED_LAUNCH_FORCE 1
#define WEAPON_FLAGS_GRENADE_TYPE 2
#define WEAPON_FLAGS_AUTOMATIC	4
#define WEAPON_FLAGS_POINTER_REQUIRED 8 // for airstrikes or teleport
#define WEAPON_FLAGS_GIRDERS 16  // When preview should be shown
#define WEAPON_FLAGS_TELEPORT 32 // When the fire corresponds to a change op position.
#define WEAPON_FLAGS_AIRDROPPED 64		// when the projectile is not deployed from player's coordinates.
#define WEAPON_FLAGS_SKIP_TURN 128
#define WEAPON_FLAGS_SURRENDER 256
#define WEAPON_FLAGS_NO_AIMPOINTER 512
//#define WEAPON_FLAGS_HAS_REPEATED_POSTFIRE_ANIMATION 2048
#define WEAPON_FLAGS_HAS_DISCRETE_DIRECTIONS 1024
#define WEAPON_FLAGS_DRILL 2048
#define WEAPON_FLAGS_BLOWTORCH 4096
//
#define	DELTA_Y_TEAM_INDICATOR 20
#define DELTA_Y_HEALTH_INDICATOR 10
//
#define COMMAND_NONE 0
#define COMMAND_LEFT 1
#define COMMAND_RIGHT 2
#define COMMAND_JUMP 3
#define COMMAND_FIRE 4 
#define PLAYER_HEIGHT 8
#define PLAYER_WIDTH 12
#define MAX_Y_STEP 3
#define PLAYER_WALK_SPEED 0x8000
#define ANIMATION_WALKING_FRAME_DELAY 1U
enum gameMenu
{
	MENU_PAGE_TITLE = 0,
	MENU_PAGE_MAIN,
	MENU_PAGE_OPTIONS,
	MENU_PAGE_HELP,
	MENU_PAGE_CREDITS,
	MENU_PAGE_PLOT,
	MENU_PAGE_MATCH_SUMMARY	
};
enum gameMenuSubState
{
	GAME_SUBSTATE_MENU_MAIN = 1,
	GAME_SUBSTATE_MENU_OPTIONS,
	GAME_SUBSTATE_MENU_HELP,
	GAME_SUBSTATE_MENU_CREDITS,
	GAME_SUBSTATE_MENU_PLOT,	
	GAME_SUBSTATE_MENU_MATCH_SUMMARY,
	GAME_SUBSTATE_MENU_GO_RIGHT,
	GAME_SUBSTATE_MENU_GO_LEFT,
};
enum gameState
{
	GAME_STATE_TITLE_SCREEN = 0,
	GAME_STATE_MENU,
	GAME_STATE_PLAYING
};
enum gameStateTitle
{
	GAME_SUBSTATE_TITLE_PRESS_ANY_KEY,
	GAME_SUBSTATE_TITLE_WAIT_FOR_ANY_KEY,
	GAME_SUBSTATE_TITLE_SCROLL_UP,
};
//
#define POST_FIRE_CHECK_PLAYER_DAMAGE_FIRST_PAUSE 57
#define DAMAGE_SHOW_TIME 32

enum gameSubstate
{
	GAME_SUBSSTATE_ROUND_INIT = 0,
	GAME_SUBSTATE_FOCUS_TO_ACTIVE_UNIT,
	GAME_SUBSTATE_PLAYER_MOVING,
	GAME_SUBSTATE_FIRING,
	GAME_SUBSTATE_PRE_FIRING_ANIMATION,
	GAME_SUBSTATE_POST_FIRING_ANIMATION,
	GAME_SUBSTATE_FOLLOW_PHYSICS,
	GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_DAMAGE_FIRST_PAUSE,
	GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_SHOW_DAMAGES,
	GAME_POST_DAMAGE_SECOND_WAIT,
	GAME_SUBSTATE_PLAYER_MOVING_AFTER_DEPLOYMENT,
	GAME_SUBSTATE_REPEAT_FIRE,
	GAME_SUBSTATE_POSITION_SELECTION,
	GAME_SUBSTATE_CONFIRM_SURRENDER,
	GAME_SUBSTATE_BLOWTORCH_ACTION,
	GAME_SUBSTATE_DRILL_ACTION,
	GAME_SUBSTATE_PLACE_GIRDERS,
	GAME_SUBSTATE_FOCUS_ON_AIRSTRIKE_DESTINATION,
	GAME_SUBSTATE_END_OF_MATCH,
	GAME_SUBSSTATE_END_OF_ROUND
};
#define REGULAR_GRAVITY 0x2000
#define REGULAR_TERMINAL_VELOCITY 0x80000
#define PROJECTILE_WATER_TERMINAL_VELOCITY 0x10000
#define PLAYER_WATER_TERMINAL_VELOCITY 0x08000
#define PROJECTILE_WATER_DRAG_FACTOR 0x7E00
#define PLAYER_WATER_DRAG_FACTOR 0x7000
// Regular terrain constants
#define REGULAR_PLAYER_X_BOUNCE 0x2000
#define REGULAR_PLAYER_Y_BOUNCE 0x2000
#define REGULAR_PLAYER_X_FRICTION 0x4000
#define REGULAR_MIN_SPEED_TO_ROLL_OVER 0x1000
// ICE terrain constants
#define ICE_PLAYER_X_BOUNCE 0x4000
#define ICE_PLAYER_Y_BOUNCE 0x4000
#define ICE_PLAYER_X_FRICTION 0xFF00
#define ICE_MIN_SPEED_TO_ROLL_OVER 0x200
//
#define JUMPING_MAXSPEED 0x50000 
#define JUMPING_INITIAL_SPEED -0x30000
#define JUMPING_HORIZONTAL_SPEED 0x10000
#define UNIT_MIN_SPEED 0x1000 // about 4 pix/s
#define PROJECTILE_FIRING_DISTANCE 0x40000
// to recover some temporary tiles, we assume that if a tile has at least 56 pixel cleared, then it is completely cleaderd :)
#define MIN_PIXEL_TO_CLEAR	60

#define STATIC_UNIT_TYPE_NONE 0
#define STATIC_UNIT_TYPE_MINE 1
#define STATIC_UNIT_TYPE_AMMO_CRATE 2
#define STATIC_UNIT_TYPE_MEDIPACK 3

// every sprite, regardless its function, is an unit. It can be a soldier, a mine, a projectile, a bonus, etc.
typedef struct  
{
	// 16.16 fixed point position and speed
	int32_t x;
	int32_t y;
	int32_t vx;
	int32_t vy;	
	// time keeper for animations
	uint8_t lastFrameTick;		
	// number of frame
	uint8_t frameNumber;
	// current state
	uint8_t state;
	uint8_t type;
	uint8_t frameFlags;
} unit_t;
// staticUnit is used for mines and bonuses, once they get to their equilibrium. When they fly or they explode, they become projectiles, until they become once again static (or they disappear).
// To avoid using too much memory, y can go only up to 512 (the map is still smaller). 
typedef struct  
{
	int16_t x;
	int16_t y : 10;
	uint16_t type : 6;				
} staticUnit_t;
// this is a dummy unit, only for camera view purposes. It is only used by drawScene, which does not use any other members!
typedef struct  
{
	int32_t x;
	int32_t y;
} camera_unit_t;
// Projectiles do not have health, so we keep this information separately for the players.
extern uint8_t xScrollTileTable[VRAMY];
extern uint16_t usedDestroyedTileNumber; // for debug
void gameLoop(uint16_t *keyBuffer, uint8_t aiTime);
void gameInit(void);
#endif /* GAME_H_ */