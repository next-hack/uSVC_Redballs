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
#include "main.h"
#define WEAPON_SOUND_CHANNEL 0
#define FASTEST_UNIT_DELTA_SQUARE_SPEED 4 // to prevent the camera from jumping back and forth
#define DELTA_TILE_Y_MENU 6   // how many tiles we need to scroll down for menu
#define TITLE_MENU_Y_OFFSET (DELTA_TILE_Y_MENU + 1)	// where is the game title placed
#define GAME_LOGO_TILE_HEIGHT 8 // height in tiles of the game logo (including its y offset)
#define TEXT_HIGHLIGHT_COLOR 6
#define TEXT_NORMAL_COLOR 2
#define TEXT_NORMAL (TEXT_HIGHLIGHT_COLOR > TEXT_NORMAL_COLOR ? 0 : 1)
#define TEXT_HIGH_LIGHT (TEXT_HIGHLIGHT_COLOR > TEXT_NORMAL_COLOR ? 1 : 0)
#define FIRST_TEXT_COLOR  (TEXT_HIGHLIGHT_COLOR > TEXT_NORMAL_COLOR ? TEXT_NORMAL_COLOR : TEXT_HIGHLIGHT_COLOR)
#define SECOND_TEXT_COLOR (TEXT_HIGHLIGHT_COLOR > TEXT_NORMAL_COLOR ?  TEXT_HIGHLIGHT_COLOR : TEXT_NORMAL_COLOR)
#define BLACK_COLOR_INDEX 5
#define BLACK_BACKGROUND (BLACK_COLOR_INDEX * 0x11111111)
#define TEXT_FLAGS_COLOR_NORMAL 0
#define TEXT_FLAGS_COLOR_HIGHLIGHT 1
#define TEXT_FLAGS_ALIGN_LEFT 0
#define TEXT_FLAGS_ALIGN_RIGHT 2
#define TEXT_FLAGS_ALIGN_CENTER 4
#define TEXT_FLAGS_IS_ITEM 0x80
#define BONUS_PROBABILITY 50
#define BONUS_PROBABILITY_BIN (655 * BONUS_PROBABILITY)
#define NUMBER_OF_WEAPONS (sizeof(weaponsData) / sizeof(weapon_t))
#define PLAYER_BLOWTORCH_SPEED 0x2000
#define X_NOSPAWN_BORDER 10
#define Y_NOSPAWN_BORDER 10
#define BOUNCE_HORIZONTAL 1  
#define BOUNCE_VERTICAL 2
#define BOUNCED_TIMEOUT 0x80
#define FPS 57
#define FOCUS_AIRSTRIKE_TIME (FPS)
#define DYNAMITE_CONTROL_TIME (FPS*6)
#define MINE_CONTROL_TIME (FPS*6)
#define GAME_END_OF_MATCH_SHOW_TIME (FPS * 4)
#define MAX_INITIAL_MINES 6
#define MATCH_TURN_TIME 30		
#define MINE_SET_OFF_SQUARE_RADIUS (100)
#define AIRDROP_Y (-50)
#define MEDIPACK_HEALTH 25
#define DAMAGE_VELOCITY_COEFFICIENT 0x2000
//
#define CHECKPOINT_PERIOD 16
#define CHECKPOINT_PERIOD_LOG2 4
#define CHECKPOINT_PALETTES 2
#define CHANGED_PALETTE_ENTRIES 16
//
//
#define FOCUS_TO_ACTIVE_UNIT_TIME 57
//
#define MAX_AI_PROJECTION_ITERATIONS (10*57)	// 5 seconds...
#define AI_STATE_CHOOSE_ACTION 0
#define AI_STATE_CONTINUE_SIMULATION 1
#define AI_STATE_FIRE 2
#define AI_STATE_RUN_AWAY 3
//
#define AI_MOVEMENT_PROBABILITY 1
#define AIRSTRIKE_PROBABILITY 50
#define AI_JUMP_PROBABILITY 10
#define ADVANCED_STRATEGY_PROBABILITY 20
#define AI_MOVEMENT_BIN_PROBABILITY (655 * AI_MOVEMENT_PROBABILITY)
#define AI_JUMP_BIN_PROBABILITY (655 * AI_JUMP_PROBABILITY)
#define ADVANCED_STRATEGY_BIN_PROBABILITY (655 * ADVANCED_STRATEGY_PROBABILITY)
#define AIRSTRIKE_BIN_PROBABILITY  (655 * AIRSTRIKE_PROBABILITY)
//
#define AI_MOVEMENT_POSSIBLE 0
#define AI_MOVEMENT_BAD_DECISION 1
#define AI_MOVEMENT_CANNOT_MOVE 2
#define AI_MOVEMENT_CANNOT_EVALUATE 3
#define MAX_FRAME_LINE 505
#define MAX_MOVEMENT_FRAME_LINE 490
// messages to show 
#define MESSAGE_NEW_WEAPON 1
#define MESSAGE_NO_ENOUGH_AMMO 2
#define MESSAGE_CONFIRM_SURRENDER 4	
#define MESSAGE_TIMEUP 8
#define MESSAGE_DEAD_UNIT 16
#define MESSAGE_MATCH_WON 32
#define MESSAGE_NO_SPACE_FOR_GIRDER 64
#define MESSAGE_BONUS 128
#define MESSAGE_BONUS_WEAPON 256
#define MESSAGE_BONUS_HEALTH 512
#define MESSAGE_GET_THEM 1024
#define MESSAGE_ROUND_WON 2048
#define DEBUG_KEYS 0
#if DEBUG_TIME 
	uint16_t gLine = 0;
#endif
//
#define WATER_HEIGHT water_height
#define WATER_LEVEL (levels[currentLevel].pixelSizeY - WATER_HEIGHT)
//
#define MAX_GIRDER_X_BY_Y 12	// girders are either vertical 1x4 or horizontal 4x1. Exceptions are oblique girders, which are 4x3.
//
#define CLOUD_PATTERN_WIDTH 320
#define NUMBER_OF_CLOUDS 6
#define NUMBER_OF_WIND_SPEEDS 5
uint8_t xScrollTileTable[VRAMY];
const int8_t windSpeeds[2 * NUMBER_OF_WIND_SPEEDS] = {-16, -8, -4, -2, -1, 1, 2, 4, 8, 16};
//
const uint8_t healthBarCoordinates[4][2] = { {16, 1}, { 29,1}, {16, 2}, {29,2}};
//
const uint8_t aiWeapons[] = {WEAPON_BAZOOKA, WEAPON_GRENADE, WEAPON_CLUSTER_BOMB, WEAPON_SHOTGUN, WEAPON_MACHINEGUN};
//
const uint16_t cloudY[NUMBER_OF_CLOUDS] = {16, 32+2, 48+4, 32+2, 16, 48+4};
const uint16_t cloudX[NUMBER_OF_CLOUDS] = {0, 55, 110, 165, 220, 275};
const uint16_t cloudFrames[NUMBER_OF_CLOUDS] = {CLOUD1_CLOUD_FRAMEINDEX, CLOUD3_CLOUD_FRAMEINDEX, CLOUD4_CLOUD_FRAMEINDEX, CLOUD3_CLOUD_FRAMEINDEX, CLOUD2_CLOUD_FRAMEINDEX, CLOUD4_CLOUD_FRAMEINDEX};
int32_t cloudPatternX;

const char teamNames[4][7] = {"RED", "BLUE", "YELLOW", "GRAY"};
static const char * roundWonStrings[] = {"EVERYONE IS DEAD", "ROUND: RED!", "ROUND: BLUE!", "ROUND: YELLOW!", "ROUND: GRAY!"};
static const char * matchWonStrings[] = {"DRAW MATCH!", "MATCH: RED!", "MATCH: BLUE!", "MATCH: YELLOW!", "MATCH: GRAY!"};
static const char * matchWinnerString[] = {"TEAM RED WINS THE MATCH!", "TEAM BLUE WINS THE MATCH!", "TEAM YELLOW WINS THE MATCH!", "TEAM GRAY WINS THE MATCH!"};
const uint8_t teamColors[5] = {8, 11, 3 , 9, 7};
uint8_t water_height = 24;
uint32_t usedDestroyedTiles[MAX_DESTROYED_TILES / 32];
uint16_t usedDestroyedTileNumber = 0; // for debug
const uint8_t explosionScreenOscillation [] = {3, 4, 5, 3 , 1 , 0, 2,  4, 3, 2, 1, 0, 1,  3, 2, 1, 2, 0};
const uint16_t staticUnitFrames[] = {MINE_ROTATING_FRAMEINDEX, AMMO_BONUS_FRAMEINDEX, MEDIPACK_BONUS_FRAMEINDEX};
unit_t playerUnits[MAX_PLAYER_UNITS];
static const char * decimalStrings [] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
static const char * teamTypeStrings [] = {"HUMAN", "CPU - DUMB"}; // TODO: better AI
typedef struct
{
	uint16_t index : 3;
	int16_t damage : 13;
} evaluatedDamage_t;
typedef struct
{
	int32_t x;
	int32_t y;
	int32_t vx;
	int32_t vy;
} projectileData_t;
typedef struct
{
	projectileData_t projectile;		// data of the current projectile
	evaluatedDamage_t damages[MAX_PLAYER_UNITS];
	uint16_t simulationIterationNumber;			// iteration number of the simuation
	uint16_t goingRight : 1;
	int16_t rightMostCheckedX:15;
	int8_t simulationAngle;
	uint8_t simulationForce;
	uint8_t simulationConfigTimeMultiplier;
	uint8_t simulationWeapon;
	uint8_t aiState;
	uint8_t aiStateParam;

} ai_t;
typedef struct  
{
	uint8_t loTileNumber[MAX_MAP_SIZE_X * MAX_MAP_SIZE_Y];
	uint8_t highBitTileNumber[MAX_MAP_SIZE_X * MAX_MAP_SIZE_Y / 8];
} gameRamMap_t;
gameRamMap_t gameRamMap;
const uint16_t teamsIndicator[4] = {TEAMRED_INDICATOR_FRAMEINDEX, TEAMBLUE_INDICATOR_FRAMEINDEX, TEAMYELLOW_INDICATOR_FRAMEINDEX, TEAMGREY_INDICATOR_FRAMEINDEX};
typedef struct  
{
	uint8_t frame;
	uint8_t flags;
} directionalSprites_t;
typedef struct  
{
	uint8_t sizeX;
	uint8_t sizeY;
	uint8_t tiles[MAX_GIRDER_X_BY_Y];
} girderTemplate_t;
const girderTemplate_t girderTemplates[] =
{
	{	// vertical
		.sizeX = 1,
		.sizeY = 4,
		.tiles = {67, 67, 67, 67}
	},
	{	// horizontal
		.sizeX = 4,
		.sizeY = 1,
		.tiles = {68, 68, 68, 68}
	},  
	{	// down-right
		.sizeX = 4,
		.sizeY = 3,
		.tiles = {69, 70,  0,  0,
				  71, 72, 69, 70,
				  0,   0, 71, 72
				  }
	},
	{	// up-right
		.sizeX = 4,
		.sizeY = 3,
		.tiles = { 0,  0, 73, 74,
			      73, 74, 75, 76,
			      75, 76,  0, 0
		}
	},	
};

const directionalSprites_t directionalSprites16[16] =
{
	{	.frame = 4, .flags = 0  },
	{	.frame = 3, .flags = SPRITE_FLAGS_FLIP_VERTICAL},
	{	.frame = 2, .flags = SPRITE_FLAGS_FLIP_VERTICAL  },
	{	.frame = 1, .flags = SPRITE_FLAGS_FLIP_VERTICAL  },
	{	.frame = 0, .flags = SPRITE_FLAGS_FLIP_VERTICAL  },
	{	.frame = 1, .flags = SPRITE_FLAGS_FLIP_VERTICAL | SPRITE_FLAGS_FLIP_HORIZONTAL},
	{	.frame = 2, .flags = SPRITE_FLAGS_FLIP_VERTICAL | SPRITE_FLAGS_FLIP_HORIZONTAL},
	{	.frame = 3, .flags = SPRITE_FLAGS_FLIP_VERTICAL | SPRITE_FLAGS_FLIP_HORIZONTAL},
	{	.frame = 4, .flags =  SPRITE_FLAGS_FLIP_HORIZONTAL},
	{	.frame = 3, .flags = SPRITE_FLAGS_FLIP_HORIZONTAL  },
	{	.frame = 2, .flags = SPRITE_FLAGS_FLIP_HORIZONTAL  },
	{	.frame = 1, .flags = SPRITE_FLAGS_FLIP_HORIZONTAL  },
	{	.frame = 0, .flags = 0  },
	{	.frame = 1, .flags = 0  },
	{	.frame = 2, .flags = 0  },
	{	.frame = 3, .flags = 0  }
};
typedef struct  
{
	const int16_t walkFrameIndex[3] ;
	const int8_t walkFrameNumbers[3];	
	const int16_t aimFrameIndex[3];
	const int16_t shootFrameIndex[3];
	const int8_t  preShootFrameNumbers[3];
	const int8_t  duringShootFrameNumbers[3];
	const int8_t  postShootFrameNumbers[3];
	const uint8_t weaponSymbols[4];
	int16_t preShootSoundPatch;
	int16_t duringShootSoundPatch;
	int16_t postShootSoundPatch;	
	uint16_t flags;								// e.g. if is a fixed force firing weapon (grenade launcher)
	uint16_t afterFirePlayerControlTime;		// number of frames, during which the player can still move after firing/deployment (e.g. for dynamite and mines)
	uint8_t projectileType;						// projectile that will be fired/deployed
	uint8_t numberOfManualRounds;				// number of turns the player can shoot with the weapon (e.g. 2 for shotgun)
	uint8_t numberOfAutomaticRounds;			// number of rounds fired automatically (e.g. for machineguns)
	uint8_t delayBetweenRounds;					// in frames. 
	uint8_t maxAngleError;						// amount of max angle error. For instance on automatic weapons. Unit: 1 = +/- PI/512 
	uint8_t fireAnimDelay;

	const char name[16];								// display name
} weapon_t;
typedef struct  
{
	uint32_t flags;									
	uint16_t projectileFrameIndex ;					 
	uint16_t projectileNumberOfFrames;				// number of frames for the animation
	uint8_t projectileSequenceNumFrames;			// number of frames per each anim sequence (only if PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES is used)
	uint8_t projectileSequenceDuration;				// how many animation frames should last each sequence (only if PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES is used)
	uint16_t directHitDamage;						// 100% direct hit damage
	uint16_t directHitSquaredRadius;
	uint16_t damageSquaredRadius;					// damage radius^2. 
	uint16_t proximitySetOffSquareRadius;
	uint16_t autoSetOffTime;						// in frames...
	int16_t bounceSoundPatch;
	int16_t setOffSoundPatch;	
	int16_t xBounceFract;
	int16_t yBounceFract;
	uint8_t projectileDelayBetweenFrames;			// delay (in frames) between each frame of the animation
	uint16_t damageHole;							// shape to be removed from ground
	uint8_t halfWidth;
	uint16_t maxSpeed;							// in pixels per frame, 8.8 fixed fract.
	uint8_t setOffProjectileType;				// explosion or other
	uint8_t setOffSecondaryProjectileType;		// for cluster bombs
	uint8_t setOffSecondaryProjectileNumber;	// for cluster bombs					
} projectile_t;
// function prototypes
uint8_t movePlayer(unit_t *pUnit, uint8_t isSimulating);
uint8_t playerHandler(unit_t* pUnit, uint8_t command, uint8_t isSimulating);
void calculateInitialProjectileData( projectileData_t *pid, unit_t *pUnit, int8_t angle , uint8_t shootRight, uint8_t weapon, uint8_t force);
uint8_t evaluateAiMovement(unit_t *pUnit, uint8_t action, uint16_t maxFrameLine);
uint8_t createRandomLevel(int seed);
uint8_t putTemplate(int x, int y, int t);
void drawScene(unit_t * pFocusedUnit, uint8_t forceRedraw);
void randomlyPlaceUnits();
uint8_t bresenhamCollisionCheckAgainstUnits(int x0, int y0, int x1, int y1, int *xColl, int *yColl, const projectile_t * pProjectile);
uint8_t bresenhamCollisionCheck(int x0, int y0, int x1, int y1, int *xColl, int *yColl);
uint8_t bresenhamCollisionCheckTiles(int x0, int y0, int x1, int y1);
void clearDamages();
void roundInit();
int getNextAvailableTeamUnit(int teamNumber);
int16_t evaluateAIProjectileDamage(uint16_t  bonusEnemyDeath, uint16_t bonusEnemyDamage, uint16_t malusTeamDeath, uint16_t malusTeamDamage, uint8_t *finished, uint16_t maxFrameLine);
uint16_t aiHandler(uint8_t t, uint8_t u);
unit_t * convertStaticToProjectile(staticUnit_t * sU);
uint8_t physics(unit_t **ppfastestMovingUnit, uint8_t doNotCalculatePlayers, uint16_t *message, uint32_t *messageParam);
void convertProjectileToStatic(unit_t *pUnit);
int getNextTeam();
void fireCurrentWeapon(unit_t *pUnit, uint8_t shootRight);
uint8_t placeGirder(int16_t newX, int16_t newY, int16_t *oldX, int16_t *oldY, uint8_t templateNumber, uint8_t oldTemplateNumber);
uint8_t dropBonus();
inline void initFramePalettes(int remapVerticalOffset, const uint16_t *pCheckpointData);
void setViewPosition(int *px, int * py, uint8_t forceRedraw);
inline uint32_t getRamMapTileFromTileCoordinates(uint16_t xTile,  uint16_t yTile);
inline uint8_t getMapPixel(int x, int y);
inline void setRamMapTileFromTileCoordinates(uint16_t xTile,  uint16_t yTile, uint16_t tileNumber);
void drawRamMap(uint16_t xOffset, uint16_t yOffset, uint8_t forceRedraw);
void setDestroyedTileIndexUsedState(int index, int state);
int getFirstAvailableDestroyedTile();
uint32_t destroyPlayground(int32_t xSpr, int32_t ySpr, uint32_t nf);
void updateFixedSection(uint8_t updateText, const char *text, uint8_t textColor, uint8_t forceUpdate);
inline void putSymbolToFixedTile(uint8_t fixedTileNumber, uint8_t symbol);
void drawMenuPage(uint8_t menuPage, uint8_t selectedItem);
uint8_t goToMenuPage(uint8_t menuPage);
void printLine (const char * text, int8_t col, uint8_t row, uint8_t flags);
void initFixedSection(uint8_t gameMode);
void menuInit(void);
typedef struct
{
	uint8_t col;
	uint8_t row;
	uint8_t flags;
	const char *text;
} menuPageLine_t;
const menuPageLine_t mainMenuPageLines [] =
{
	{.col = 20, .row = 16, .flags =  TEXT_FLAGS_ALIGN_CENTER, "Up/down to select, enter to confirm."},
	{.col = 20, .row = 19, .flags =  TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_IS_ITEM, "Game Start"},
	{.col = 20, .row = 21, .flags =  TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_IS_ITEM, "Options"},
	{.col = 20, .row = 23, .flags =  TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_IS_ITEM, "How to play"},
	{.col = 20, .row = 25, .flags =  TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_IS_ITEM, "Credits"},
	{.col = 20, .row = 27, .flags =  TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_IS_ITEM, "Plot"},
	{0,0,0,NULL}
};
const menuPageLine_t optionsMenuPageLines [] =
{
	{.col = 60, .row = 1 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "Up/down to select, enter to change."},
	{.col = 45, .row = 6 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM,  "Number of Teams:  "},
	{.col = 45, .row = 8 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM,  "Team Red......... "},
	{.col = 45, .row = 10 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Team Yellow...... "},
	{.col = 45, .row = 12 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Team Blue........ "},
	{.col = 45, .row = 14 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Team Gray........ "},
	{.col = 45, .row = 16 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Rounds per Match: "},
	{.col = 41, .row = 21 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Go back to main menu"},
	{0,0,0,NULL}
};
const menuPageLine_t helpMenuPageLines [] =
{
	{.col = 60, .row = 0 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "Key mappings keyboard - (gamepad)"},
	{.col = 41, .row = 2 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "a-d (gamepad: left/right):"},
	{.col = 79, .row = 3 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "go left/right"},
	{.col = 41, .row = 4 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "w-s (gamepad: up/down):"},
	{.col = 79, .row = 5 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "aim up/down."},
	{.col = 41, .row = 6 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "f (gamepad: 2/B):"},
	{.col = 79, .row = 7 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "jump"},
	{.col = 41, .row = 8 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,  "g (gamepad: 3/A)"},
	{.col = 79, .row = 9 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT,  "fire weapon"},
	{.col = 41, .row = 10 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Cursor keys (gamepad: R2 + d-pad"},
	{.col = 79, .row = 11 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "move view"},
	{.col = 41, .row = 12 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "y - ESC (gamepad: 1/Y - 4/X):"},
	{.col = 79, .row = 13 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "confirm - abort airstrike or teleport"},
	{.col = 41, .row = 14 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "1-5 (gamepad L2):"},
	{.col = 79, .row = 15 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "select grenade set off time"},
	{.col = 41, .row = 16 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "F1-F12 once or F1-F3 twice"},
	{.col = 79, .row = 17 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "select weapons 1-12 or 13-15."},
	{.col = 41, .row = 18 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "+ and - (gamepad L1 and R1):"},
	{.col = 79, .row = 19 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_RIGHT, "Choose previous and next weapons"},
	{.col = 41, .row = 21 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Go back to main menu"},
	{0,0,0,NULL}
};
const menuPageLine_t creditsMenuPageLines [] =
{
	{.col = 60, .row = 1 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "RedBalls* CREDITS"},
	{.col = 60, .row = 2 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "*not in the UrbanDictionary definition!"},
//	{.col = 60, .row = 0 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "01234567890123456789012345678901234567"},
	{.col = 41, .row = 4 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "A Worms-like game on a Cortex M0+!"},
	{.col = 60, .row = 6 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "--USVC Kernel--"},
	{.col = 41, .row = 7 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "VGA engine: Nicola Wrachien"},
	{.col = 41, .row = 8 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Audio Mixer: Nicola Wrachien"},
	{.col = 41, .row = 9 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Sound Engine:"},
	{.col = 41, .row = 10 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "- Original: Alec Borque (UZEBOX)"},
	{.col = 41, .row = 11 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "- Additional Features: Nicola Wrachien"},
	{.col = 60, .row = 13 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "--Game--"},
	{.col = 41, .row = 14 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Graphics: Nicola Wrachien"},
	{.col = 41, .row = 15 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Additional Graphics: "},
	{.col = 41, .row = 16 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "- Original Explosion: JRob774"},
	{.col = 41, .row = 17 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Programming: Nicola Wrachien"},
	{.col = 41, .row = 18 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Sounds: Nicola Wrachien"},
	{.col = 41, .row = 19 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT, "Additional Sounds: Public domain"},
	{.col = 41, .row = 21 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Go back to main menu"},
	{0,0,0,NULL}
};
const menuPageLine_t plotMenuPageLines [] =
{
	{.col = 60, .row = 1 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "RedBalls - The story"},
//	{.col = 60, .row = 00 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "01234567890123456789012345678901234567"},
	{.col = 41, .row = 3 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "The era is unknown. Alocin, an evil"},
	{.col = 41, .row = 4 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "programmer that hates phyton and"},
	{.col = 41, .row = 5 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "thinks  it should not be used on "},
	{.col = 41, .row = 6 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "microcontrollers (actually it should"},
	{.col = 41, .row = 7 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "be used nowhere),  is coding a game"},
	{.col = 41, .row = 8 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "in C, to show that it is better"},
	{.col = 41, .row = 9 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "suited. He the starts drawing the"},
	{.col = 41, .row = 10 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "main characters, the redballs, a"},
	{.col = 41, .row = 11 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "little lovely alien creature."},
	{.col = 41, .row = 12 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "However, Alocin is so full of hate"},
	{.col = 41, .row = 13 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "that those bunch of pixels come"},
	{.col = 41, .row = 14 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "alive, invading the known universe."},
	{.col = 41, .row = 15 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "Those creatures do not love each"},
	{.col = 41, .row = 16 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "other, so they start forming squads,"},
	{.col = 41, .row = 17 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT,   "to fight to the death."},
	{.col = 41, .row = 21 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Go back to main menu"},
	{0,0,0,NULL}
};
const menuPageLine_t matchSummaryMenuPageLines [] =
{
	{.col = 60, .row = 1 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_CENTER, "Match summary..."},
	{.col = 41, .row = 21 + DELTA_TILE_Y_MENU, .flags =  TEXT_FLAGS_ALIGN_LEFT | TEXT_FLAGS_IS_ITEM, "Go Back to main menu"},
	{0,0,0,NULL}
};
static const menuPageLine_t * pMenuPages[] = 
{
	NULL,
	mainMenuPageLines,
	optionsMenuPageLines,
	helpMenuPageLines,
	creditsMenuPageLines,
	plotMenuPageLines,
	matchSummaryMenuPageLines,
};
const projectile_t projectiles[] = 
{
	{ //0 none
		.flags = 0,
		.projectileFrameIndex = ROCKET_FLYING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = 1,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = 0,
		.halfWidth = 0,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1
	},
	{ // 1 rocket
		.flags = PROJECTILE_FLAGS_NO_SELF_PROXIMITY_SET_OFF | PROJECTILE_FLAGS_SUBJECTED_TO_WIND | PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN | PROJECTILE_FLAGS_PROXIMITY_SETOFF | PROJECTILE_FLAGS_HAS_DIRECTIONS,
		.projectileFrameIndex = ROCKET_FLYING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = 9,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x1000,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{  // 2 - explosion
		.flags = 0,
		.projectileFrameIndex = EXPLOSION_EXPLOSION_FRAMEINDEX,
		.projectileNumberOfFrames = 12,		// 
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 100,
		.damageSquaredRadius = 400,
		.proximitySetOffSquareRadius = 100,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x1000,
		.setOffProjectileType = 0,	// this does not setoff		
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},
	// Player death animation
	{ // 3
		.flags = PROJECTILE_FLAGS_END_OF_ANIM_SETOFF | PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION,
		.projectileFrameIndex = PLAYER_DEATH_FRAMEINDEX,
		.projectileNumberOfFrames = 11,		//
		.projectileDelayBetweenFrames = 3,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 30,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x1000,
		.setOffProjectileType = PROJECTILE_DEATH_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_POP		
	},
	{  // 4 - player death
		.flags = 0,
		.projectileFrameIndex = PLAYER_DEATH_FRAMEINDEX + 11,
		.projectileNumberOfFrames = 2,		//
		.projectileDelayBetweenFrames = 2,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 20,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 0,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x1000,
		.setOffProjectileType = 0,	// this does not setoff
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},
	{ // 5 Grenade
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE | PROJECTILE_FLAGS_CONFIGURABLE_SET_OFF_TIME | PROJECTILE_FLAGS_BOUNCES | PROJECTILE_FLAGS_TIME_SETOFF,
		.projectileFrameIndex = GRENADE_FLYING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		// 
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = FPS,			// 1 sec, multiplied by time multiplier...
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 1,
		.maxSpeed = 0x800,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = PATCH_GRENADE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{ // 6 Dynamite
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_BOUNCES | PROJECTILE_FLAGS_TIME_SETOFF | PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES,
		.projectileFrameIndex = DYNAMITE_BURNING_FRAMEINDEX + 1,
		.projectileNumberOfFrames = DYNAMITE_CONTROL_TIME,		// 
		.projectileDelayBetweenFrames = 1,
		.projectileSequenceNumFrames = 3,			// number of frames per each anim sequence (only if PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES is used)
	    .projectileSequenceDuration = FPS,				// how many animation frames should last each sequence (only if PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES is used)
		.damageHole = DAMAGEMASK32_DAMAGEMASK32_FRAMEINDEX,		//
		.directHitDamage = 75,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = DYNAMITE_CONTROL_TIME,			
		.damageSquaredRadius = 50*50,
		.proximitySetOffSquareRadius = 0,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 1,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = PATCH_GRENADE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},	
	{ // 7 Cluster Bomb
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE | PROJECTILE_FLAGS_CONFIGURABLE_SET_OFF_TIME | PROJECTILE_FLAGS_BOUNCES | PROJECTILE_FLAGS_TIME_SETOFF,
		.projectileFrameIndex = GRENADE_FLYING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 20,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = FPS,			// 1 sec, multiplied by time multiplier...
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 1,
		.maxSpeed = 0x800,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileType = PROJECTILE_BOMBLET,
		.setOffSecondaryProjectileNumber = 4,
		.bounceSoundPatch = PATCH_GRENADE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{ // 8 Bomblet
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY |  PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN,
		.projectileFrameIndex = BOMBLET_ROTATING_FRAMEINDEX,
		.projectileNumberOfFrames = 2,		//
		.projectileDelayBetweenFrames = 2,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 20,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = FPS,			// 1 sec, multiplied by time multiplier...
		.damageSquaredRadius = 400,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 1,
		.maxSpeed = 0x400,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{ // 9 Mine
		.flags = PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_BECOMES_STATIC | PROJECTILE_FLAGS_BOUNCES,
		.projectileFrameIndex = MINE_ROTATING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = MINE_CONTROL_TIME,		
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = MINE_SET_OFF_SQUARE_RADIUS,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_ACTIVE_MINE,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = PATCH_MINE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},	
	{ // 10 Active mine (when a static mine is hit by explosion or other projectiles so it can fly)
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_BECOMES_STATIC | PROJECTILE_FLAGS_BOUNCES | PROJECTILE_FLAGS_PROXIMITY_SETOFF,
		.projectileFrameIndex = MINE_ROTATING_FRAMEINDEX,
		.projectileNumberOfFrames = MINE_ROTATING_NUMFRAMES,		//
		.projectileDelayBetweenFrames = 2,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = MINE_SET_OFF_SQUARE_RADIUS,
		.xBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = PATCH_MINE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},	
	{	// 11: exploding mine. When a static object of type mine came too close to a mine
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION |  PROJECTILE_FLAGS_IMMEDIATE_SETOFF ,
		.projectileFrameIndex = MINE_ROTATING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = MINE_SET_OFF_SQUARE_RADIUS,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{	// 12: shotgun round
		.flags =  PROJECTILE_FLAGS_NO_SELF_PROXIMITY_SET_OFF | PROJECTILE_FLAGS_PROXIMITY_SETOFF | PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_FIXED_LAUNCH_SPEED,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 25,
		.directHitSquaredRadius = 9,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 400,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x2000,
		.setOffProjectileType = PROJECTILE_SMOKE16 | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1			// put projectile sound!	
	},
	{  // 13 - smoke16
		.flags = 0,
		.projectileFrameIndex = SMOKE16_SMOKE16_FRAMEINDEX,
		.projectileNumberOfFrames = 11,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 0,
		.directHitSquaredRadius = 100,
		.damageSquaredRadius = 400,
		.proximitySetOffSquareRadius = 100,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x1000,
		.setOffProjectileType = 0,	// this does not setoff
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},
	{ // 14 Airstrike rocket
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION | PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN | PROJECTILE_FLAGS_PROXIMITY_SETOFF | PROJECTILE_FLAGS_HAS_DIRECTIONS | PROJECTILE_FLAGS_FIXED_LAUNCH_SPEED,
		.projectileFrameIndex = ROCKET_FLYING_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x0200,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},	
	{ // 15 TELEPORT
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 10,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},	
	{ // 16 BLOWTORCH STRAIGHT
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = BLOWTORCH_DAMAGEMASK_STRAIGHT_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},	
	{	// 17: machine gun round
		.flags =  PROJECTILE_FLAGS_NO_SELF_PROXIMITY_SET_OFF | PROJECTILE_FLAGS_PROXIMITY_SETOFF | PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_FIXED_LAUNCH_SPEED,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK8_DAMAGEMASK8_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 36,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 81,
		.proximitySetOffSquareRadius = 36,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x2000,
		.setOffProjectileType = PROJECTILE_SMOKE16 | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		 // put projectile sound!
	},
	{ // 18 DRILL
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = DRILL_DAMAGEMASK_DOWN_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},	
	{ // 19 Ammo Bonus
		.flags = PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_BECOMES_STATIC | PROJECTILE_FLAGS_BOUNCES,
		.projectileFrameIndex = AMMO_BONUS_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK32_DAMAGEMASK32_FRAMEINDEX,		//
		.directHitDamage = 75,
		.directHitSquaredRadius = 81,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = PATCH_GRENADE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},	
	{ // 20 Medipack Bonus
		.flags = PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY | PROJECTILE_FLAGS_BECOMES_STATIC | PROJECTILE_FLAGS_BOUNCES,
		.projectileFrameIndex = MEDIPACK_BONUS_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK24_DAMAGEMASK24_FRAMEINDEX,		//
		.directHitDamage = 50,
		.directHitSquaredRadius = 81,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 512,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = PATCH_GRENADE_BOUNCE,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{	// 21: exploding crate
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION |  PROJECTILE_FLAGS_IMMEDIATE_SETOFF ,
		.projectileFrameIndex = AMMO_BONUS_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK32_DAMAGEMASK32_FRAMEINDEX,		//
		.directHitDamage = 75,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 1024,
		.proximitySetOffSquareRadius = MINE_SET_OFF_SQUARE_RADIUS,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_EXPLOSION | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileType = PROJECTILE_BOMBLET,
		.setOffSecondaryProjectileNumber = 4,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{	// 22: exploding medpack
		.flags = PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION |  PROJECTILE_FLAGS_IMMEDIATE_SETOFF ,
		.projectileFrameIndex = AMMO_BONUS_FRAMEINDEX,
		.projectileNumberOfFrames = 1,		//
		.projectileDelayBetweenFrames = 0,
		.damageHole = DAMAGEMASK16_DAMAGEMASK16_FRAMEINDEX,		//
		.directHitDamage = 25,
		.directHitSquaredRadius = 25,
		.autoSetOffTime = 0,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = MINE_SET_OFF_SQUARE_RADIUS,
		.xBounceFract = 0x4000,			// multiplied by the current multiplier in game_t structure
		.yBounceFract = 0x2000,			// multiplied by the current multiplier in game_t structure
		.halfWidth = 3,
		.maxSpeed = 0x0,
		.setOffProjectileType = PROJECTILE_SMOKE16 | PROJECTILE_TYPE_EFFECT,
		.setOffSecondaryProjectileType = PROJECTILE_BOMBLET,
		.setOffSecondaryProjectileNumber = 1,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = PATCH_EXPLOSION		
	},
	{ // 23 BLOWTORCH UP_RIGHT
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = BLOWTORCH_DAMAGEMASK_UP_RIGHT_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},
	{ // 24 BLOWTORCH DOWN_RIGHT
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = BLOWTORCH_DAMAGEMASK_DOWN_RIGHT_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},	
	{ // 25 BLOWTORCH UP_LEFT
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = BLOWTORCH_DAMAGEMASK_UP_LEFT_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},
	{ // 26 BLOWTORCH DOWN_LEFT
		.flags = PROJECTILE_FLAGS_IMMEDIATE_SETOFF | PROJECTILE_FLAGS_INVISIBLE | PROJECTILE_FLAGS_NO_SELF_DAMAGE,
		.projectileFrameIndex = 0,
		.projectileNumberOfFrames = 1,		// no animation
		.projectileDelayBetweenFrames = 0,
		.damageHole = BLOWTORCH_DAMAGEMASK_DOWN_LEFT_FRAMEINDEX,		//
		.directHitDamage = 5,
		.directHitSquaredRadius = 25,
		.damageSquaredRadius = 256,
		.proximitySetOffSquareRadius = 64,
		.xBounceFract = 0x4000,
		.yBounceFract = 0x4000,
		.halfWidth = 1,
		.maxSpeed = 0x00,
		.setOffProjectileType = 0,
		.setOffSecondaryProjectileNumber = 0,
		.bounceSoundPatch = -1,
		.setOffSoundPatch = -1		
	},	
};
const weapon_t weaponsData[] =
{
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {0, 0, 0},
		.preShootFrameNumbers = {0, 0, 0},
		.postShootFrameNumbers = {0, 0, 0},
		.fireAnimDelay =0,
		.flags = 0,
		.afterFirePlayerControlTime = 0,
		.projectileType = 0,
		.weaponSymbols = {40, 41, 42, 43},
		.name = "NONE"
	},
	{
		.walkFrameIndex = {PLAYER_WALK_BAZOOKA_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_BAZOOKA_NUMFRAMES, PLAYER_WALK_BAZOOKA_NUMFRAMES, PLAYER_WALK_BAZOOKA_NUMFRAMES},
		.aimFrameIndex = {PLAYER_BAZOOKA_AIM_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX + 6,  PLAYER_BAZOOKA_AIM_FRAMEINDEX + 1},
		.shootFrameIndex = {PLAYER_BAZOOKA_SHOOT_UP_FRAMEINDEX, PLAYER_BAZOOKA_SHOOT_STRAIGHT_FRAMEINDEX, PLAYER_BAZOOKA_SHOOT_DOWN_FRAMEINDEX},
		.preShootFrameNumbers = {BAZOOKA_PRE_FIRE_ANIMATION, BAZOOKA_PRE_FIRE_ANIMATION, BAZOOKA_PRE_FIRE_ANIMATION},
		.postShootFrameNumbers = {BAZOOKA_POST_FIRE_ANIMATION, BAZOOKA_POST_FIRE_ANIMATION, BAZOOKA_POST_FIRE_ANIMATION},
		.fireAnimDelay =0,
		.flags = 0,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 1,
		.delayBetweenRounds = 30,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_ROCKET,
		.weaponSymbols = {40, 41, 42, 43},
		.name = "BAZOOKA",
		.preShootSoundPatch = PATCH_BAZOOKA,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1
	},
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX,PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_THROW_GRENADE_FRAMEINDEX, PLAYER_THROW_GRENADE_FRAMEINDEX, PLAYER_THROW_GRENADE_FRAMEINDEX},
		.preShootFrameNumbers = {PLAYER_THROW_GRENADE_NUMFRAMES - 1,PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1},
		.postShootFrameNumbers = {PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1},
		.fireAnimDelay = 2,
		.flags = WEAPON_FLAGS_GRENADE_TYPE,
		.numberOfManualRounds = 1,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_GRENADE,
		.weaponSymbols = {44, 45, 31, 26},
		.name = "GRENADE",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = PATCH_THROWGRENADE		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {0, 0, 0},
		.preShootFrameNumbers = {0, 0, 0},
		.postShootFrameNumbers = {0, 0, 0},
		.fireAnimDelay = 0,
		.flags = WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_NO_AIMPOINTER,
		.afterFirePlayerControlTime = DYNAMITE_CONTROL_TIME,
		.projectileType = PROJECTILE_DYNAMITE,
		.weaponSymbols = {48, 49, 50, 51},
		.name = "DYNAMITE",
		.preShootSoundPatch = PATCH_FUSE,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},	
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_THROW_GRENADE_FRAMEINDEX, PLAYER_THROW_GRENADE_FRAMEINDEX, PLAYER_THROW_GRENADE_FRAMEINDEX},
		.preShootFrameNumbers = {PLAYER_THROW_GRENADE_NUMFRAMES - 1,PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1},
		.postShootFrameNumbers = {PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1},
		.fireAnimDelay = 2,
		.flags = WEAPON_FLAGS_GRENADE_TYPE,
		.numberOfManualRounds = 1,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_CLUSTER_BOMB,
		.weaponSymbols = {52, 53, 38, 33}, 
		.name = "CLUSTER BOMB",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = PATCH_THROWGRENADE		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_THROW_GRENADE_FRAMEINDEX, PLAYER_THROW_GRENADE_FRAMEINDEX, PLAYER_THROW_GRENADE_FRAMEINDEX},
		.preShootFrameNumbers = {PLAYER_THROW_GRENADE_NUMFRAMES - 1,PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1},
		.postShootFrameNumbers = {PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1, PLAYER_THROW_GRENADE_NUMFRAMES - 1},
		.fireAnimDelay =0,
		.flags = WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_NO_AIMPOINTER,
		.numberOfManualRounds = 1,
		.afterFirePlayerControlTime = MINE_CONTROL_TIME,
		.projectileType = PROJECTILE_INACTIVE_MINE,
		.weaponSymbols = {72, 73, 74, 75}, 
		.name = "MINE",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_SHOTGUN_FRAMEINDEX, PLAYER_WALK_SHOTGUN_FRAMEINDEX, PLAYER_WALK_SHOTGUN_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_SHOTGUN_NUMFRAMES, PLAYER_WALK_SHOTGUN_NUMFRAMES, PLAYER_WALK_SHOTGUN_NUMFRAMES},
		.aimFrameIndex = {PLAYER_SHOTGUN_SHOOT_UP_FRAMEINDEX, PLAYER_SHOTGUN_SHOOT_STRAIGHT_FRAMEINDEX,  PLAYER_SHOTGUN_SHOOT_DOWN_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_SHOTGUN_SHOOT_UP_FRAMEINDEX, PLAYER_SHOTGUN_SHOOT_STRAIGHT_FRAMEINDEX, PLAYER_SHOTGUN_SHOOT_DOWN_FRAMEINDEX},
		.preShootFrameNumbers = {7, 7, 7},
		.postShootFrameNumbers = {PLAYER_SHOTGUN_SHOOT_UP_NUMFRAMES, PLAYER_SHOTGUN_SHOOT_STRAIGHT_NUMFRAMES, PLAYER_SHOTGUN_SHOOT_DOWN_NUMFRAMES},
		.fireAnimDelay = 3,
		.flags = WEAPON_FLAGS_FIXED_LAUNCH_FORCE,
		.numberOfManualRounds = 2,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_SHOTGUN_ROUND,
		.weaponSymbols = {56, 57, 58, 59}, 
		.name = "SHOTGUN",
		.preShootSoundPatch = PATCH_RELOAD,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = PATCH_SHOTGUN		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.preShootFrameNumbers = {0, 0, 0},
		.postShootFrameNumbers = {0, 0, 0},
		.fireAnimDelay = 3,
		.flags = WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_POINTER_REQUIRED | WEAPON_FLAGS_AIRDROPPED | WEAPON_FLAGS_AUTOMATIC | WEAPON_FLAGS_NO_AIMPOINTER,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 4,
		.delayBetweenRounds = 10,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_AIRSTRIKE_ROCKET,
		.weaponSymbols = {76, 77, 78, 79},	 
		.name = "AIRSTRIKE",
		.preShootSoundPatch = PATCH_BAZOOKA,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_TELEPORT_FRAMEINDEX, PLAYER_TELEPORT_FRAMEINDEX, PLAYER_TELEPORT_FRAMEINDEX},
		.preShootFrameNumbers = {13, 13, 13},
		.postShootFrameNumbers = {19, 19, 19},
		.fireAnimDelay = 4,
		.flags = WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_POINTER_REQUIRED | WEAPON_FLAGS_TELEPORT | WEAPON_FLAGS_NO_AIMPOINTER,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 0,
		.delayBetweenRounds = 0,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_TELEPORT,
		.weaponSymbols = {68, 69, 70, 71},
		.name = "TELEPORT",
		.preShootSoundPatch = PATCH_TELEPORT,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_BAZOOKA_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_BAZOOKA_NUMFRAMES, PLAYER_WALK_BAZOOKA_NUMFRAMES, PLAYER_WALK_BAZOOKA_NUMFRAMES},
		.aimFrameIndex = {PLAYER_BAZOOKA_AIM_FRAMEINDEX, PLAYER_WALK_BAZOOKA_FRAMEINDEX + 6,  PLAYER_BAZOOKA_AIM_FRAMEINDEX + 1},
		.shootFrameIndex = {PLAYER_BAZOOKA_SHOOT_UP_FRAMEINDEX, PLAYER_BAZOOKA_SHOOT_STRAIGHT_FRAMEINDEX, PLAYER_BAZOOKA_SHOOT_DOWN_FRAMEINDEX},
		.preShootFrameNumbers = {BAZOOKA_PRE_FIRE_ANIMATION, BAZOOKA_PRE_FIRE_ANIMATION, BAZOOKA_PRE_FIRE_ANIMATION},
		.postShootFrameNumbers = {BAZOOKA_POST_FIRE_ANIMATION, BAZOOKA_POST_FIRE_ANIMATION, BAZOOKA_POST_FIRE_ANIMATION},
		.fireAnimDelay = 0,
		.flags = WEAPON_FLAGS_AUTOMATIC,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 4,
		.delayBetweenRounds = 5,
		.afterFirePlayerControlTime = 0,
		.maxAngleError = 20,
		.projectileType = PROJECTILE_ROCKET,
		.weaponSymbols = {80, 81, 82, 83},
		.name = "DEVASTATOR",
		.preShootSoundPatch = PATCH_BAZOOKA,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},	
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.preShootFrameNumbers = {0, 0, 0},
		.postShootFrameNumbers = {0, 0, 0},
		.fireAnimDelay = 0,
		.flags = WEAPON_FLAGS_SKIP_TURN | WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_NO_AIMPOINTER,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 4,
		.delayBetweenRounds = 30,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_ROCKET,
		.weaponSymbols = {84, 85, 86, 87},
		.name = "SKIP TURN",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},	
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.preShootFrameNumbers = {0, 0, 0},
		.postShootFrameNumbers = {0, 0, 0},
		.fireAnimDelay = 0,
		.flags = WEAPON_FLAGS_SURRENDER | WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_NO_AIMPOINTER,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 4,
		.delayBetweenRounds = 30,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_ROCKET,
		.weaponSymbols = {88, 89, 90, 91},
		.name = "SURRENDER",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1	
	},
	{
		.walkFrameIndex = {PLAYER_WALK_BLOWTORCH_FRAMEINDEX, PLAYER_WALK_BLOWTORCH_FRAMEINDEX, PLAYER_WALK_BLOWTORCH_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_BLOWTORCH_FRAMEINDEX, PLAYER_WALK_BLOWTORCH_FRAMEINDEX,  PLAYER_WALK_BLOWTORCH_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_BLOWTORCH_IGNITE_UP_FRAMEINDEX, PLAYER_BLOWTORCH_IGNITE_STRAIGHT_FRAMEINDEX, PLAYER_BLOWTORCH_IGNITE_DOWN_FRAMEINDEX},
		.preShootFrameNumbers = {9, 9, 9},
		.duringShootFrameNumbers = {9 + 12, 9 + 12, 9 + 12},
		.postShootFrameNumbers = {9 + 12 + 3, 9 + 12 + 3, 9 + 12 + 3},
		.fireAnimDelay = 4,
		.flags = WEAPON_FLAGS_AUTOMATIC | /*WEPAON_FLAGS_PLAYER_WALKS_DURING_SHOOT |*/ WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_HAS_DISCRETE_DIRECTIONS | WEAPON_FLAGS_BLOWTORCH,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 4,
		.delayBetweenRounds = 16,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_BLOWTORCH,
		.weaponSymbols = {92, 93, 94, 95},	
		.name = "BLOW TORCH",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = PATCH_BLOWTORCH,
		.postShootSoundPatch = -1		
	},	
	{
		.walkFrameIndex = {PLAYER_WALK_SHOTGUN_FRAMEINDEX, PLAYER_WALK_SHOTGUN_FRAMEINDEX, PLAYER_WALK_SHOTGUN_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_SHOTGUN_NUMFRAMES, PLAYER_WALK_SHOTGUN_NUMFRAMES, PLAYER_WALK_SHOTGUN_NUMFRAMES},
		.aimFrameIndex = {PLAYER_SHOTGUN_SHOOT_UP_FRAMEINDEX, PLAYER_SHOTGUN_SHOOT_STRAIGHT_FRAMEINDEX,  PLAYER_SHOTGUN_SHOOT_DOWN_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_SHOTGUN_SHOOT_UP_FRAMEINDEX + 6, PLAYER_SHOTGUN_SHOOT_STRAIGHT_FRAMEINDEX + 6, PLAYER_SHOTGUN_SHOOT_DOWN_FRAMEINDEX + 6},
		.preShootFrameNumbers = {2, 2, 2},
		.postShootFrameNumbers = {2, 2, 2},
		.fireAnimDelay = 1,
		.flags = WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_AUTOMATIC,
		.numberOfManualRounds = 1,
		.maxAngleError = 28,
		.delayBetweenRounds = 0,		
		.numberOfAutomaticRounds = 20,		
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_MACHINEGUN_ROUND,
		.weaponSymbols = {60, 61, 62, 63},
		.name = "MACHINE GUN",
		.preShootSoundPatch = PATCH_MACHINE_GUN,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},	
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_DRILL_ANIM_FRAMEINDEX, PLAYER_DRILL_ANIM_FRAMEINDEX, PLAYER_DRILL_ANIM_FRAMEINDEX},
		.preShootFrameNumbers = {6, 6, 6},
		.duringShootFrameNumbers = {9, 9 , 9},
		.postShootFrameNumbers = {PLAYER_DRILL_ANIM_NUMFRAMES, PLAYER_DRILL_ANIM_NUMFRAMES, PLAYER_DRILL_ANIM_NUMFRAMES},
		.fireAnimDelay = 4,
		.flags = WEAPON_FLAGS_AUTOMATIC | WEAPON_FLAGS_FIXED_LAUNCH_FORCE | WEAPON_FLAGS_NO_AIMPOINTER | WEAPON_FLAGS_DRILL,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 40,
		.delayBetweenRounds = 40,
		.afterFirePlayerControlTime = 0,
		.projectileType = PROJECTILE_DRILL,
		.weaponSymbols = {96, 97, 98, 99},
		.name = "PNEUMATIC DRILL",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = PATCH_DRILL,
		.postShootSoundPatch = -1		
	},
	{
		.walkFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.walkFrameNumbers = {PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES, PLAYER_WALK_NUMFRAMES},
		.aimFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX,  PLAYER_WALK_FRAMEINDEX},
		.shootFrameIndex = {PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX, PLAYER_WALK_FRAMEINDEX},
		.preShootFrameNumbers = {0, 0, 0},
		.duringShootFrameNumbers = {0, 0 , 0},
		.postShootFrameNumbers = {0, 0, 0},
		.fireAnimDelay = 4,
		.flags = WEAPON_FLAGS_GIRDERS | WEAPON_FLAGS_NO_AIMPOINTER,
		.numberOfManualRounds = 1,
		.numberOfAutomaticRounds = 0,
		.delayBetweenRounds = 0,
		.afterFirePlayerControlTime = 0,
		.projectileType = 0,
		.weaponSymbols = {64, 65, 66, 67},	
		.name = "GIRDERS",
		.preShootSoundPatch = -1,
		.duringShootSoundPatch = -1,
		.postShootSoundPatch = -1		
	},	
};
typedef struct
{
	uint8_t health[MAX_TEAM_UNITS];
	uint8_t damage[MAX_TEAM_UNITS];
	uint8_t firstUnitIndex;
	uint8_t selectedUnit;
	int8_t inventory[NUMBER_OF_WEAPONS];
	uint8_t type;
	uint8_t wonRounds;	
} team_t;
camera_unit_t cameraUnit;
typedef struct
{
	unit_t *focusedUnit;
	int32_t lastViewX;
	int32_t lastViewY;
	ai_t aiData;
	uint16_t stateTime;
	uint16_t bombDeployTime;		// frames since a bomb/mine/dynamite has been deployed.
	int16_t xPointer;				// for pointer based weapon as teleport or airstrike
	int16_t yPointer;				// for pointer based weapon as teleport or airstrike
	uint8_t deadMask;				// to easily detect deaths.
	uint8_t turnTime;				// in seconds
	uint8_t tickTurnTime;			// in frames to count each second
	int8_t windSpeed;
	uint8_t gameState;
	uint8_t gameSubState;
	uint8_t currentTeam;
	int8_t currentAimAngle;		// -128 = up. 127 = down
	uint8_t firingForce;		// 0.8 fixed point
	uint8_t currentWeapon;
	uint8_t fireRound;			// number of the fire round
	uint8_t automaticRoundsFired;
	uint8_t configBounceMultiplier;
	uint8_t configTimeMultiplier;
	uint8_t screenShake;
	uint8_t numberOfUnitsPerTeam;
	uint8_t numberOfTeams;
	uint8_t aiKey;
	int8_t yExplosionShakeOffset;
	uint8_t showCurrentUnitIndicator;
	uint8_t menuPage;
	uint8_t selectedMenuItem;
	uint8_t numberOfRoundsPerMatch : 4;
	uint8_t numberOfRoundsPlayed : 4;
} game_t;
game_t game;
team_t teams[MAX_TEAMS];
// defines and templates for creating the map
#define PROBABILITY_TURN_RIGHT 20
#define PROBABILITY_TURN_LEFT 5
#define MAX_TEMPLATE_SIZE 24
#define MAX_COMPATIBLE_TEMPLATE_NUMBER 10
#define TEMPLATE_DIRECTION_MASK 0xF
#define NUMBER_OF_TEMPLATES 25

enum
{
	DIRECTION_UP = 1,
	DIRECTION_DOWN = 2,
	DIRECTION_LEFT = 4,
	DIRECTION_RIGHT = 8,
	DIRECTION_RIGHTUP = DIRECTION_UP | DIRECTION_RIGHT,
	DIRECTION_RIGHTDOWN = DIRECTION_DOWN | DIRECTION_RIGHT,
	DIRECTION_LEFTUP = DIRECTION_UP | DIRECTION_LEFT,
	DIRECTION_LEFTDOWN = DIRECTION_DOWN | DIRECTION_LEFT,
	DIRECTION_NOTUP = DIRECTION_DOWN | DIRECTION_LEFT | DIRECTION_RIGHT,
	DIRECTION_NOTDOWN = DIRECTION_UP | DIRECTION_LEFT | DIRECTION_RIGHT,
	DIRECTION_NOTRIGHT = DIRECTION_DOWN | DIRECTION_LEFT | DIRECTION_UP,
	DIRECTION_NOTLEFT = DIRECTION_DOWN | DIRECTION_UP | DIRECTION_RIGHT,
	TEMPLATE_FLAGS_UPWARD = 128
};

typedef struct
{
	uint8_t size;
	uint8_t templateIndexes[MAX_COMPATIBLE_TEMPLATE_NUMBER];
} compatibleTemplateIndexArray_t;
typedef struct
{
	uint8_t tileIndexes[MAX_TEMPLATE_SIZE];
	int8_t dx;
	int8_t dy;
	uint8_t sizeX;
	uint8_t sizeY;
	uint8_t direction;
} mapTemplate_t;
const int16_t templateAngle[16] =
{
	-1, 90, 270, -1,
	180, 135, 180 + 45, -1,
	0, 45, 270 + 45, -1,
	-1, -1, -1, -1
};
extern const compatibleTemplateIndexArray_t compatibleTemplates[NUMBER_OF_TEMPLATES];
extern const mapTemplate_t mapTemplates[NUMBER_OF_TEMPLATES];
const mapTemplate_t mapTemplates[NUMBER_OF_TEMPLATES] =
{
	{.dx = 2, .dy = -1, .sizeX = 2, .sizeY = 2, .direction = 9 ,.tileIndexes = {0, 37, 39, 40}},
	{.dx = 2, .dy = 1, .sizeX = 2, .sizeY = 2, .direction = 10 ,.tileIndexes = {38, 0, 41, 42}},
	{.dx = 0, .dy = -2, .sizeX = 1, .sizeY = 2, .direction = 9 ,.tileIndexes = {43, 45}},
	{.dx = 1, .dy = 1, .sizeX = 1, .sizeY = 2, .direction = 10 ,.tileIndexes = {44, 46}},
	{.dx = 1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 8 ,.tileIndexes = {18}},
	{.dx = 1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 8 ,.tileIndexes = {19}},
	{.dx = 1, .dy = -2, .sizeX = 2, .sizeY = 2, .direction = 9 ,.tileIndexes = {17, 47, 49, 0}},
	{.dx = 2, .dy = 1, .sizeX = 2, .sizeY = 2, .direction = 10 ,.tileIndexes = {48, 22, 0, 50}},
	{.dx = 0, .dy = -1, .sizeX = 1, .sizeY = 1, .direction = 1 ,.tileIndexes = {23}},
	{.dx = 0, .dy = 1, .sizeX = 1, .sizeY = 1, .direction = 2 ,.tileIndexes = {26}},
	{.dx = -2, .dy = -1, .sizeX = 2, .sizeY = 2, .direction = 5 ,.tileIndexes = {51, 52, 0, 55}},
	{.dx = -2, .dy = 1, .sizeX = 2, .sizeY = 2, .direction = 6 ,.tileIndexes = {53, 54, 56, 0}},
	{.dx = -1, .dy = -1, .sizeX = 1, .sizeY = 2, .direction = 5 ,.tileIndexes = {57, 59}},
	{.dx = 0, .dy = 2, .sizeX = 1, .sizeY = 2, .direction = 6 ,.tileIndexes = {58, 60}},
	{.dx = -1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 4 ,.tileIndexes = {32}},
	{.dx = -1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 4 ,.tileIndexes = {33}},
	{.dx = -2, .dy = -1, .sizeX = 2, .sizeY = 2, .direction = 5 ,.tileIndexes = {63, 0, 61, 65}},
	{.dx = -1, .dy = 2, .sizeX = 2, .sizeY = 2, .direction = 6 ,.tileIndexes = {0, 64, 66, 62}},
	{.dx = 1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 8 ,.tileIndexes = {17}},
	{.dx = 0, .dy = 1, .sizeX = 1, .sizeY = 1, .direction = 2 ,.tileIndexes = {22}},
	{.dx = -1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 4 ,.tileIndexes = {36}},
	{.dx = 0, .dy = -1, .sizeX = 1, .sizeY = 1, .direction = 1 ,.tileIndexes = {31}},
	{.dx = 0, .dy = -1, .sizeX = 1, .sizeY = 1, .direction = 1 ,.tileIndexes = {61}},
	{.dx = -1, .dy = 0, .sizeX = 1, .sizeY = 1, .direction = 4 ,.tileIndexes = {62}},
	{.dx = 4, .dy = 0, .sizeX = 4, .sizeY = 6, .direction = 136 ,.tileIndexes = {0, 77, 78, 0, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 18, 95, 96, 21}}
};
const compatibleTemplateIndexArray_t compatibleTemplates[NUMBER_OF_TEMPLATES] =
{
	{.size = 8, .templateIndexes = {0, 1, 2, 4, 5, 6, 19, 24}},
	{.size = 8, .templateIndexes = {0, 1, 2, 4, 5, 6, 19, 24}},
	{.size = 4, .templateIndexes = {12, 16, 8, 18}},
	{.size = 7, .templateIndexes = {0, 1, 2, 4, 5, 6, 19}},
	{.size = 8, .templateIndexes = {0, 1, 2, 4, 5, 6, 19, 24}},
	{.size = 8, .templateIndexes = {0, 1, 2, 4, 5, 6, 19, 24}},
	{.size = 4, .templateIndexes = {12, 16, 8, 18}},
	{.size = 7, .templateIndexes = {0, 1, 2, 4, 5, 6, 19}},
	{.size = 4, .templateIndexes = {12, 16, 8, 18}},
	{.size = 4, .templateIndexes = {3, 7, 9, 20}},
	{.size = 7, .templateIndexes = {10, 11, 13, 14, 15, 17, 21}},
	{.size = 8, .templateIndexes = {10, 11, 13, 14, 15, 17, 21, 22}},
	{.size = 7, .templateIndexes = {10, 11, 13, 14, 15, 17, 22}},
	{.size = 4, .templateIndexes = {3, 7, 9, 23}},
	{.size = 8, .templateIndexes = {10, 11, 13, 14, 15, 17, 21, 22}},
	{.size = 8, .templateIndexes = {10, 11, 13, 14, 15, 17, 21, 22}},
	{.size = 7, .templateIndexes = {10, 11, 13, 14, 15, 17, 22}},
	{.size = 4, .templateIndexes = {3, 7, 9, 23}},
	{.size = 8, .templateIndexes = {0, 1, 2, 4, 5, 6, 19, 24}},
	{.size = 4, .templateIndexes = {3, 7, 9, 20}},
	{.size = 8, .templateIndexes = {10, 11, 13, 14, 15, 17, 21, 22}},
	{.size = 2, .templateIndexes = {8, 18}},
	{.size = 4, .templateIndexes = {12, 16, 8, 18}},
	{.size = 8, .templateIndexes = {10, 11, 13, 14, 15, 17, 21, 22}},
	{.size = 6, .templateIndexes = {0, 1, 4, 5, 6, 19}}
};

unit_t projectileUnits[MAX_INGAME_PROJECTILES];
staticUnit_t staticUnits[MAX_STATIC_UNITS];
uint16_t getMapTileFromTileCoordinates(uint8_t levelN,uint16_t xTile,  uint16_t yTile)
{
	const level_t *pLevel = &levels[levelN];
	#if LEVELS_CAN_USE_META_TILE
	if (pLevel->useMetaTiles)
	{
		uint16_t metaNumber = pLevel->pGameMap[(xTile >> 1) + (yTile >> 1) * pLevel->mapSizeX ];
		return pLevel->pMetaTiles[metaNumber * 4 + (xTile & 1) + 2 * (yTile & 1)];
	}
	else
	#endif
	return pLevel->pGameMap[xTile + yTile * pLevel->mapSizeX];
}
inline uint32_t getRamMapTileFromTileCoordinates(uint16_t xTile,  uint16_t yTile)
{
	int n = xTile + yTile * MAX_MAP_SIZE_X;
	if (gameRamMap.highBitTileNumber[n >> 3] & (1 << (n & 7)))
		return 256 | gameRamMap.loTileNumber[n];
	return gameRamMap.loTileNumber[n];
}
inline uint8_t getMapPixel(int x, int y)
{
	if (y < 0 /*|| x < 0 || x >= MAX_MAP_SIZE_X *8*/ || (y > MAX_MAP_SIZE_Y * 8 - 1))
		return 0;
	if (x < 0)
		x = 0;
	if (x > MAX_MAP_SIZE_X *8 - 1)
		x = MAX_MAP_SIZE_X *8 - 1;
	// get tile
	uint8_t *pTile  = (uint8_t *) &tiles[getRamMapTileFromTileCoordinates((x >> LOG2_TILE_SIZE_X ), (y >> LOG2_TILE_SIZE_Y))];
	// get pixel of tile
	uint8_t pixel = pTile[ ((x & 7) >> 1) + (y & 7) * (TILE_SIZE_X / 2) ];
	if (x & 1)
		return pixel >> 4;
	else 
		return  pixel & 0xF;
}
// Since this game requires us having a Ram Map, we exploit this fact save some flash rom.
void copyMapToRam(uint8_t levelN, uint16_t sx, uint16_t sy, uint16_t dx, uint16_t dy, uint16_t sw, uint16_t sh)
{
	// first clear the ram map.
	memset(&gameRamMap, 0, sizeof(gameRamMap));
	// then copy the source
	//
	for (int y = 0; y < sh ; y++)
	{
		for (int x = 0; x < sw; x++)
		{
			int n = (x + dx) + (y + dy) * MAX_MAP_SIZE_X;
			uint16_t tileNumber = getMapTileFromTileCoordinates(levelN, x + sx, y + sy);
			gameRamMap.loTileNumber[n] = tileNumber;
			if (tileNumber & (1 << 8))
				gameRamMap.highBitTileNumber[n >> 3] |= 1 << (n & 7);
			else
				gameRamMap.highBitTileNumber[n >> 3] &= ~ (1 << (n & 7));
			
		}
	}
}
//
void calculateInitialProjectileData( projectileData_t *pid, unit_t *pUnit, int8_t angle , uint8_t shootRight, uint8_t weapon, uint8_t force)
{
	int currentAngle = (shootRight ? angle + 128 :  128 + 255 - angle ) << 1;
	int deltaX = 0, deltaY = 0;
	uint8_t projectileType = weaponsData[weapon].projectileType;
	int32_t speed = projectiles[projectileType].maxSpeed << (FRACTIONAL_PART - PROJECTILE_MAXSPEED_FRACTIONAL_PART);
	if (!(projectiles[projectileType].flags & PROJECTILE_FLAGS_FIXED_LAUNCH_SPEED))
	{
		speed = (speed * force) >> 8;
	}
	if (weaponsData[weapon].flags & WEAPON_FLAGS_NO_AIMPOINTER)
	{
		deltaX = shootRight  ? PROJECTILE_FIRING_DISTANCE : - PROJECTILE_FIRING_DISTANCE;
		deltaY = - PROJECTILE_FIRING_DISTANCE;
	}
	else
	{
		deltaX = (FIXED_MULFRACT(PROJECTILE_FIRING_DISTANCE, sinTable[currentAngle]) + (shootRight ? PROJECTILE_FIRING_DISTANCE : - PROJECTILE_FIRING_DISTANCE));
		deltaY = - FIXED_MULFRACT(PROJECTILE_FIRING_DISTANCE, sinTable[(currentAngle + 256) & 1023]);
	}
	pid->vx  = FIXED_MULFRACT(speed, sinTable[currentAngle]);
	pid->vy = - FIXED_MULFRACT(speed , sinTable[(currentAngle + 256) & 1023]);
	pid->x = pUnit->x + deltaX;
	pid->y =  pUnit->y - ((PLAYER_HEIGHT / 2) << FRACTIONAL_PART) + deltaY; 
}
uint8_t evaluateAiMovement(unit_t *pUnit, uint8_t action, uint16_t maxFrameLine)
{
	// 
	uint8_t result = 0;
	uint8_t oldAction = action;
	uint16_t oldFrameLine = videoData.currentLineNumber;
	while (videoData.currentLineNumber < maxFrameLine && oldFrameLine <= videoData.currentLineNumber)
	{
		oldFrameLine = videoData.currentLineNumber;
		result = playerHandler(pUnit, action, 1);
		action = COMMAND_NONE;	// only the first operation is triggered
		// as a first approximation, we always consider that hitting a mine is a bad idea. Is some cases, though, it might be a good idea (e.g. if you have an opponent with 1 health and you have a full health team mate far away)
		for (int s = 0; s < MAX_STATIC_UNITS; s++)
		{
			if (staticUnits[s].type == STATIC_UNIT_TYPE_MINE)
			{				
				int xU = pUnit->x >> FRACTIONAL_PART;
				//int yU = pUnit->y >> FRACTIONAL_PART;
				int yU = (pUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;
				int xS = staticUnits[s].x;
				int yS = staticUnits[s].y;
				int squareDistance = (xS - xU) * (xS - xU) + (yS - yU) * (yS - yU);
				if ( squareDistance  <= projectiles[PROJECTILE_EXPLODING_MINE].proximitySetOffSquareRadius)
				{				
					pUnit->state = PLAYER_STATE_DEAD;
					break;	
				}			
			}
		}
		if ((result == 0 && !(pUnit->state & (PLAYER_STATE_FALLING | PLAYER_STATE_JUMPING))) || (pUnit->state & PLAYER_STATE_DEAD))
			break;
	}
	if ((pUnit->state & PLAYER_STATE_DEAD) || ((pUnit->y >> FRACTIONAL_PART) >= WATER_LEVEL + PLAYER_HEIGHT))
	{
		return AI_MOVEMENT_BAD_DECISION;
	}
	if ((oldAction == COMMAND_LEFT || oldAction == COMMAND_RIGHT) && (pUnit->state & (PLAYER_STATE_WALKING | PLAYER_STATE_IDLE)) == (PLAYER_STATE_WALKING | PLAYER_STATE_IDLE))
		return AI_MOVEMENT_CANNOT_MOVE;
	if (videoData.currentLineNumber >= maxFrameLine || oldFrameLine > videoData.currentLineNumber)
		return AI_MOVEMENT_CANNOT_EVALUATE;		
	return AI_MOVEMENT_POSSIBLE;
}
uint8_t movePlayer(unit_t *pUnit, uint8_t isSimulating)
{
	// check if there is room to move
	int newX = pUnit->x + pUnit->vx;
	int newY = pUnit->y + pUnit->vy;
	uint8_t state = pUnit->state;
	// is it possible to move there?
	if (state & PLAYER_STATE_DEAD)
		return 0;
	if (!isSimulating && ((newY >> FRACTIONAL_PART) >= WATER_LEVEL + PLAYER_HEIGHT) && ((pUnit->y >> FRACTIONAL_PART) < WATER_LEVEL + PLAYER_HEIGHT))
		triggerFx(PATCH_WATER, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);
	if ((pUnit->y >> FRACTIONAL_PART) >= WATER_LEVEL + PLAYER_HEIGHT)
	{ 
		pUnit->vx = FIXED_MULFRACT(pUnit->vx, PLAYER_WATER_DRAG_FACTOR);		
		if (pUnit->vy >= PLAYER_WATER_TERMINAL_VELOCITY)
			pUnit->vy  = PLAYER_WATER_TERMINAL_VELOCITY;
	}
	else if (pUnit->vy > ((customLevelData_t *) levels[currentLevel].customData)->terminalVelocity)
		pUnit->vy = ((customLevelData_t *) levels[currentLevel].customData)->terminalVelocity;
	if (state & PLAYER_STATE_FIRING)
	{	
		if (getMapPixel(pUnit->x >> FRACTIONAL_PART, 1 + (pUnit->y >> FRACTIONAL_PART)))
		{
			pUnit->vy = 0;
		}
//		if (pUnit->vx == 0 && !(pUnit->state & ~(PLAYER_STATE_WALKING | PLAYER_FACING_RIGHT)))	
//			return 0;
	}
	if ((state & PLAYER_STATE_IDLE))
	{
		// let's see if there is something underneath him.
		if (getMapPixel(pUnit->x >> FRACTIONAL_PART, 1 + (pUnit->y >> FRACTIONAL_PART)) || getMapPixel(pUnit->x >> FRACTIONAL_PART, (pUnit->y >> FRACTIONAL_PART)) )
		{
			pUnit->vy = 0;
			return 0;
		}
		else
		{
			state = PLAYER_STATE_JUMPING | (state & PLAYER_FACING_RIGHT);
			pUnit->state = PLAYER_STATE_JUMPING | (state & PLAYER_FACING_RIGHT);
		}
	}
	if (state & PLAYER_STATE_WALKING)
	{
		/* The player can go left/right if at the new X position there is some pixel, that:
			1) Is no more thar MAX_Y_STEP higher than the newY, otherwise the player state goes back to PLAYER_STATE_IDLE and the playe won't move.
			2) Is no more than MAX_Y_STEP lower than the newY, otherwise the player goes into jump state.
			3) There are at least PLAYER_HEIGHT pixels free.
		*/
		// clip newX to the playground area.
		if (newX < 0)
		{
			newX = 0;
			pUnit->state =  PLAYER_STATE_WALKING | PLAYER_STATE_IDLE | (state & PLAYER_FACING_RIGHT);
			pUnit->vx = 0;
			return 0;			
		}
		if (newX > ((MAX_MAP_SIZE_X * TILE_SIZE_X - 1) << FRACTIONAL_PART))
		{
			newX = (MAX_MAP_SIZE_X * TILE_SIZE_X - 1) << FRACTIONAL_PART;
			pUnit->state =  PLAYER_STATE_WALKING | PLAYER_STATE_IDLE | (state & PLAYER_FACING_RIGHT);
			pUnit->vx = 0;
			return 0;
		}
		// the algorithm is quite simple. We try to find a sequence of PLAYER_HEIGHT or more "0" pixels, followed by one "1" pixel
		int freePixels = 0;
		int8_t groundFound = 0;
		int yPos = newY >> FRACTIONAL_PART;
		int testY;
		for (testY = yPos - PLAYER_HEIGHT - MAX_Y_STEP; testY < yPos + MAX_Y_STEP + 1; testY++)
		{
			if (!getMapPixel(newX >> FRACTIONAL_PART, testY))
			{
				freePixels++;
			}
			else
			{
				if (freePixels >= PLAYER_HEIGHT)
				{
					// we found something to sit on
					groundFound = 1;
					break;
				}
				else 
				{
					freePixels = 0;
					if ( testY >= yPos - PLAYER_HEIGHT + MAX_Y_STEP)
					{
                        groundFound = -1;
						break;
					}
				}
			}
		}
		if (groundFound > 0)
		{
			newY = testY << FRACTIONAL_PART;
			pUnit->y = newY;
			pUnit->x = newX;
			return 0;			
		}
		else if (groundFound == 0)
		{
			pUnit->state =  PLAYER_STATE_JUMPING | (state & PLAYER_FACING_RIGHT);
			pUnit->vx = 0;
			pUnit->x = newX;
		}
		else
		{	// can't move...
			pUnit->state =  PLAYER_STATE_WALKING | PLAYER_STATE_IDLE | (state & PLAYER_FACING_RIGHT);
			pUnit->vx = 0;	
			return 0;		
		}
	}
	else if ((state & PLAYER_STATE_JUMPING) || (state & PLAYER_STATE_FALLING))
	{
		// if it is going to fast, then it is not jumping, but falling!
		if (pUnit->vy > JUMPING_MAXSPEED)
		{
			if (state & PLAYER_STATE_JUMPING)
				pUnit->lastFrameTick = 0; 
			state =  PLAYER_STATE_FALLING | (state & PLAYER_FACING_RIGHT);
			pUnit->state = state;
		}
		// check if there is free space where we should land
		// note: we assume that in the old position, there was room to stay.
		// before doing anything, let's see if the x speed is high enough to overcome a 1 pixel barrier.
		// let's see if we are falling and we have enough x speed to roll over a pixel
		uint8_t canRollOver = 0;
		if (state & PLAYER_STATE_FALLING)
		{
			if (pUnit->vx > ((customLevelData_t *) levels[currentLevel].customData)->minSpeedToRollOver || pUnit->vx < - ((customLevelData_t *) levels[currentLevel].customData)->minSpeedToRollOver)
			{
				canRollOver = 1;
			}
			// stop objects which have 0 speed and they are on the ground
			if (pUnit->vy > -UNIT_MIN_SPEED && pUnit->vy < UNIT_MIN_SPEED && pUnit->vx > -UNIT_MIN_SPEED && pUnit->vx < UNIT_MIN_SPEED && (getMapPixel(pUnit->x >> FRACTIONAL_PART, pUnit->y >> FRACTIONAL_PART) /* || getMapPixel(pUnit->x >> FRACTIONAL_PART, 1 +(pUnit->y >> FRACTIONAL_PART))*/))
			{
				pUnit->state = PLAYER_STATE_IDLE | (pUnit->state & PLAYER_FACING_RIGHT);				
				pUnit->vx = 0;
				pUnit->vy = 0;
				return 0;
			}
		}		
		if (pUnit->vy >= 0)
		{	
			int yTest, freePixels = 0;
			for (yTest = (pUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT + 1; yTest <= (newY >> FRACTIONAL_PART); yTest++)
			{
				if (!getMapPixel(newX >> FRACTIONAL_PART, yTest))
				{
					freePixels++;
				}
				else
				{  // found an obstacle
					if (freePixels >= PLAYER_HEIGHT)
					{
						// we found something to sit on
						if (state & PLAYER_STATE_JUMPING)
						{
							pUnit->state = (state & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
							pUnit->vy = 0;
							pUnit->vx = 0;
							newY = yTest << FRACTIONAL_PART;
						}
						else if (state & PLAYER_STATE_FALLING)
						{
							newY = yTest << FRACTIONAL_PART;
							//pUnit->vy = - (pUnit->vy >> 3);							
							//pUnit->vx = FIXED_MUL(pUnit->vx, 0xA000);
							pUnit->vy = - FIXED_MUL(pUnit->vy, ((customLevelData_t *) levels[currentLevel].customData)->playerYbounce);
							// when touching the ground, friction will occur. This is ground-dependent
							pUnit->vx = FIXED_MUL(pUnit->vx, ((customLevelData_t *) levels[currentLevel].customData)->playerXfriction);
							if (pUnit->vx > -UNIT_MIN_SPEED && pUnit->vx < UNIT_MIN_SPEED) 
								pUnit->vx = 0;
							if (pUnit->vy > -UNIT_MIN_SPEED) // initially vy >0. But it has been multiplied by a negative constant, so it is negative.
							{
								pUnit->vy = 0;
								if (pUnit->vx == 0)
									pUnit->state = (state & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
							}
							else
							{
								triggerFx(PATCH_BOING, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);	
							}

						}
						break;
					}
					else if (yTest >= (newY >> FRACTIONAL_PART) - PLAYER_HEIGHT)  // 
					{
						// there is no room in newX. We have two possibilities:
						// Can we roll over? Then try one pixel above the maximum and see if the free pixels is now >= player height
						if (canRollOver && freePixels == (PLAYER_HEIGHT - 1 ) && (!getMapPixel(newX >> FRACTIONAL_PART, yTest - PLAYER_HEIGHT)) )
						{
							// yes, we can roll over. We eat some Vx to gain in height, and we modify the speed so that it is jumping up at 45°
							if (pUnit->vx > 0)
							{
								pUnit->vx -= ((customLevelData_t *) levels[currentLevel].customData)->minSpeedToRollOver;
								int32_t vx = FIXED_MUL(pUnit->vx, 0xB505);	 // SQRT2/2					
								pUnit->vx = vx;
								pUnit->vy = - FIXED_MUL(pUnit->vy, ((customLevelData_t *) levels[currentLevel].customData)->playerYbounce) - vx;

							}
							else
							{
								pUnit->vx += ((customLevelData_t *) levels[currentLevel].customData)->minSpeedToRollOver;
								int32_t vx = FIXED_MUL(pUnit->vx, 0xB505); // SQRT2/2
								pUnit->vx = vx;
								pUnit->vy = - FIXED_MUL(pUnit->vy, ((customLevelData_t *) levels[currentLevel].customData)->playerYbounce) + vx;								
							}
							pUnit->y = (yTest - 1) << FRACTIONAL_PART;
							pUnit->x = newX;
							return 1;
						}
						// We couldn't roll over one pixel. Let's try in oldX.
						freePixels = 0;
//						pUnit->vx = -pUnit->vx;
						pUnit->vx = - FIXED_MUL(pUnit->vx, ((customLevelData_t *) levels[currentLevel].customData)->playerXbounce);
						for (yTest = (pUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT + 1; yTest <= (newY >> FRACTIONAL_PART); yTest++)
						{
							if (!getMapPixel(pUnit->x >> FRACTIONAL_PART, yTest))
							{
								freePixels++;
							}
							else
							{
								if (state & PLAYER_STATE_JUMPING)
								{
									pUnit->state = (state & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
									pUnit->vy = 0;
									pUnit->vx = 0;
								}
								else if (state & PLAYER_STATE_FALLING)
								{
									pUnit->vy = - FIXED_MUL(pUnit->vy, ((customLevelData_t *) levels[currentLevel].customData)->playerYbounce);
									if (pUnit->vy > -UNIT_MIN_SPEED) // initially vy >0. But it has been multiplied by a negative constant, so it is negative.
									{
										pUnit->vy = 0;
										if (pUnit->vx == 0)
											pUnit->state = (state & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
									}
									else
									{
										triggerFx(PATCH_BOING, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);										
									}
								}
								break;	
							}
						}
						newY = yTest << FRACTIONAL_PART;
						newX = pUnit->x;
						break;
					}
					else
					{
						// there was an obstacle but it might be too high for us to make concerns
						freePixels = 0;
					}
				}				
			}
		}
		else
		{	// still jumping/flying up
			int yTest, freePixels = 0;
			for (yTest =  (newY >> FRACTIONAL_PART) - PLAYER_HEIGHT + 1; yTest <= (pUnit->y >> FRACTIONAL_PART) ; yTest++)
			{
				if (!getMapPixel(newX >> FRACTIONAL_PART, yTest))
				{
					freePixels++;
					if (freePixels >= PLAYER_HEIGHT)
					{
						if ((newY >> FRACTIONAL_PART) != yTest) // did we bounce ?
							pUnit->vy = - pUnit->vy;							
						newY = yTest << FRACTIONAL_PART;
						break;
					}
				}
				else
				{  // found an obstacle
					if (yTest > (pUnit->y >> FRACTIONAL_PART)  - PLAYER_HEIGHT) // if for sure there is no enough space vertically, then check in the old X position
					{
						// there is no room in newX. Let's try in oldX (first check if we ca roll over)
						freePixels = 0;
						if (canRollOver && freePixels == (PLAYER_HEIGHT - 1 ) && (!getMapPixel(newX >> FRACTIONAL_PART, yTest - PLAYER_HEIGHT)) )
						{
							// yes, we can roll over. We eat some Vx to gain in height, and we modify the speed so that it is jumping up at 45°
							if (pUnit->vx > 0)
							{
								pUnit->vx -= ((customLevelData_t *) levels[currentLevel].customData)->minSpeedToRollOver;
								int32_t vx = FIXED_MUL(pUnit->vx, 0xB505); // SQRT2/2
								pUnit->vx = vx;
								pUnit->vy -= vx;

							}
							else
							{
								pUnit->vx += ((customLevelData_t *) levels[currentLevel].customData)->minSpeedToRollOver;
								int32_t vx = FIXED_MUL(pUnit->vx, 0xB505); // SQRT2/2
								pUnit->vx = vx;
								pUnit->vy +=  vx;
							}
							pUnit->y = (yTest - 1) << FRACTIONAL_PART;
							pUnit->x = newX;
							return 1;
						}
						//pUnit->vx = -pUnit->vx;
						pUnit->vx = -FIXED_MUL(pUnit->vx, ((customLevelData_t *) levels[currentLevel].customData)->playerXbounce);
						// now we scan from the bottom to top (in the old position there were for sure at least "PLAYER_HEIGHT" pixels free
						for (yTest = (pUnit->y >> FRACTIONAL_PART); yTest >= (newY >> FRACTIONAL_PART);  yTest--)
						{
							if (getMapPixel(pUnit->x >> FRACTIONAL_PART, yTest - PLAYER_HEIGHT))
							{
								pUnit->vy = - pUnit->vy;
								yTest++;
								triggerFx(PATCH_BOING, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);
								break;
							}
						}
						newY = yTest << FRACTIONAL_PART;
						newX = pUnit->x;
						break;
					}
					freePixels = 0;
				}
			}			
		}
		pUnit->y = newY;
		pUnit->x = newX;
	}
	if (pUnit->y > (levels[currentLevel].pixelSizeY << FRACTIONAL_PART) || pUnit->x > ((levels[currentLevel].pixelSizeX << FRACTIONAL_PART) + PLAYER_WIDTH / 2) || (pUnit->x >> FRACTIONAL_PART) <  (- PLAYER_WIDTH / 2)  )
	{
		if (!isSimulating )
			triggerFx(PATCH_OFFSIDE, 255,FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY | FX_FLAGS_RETRIG, 1 << FRACTIONAL_PART);
		pUnit->state = PLAYER_STATE_DEAD;
		return 0;
	}
	if (pUnit->state & PLAYER_STATE_IDLE)
		return 0;
	return 1;
}
inline int calculateDeltaAngle (int currentAngle, int lastAngle)
{
	// we need to check for wrap...
	if (lastAngle <= 90 && currentAngle >= 270)
		return - 360 + currentAngle - lastAngle;
	if (lastAngle >= 270 && currentAngle <= 90)
		return 360 + currentAngle - lastAngle;
	return currentAngle - lastAngle;
}
uint8_t putTemplate(int x, int y, int t)
{
	const mapTemplate_t *pTemplate = &mapTemplates[t];
	int xStart, xStop;
	int yStart, yStop;
	if (pTemplate->dx >= 0 )
	{
		xStart = x;
		xStop = x + pTemplate->sizeX;
	}
	else
	{
		xStart = x - pTemplate->sizeX +1;
		xStop = x + 1;
	}
	if (pTemplate->dy >= 0 && 0 == (pTemplate->direction & TEMPLATE_FLAGS_UPWARD))
	{
		yStart = y;
		yStop = y + pTemplate->sizeY;
	}
	else
	{
		yStart = y - pTemplate->sizeY + 1;
		yStop = y + 1;
	}
	int i = 0;
	// first check if we can put one template.
	for (int yy = yStart; yy < yStop; yy++)
	{
		for (int xx = xStart; xx < xStop; xx++ )
		{
			if (xx >= 0 && xx < MAX_MAP_SIZE_X && yy >= 0 && yy < MAX_MAP_SIZE_Y)
			{
				if (getRamMapTileFromTileCoordinates(xx, yy) != 0)
					return 0;
			}
		}
	}
	// now, if we are here, we can actually put it, to "reserve" the tiles for the next operation (Checking if we can put another template on next move)
	for (int yy = yStart; yy < yStop; yy++)
	{
		for (int xx = xStart; xx < xStop; xx++ )
		{
			if (xx >= 0 && xx < MAX_MAP_SIZE_X && yy >= 0 && yy < MAX_MAP_SIZE_Y)
			{
				if (pTemplate->tileIndexes[i] != 0)
				{
					setRamMapTileFromTileCoordinates(xx, yy, pTemplate->tileIndexes[i]);
				}
			}
			i++;
		}
	}
	// now test if we can put at least one template.
	uint8_t found = 0;
	for (i = 0; i < compatibleTemplates[t].size && found == 0; i++)
	{
		const mapTemplate_t *pNextTemplate = &mapTemplates[i]; 
		int xStart2, xStop2;
		if (pNextTemplate->dx >= 0 )
		{
			xStart2 = x + pTemplate->dx;
			xStop2 = x + pTemplate->dx +  pNextTemplate->sizeX;
		}
		else
		{
			xStart2 = x + pTemplate->dx - pNextTemplate->sizeX +1;
			xStop2 = x + pTemplate->dx + 1;
		}
		int yStart2, yStop2;
		if (pNextTemplate->dy >= 0 && 0 == (pNextTemplate->direction & TEMPLATE_FLAGS_UPWARD))
		{
			yStart2 = y + pTemplate->dy;
			yStop2 = y + pTemplate->dy + pNextTemplate->sizeY;
		}
		else
		{
			yStart2 = y + pTemplate->dy - pNextTemplate->sizeY + 1;
			yStop2 = y + pTemplate->dy + 1;
		}
		// first check if we can put next template.
		found = 1;
		for (int yy = yStart2; yy < yStop2 && found; yy++)
		{
			for (int xx = xStart2; xx < xStop2; xx++ )
			{
				if (xx >= 0 && xx < MAX_MAP_SIZE_X && yy >= 0 && yy < MAX_MAP_SIZE_Y)
				{
					if (getRamMapTileFromTileCoordinates(xx, yy) != 0)
					{
						found = 0;
						break;
					}
				}
			}
		}
		if (found)
			break;
	}
	// if not found, remove the template we previously put.
	if (!found)
	{
		for (int yy = yStart; yy < yStop; yy++)
		{
			for (int xx = xStart; xx < xStop; xx++ )
			{
				if (xx >= 0 && xx < MAX_MAP_SIZE_X && yy >= 0 && yy < MAX_MAP_SIZE_Y)
				{
					setRamMapTileFromTileCoordinates(xx, yy, 0);
				}
			}
		}
		return 0;
	}
	return 1;
}
uint8_t createRandomLevel(int seed)
{
	memset(&gameRamMap, 0, sizeof(gameRamMap));
	srand(seed);
    int16_t startX = 0;

    uint8_t goingRight = 1;
    int16_t startY = MAX_MAP_SIZE_Y / 2 + (rand() % (MAX_MAP_SIZE_Y / 2));
    uint8_t oldTemplateN = 0;
    int16_t maxX = 0;
    uint8_t preferredDirection = 0xF;
	int16_t lastAngle = 0;
    int16_t cumulativeAngle = 0;
    int8_t downThreshold = 10;
    int8_t upThreshold = 10;
    while (startX < MAX_MAP_SIZE_X && startX >= -100)
    {
        if (startX < 0)
            preferredDirection = DIRECTION_RIGHTUP;
        int i = rand() % 100;
        int leftTurnProb = startX > maxX ? PROBABILITY_TURN_LEFT : PROBABILITY_TURN_LEFT / 10;
        if ((goingRight && i < leftTurnProb) || (!goingRight && i < PROBABILITY_TURN_RIGHT))
        {
            goingRight = !goingRight;
            preferredDirection = goingRight ? DIRECTION_NOTLEFT : DIRECTION_RIGHT;
        }
        if (startY >= MAX_MAP_SIZE_Y + downThreshold)
        {
            preferredDirection = goingRight ? DIRECTION_RIGHTUP : DIRECTION_LEFTUP;
            downThreshold = -5;
        }
        else
        {
            preferredDirection |= DIRECTION_DOWN;
            downThreshold = 10;
        }
        if (startY <= upThreshold)
        {
            preferredDirection = goingRight ? DIRECTION_RIGHTDOWN : DIRECTION_LEFTDOWN;
            upThreshold = 15;
        }            
        else 
        {
            preferredDirection |= DIRECTION_UP;
            upThreshold = 10;
        }
        if (maxX >= startX)
            maxX = startX;
        // try to find a good tile
        uint8_t isOk = 0;
        uint8_t directionsTested = 0;
        uint8_t mapTemplateIndex;
		int16_t angle;
        do
        {			
            compatibleTemplateIndexArray_t compatibleTemplateArray;
			memcpy (&compatibleTemplateArray,  &compatibleTemplates[oldTemplateN], sizeof (compatibleTemplateIndexArray_t));            
			do
            {
                isOk = 0; 
                i = rand() % compatibleTemplateArray.size;
                mapTemplateIndex = compatibleTemplateArray.templateIndexes[i];
				// remove current template from the list. Removal is simply achieved by copying the last element in the current element index to be removed, and decreasing size.
				compatibleTemplateArray.templateIndexes[i] = compatibleTemplateArray.templateIndexes[compatibleTemplateArray.size - 1]; 
				compatibleTemplateArray.size--;
                //
				angle = templateAngle[mapTemplates[mapTemplateIndex].direction & TEMPLATE_DIRECTION_MASK];
				if  ((0 == ((mapTemplates[mapTemplateIndex].direction & TEMPLATE_DIRECTION_MASK ) & ~preferredDirection)) && ((cumulativeAngle + calculateDeltaAngle (angle , lastAngle)) > -270 && (cumulativeAngle + calculateDeltaAngle(angle ,lastAngle)) < 270) )
                    isOk = putTemplate(startX, startY, mapTemplateIndex);
            } while (compatibleTemplateArray.size > 0 && isOk == 0);

            if (!isOk)
            {
                goingRight = !goingRight;
                preferredDirection = goingRight ? DIRECTION_NOTLEFT : DIRECTION_NOTRIGHT;
                if (startY >= MAX_MAP_SIZE_Y + downThreshold)
                {
                    preferredDirection = goingRight ? DIRECTION_RIGHTUP : DIRECTION_LEFTUP;
                    downThreshold = -5;
                }
                else
                {
                    preferredDirection |= DIRECTION_DOWN;
                    downThreshold = 10;
                }
                if (startY <= upThreshold)
                {
                    preferredDirection = goingRight ? DIRECTION_RIGHTDOWN : DIRECTION_LEFTDOWN;
                    upThreshold = 15;
                }            
                else 
                {
                    preferredDirection |= DIRECTION_UP;
                    upThreshold = 10;
                }
            }
            directionsTested++;
        } while (!isOk && directionsTested < 2);
        if (!isOk)
        {
           return 0;
        }
        cumulativeAngle += calculateDeltaAngle(angle, lastAngle);
        lastAngle = angle;
        oldTemplateN = mapTemplateIndex;
        startX += mapTemplates[mapTemplateIndex].dx;
        startY += mapTemplates[mapTemplateIndex].dy;
    }
    if (startX < MAX_MAP_SIZE_X)
    {
		return 0;
    }
	// now fill the map!
	uint8_t valid = 1;
	for (uint8_t x = 0; x < MAX_MAP_SIZE_X && valid; x++)
	{
		uint8_t fill = 0;
		for (uint8_t y = 0; y < MAX_MAP_SIZE_Y && valid; y++)
		{
			uint8_t tile = (x & 3) + (y & 3) * 4 + 1;
			uint8_t storedTile = getRamMapTileFromTileCoordinates(x, y);
			uint8_t prop = tileProps[storedTile];
			switch (prop)
			{
				case TILE_EMPTY:
					if (fill)
					{
						setRamMapTileFromTileCoordinates(x, y, tile);
					}
					break;
				case TILE_NEUTRAL:
					// do nothing
					break;
				case TILE_STARTFILL:
					if (fill)
					{
						valid = 0;
					}
					else
					{
						fill = 1;
					}
					break;
				case TILE_STOPFILL:
					if (!fill)
					{
						valid = 0;
					}
					else
					{
						fill = 0;
					}
					break;
			}
		}
	}
	return valid;
}

void drawScene(unit_t * pFocusedUnit, uint8_t forceRedraw)
{
	int spriteNumber = 0;
	int frameNumber = 0;
	if (pFocusedUnit == NULL)
	{
		pFocusedUnit = game.focusedUnit;		
	}
	else
	{
		game.focusedUnit = pFocusedUnit;
	}
	cameraUnit.x = pFocusedUnit->x;
	cameraUnit.y = pFocusedUnit->y;	
	int x = (pFocusedUnit->x >> FRACTIONAL_PART) - SCREEN_SIZE_X / 2;
	int y = (pFocusedUnit->y >> FRACTIONAL_PART) - SCREEN_SIZE_Y / 2;
	setViewPosition(&x, &y, forceRedraw);
	for (int t = 0; t < game.numberOfTeams; t++)
	{
		for (int u = 0 ; u < game.numberOfUnitsPerTeam; u++)
		{
			team_t * pTeam = &teams[t];
			unit_t * pUnit = &playerUnits[pTeam->firstUnitIndex + u];
			int xSp = (pUnit->x >> FRACTIONAL_PART)  - x;
			int ySp =   (pUnit->y >> FRACTIONAL_PART)  - 4  - y;
			uint8_t state = pUnit->state;
			if (state & PLAYER_STATE_DEAD)
				continue;			
			//  get frame number offset by animation state.
			if ((state & PLAYER_STATE_WALKING) && !(state & PLAYER_STATE_FIRING))
			{
				frameNumber = weaponsData[game.currentWeapon].walkFrameIndex[ (3 * (game.currentAimAngle + 128)) >> 8];
				frameNumber += pUnit->frameNumber;
			}
			else if (state & PLAYER_STATE_IDLE)
			{
				int currentWeapon = game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING || game.gameSubState == GAME_SUBSTATE_FIRING ?  game.currentWeapon : WEAPON_NONE;
				frameNumber = weaponsData[currentWeapon].aimFrameIndex[ (3 * (game.currentAimAngle + 128)) >> 8];
			}
			else if (state & PLAYER_STATE_JUMPING)
			{
				frameNumber = PLAYER_WALK_FRAMEINDEX + 3;
			}
			else if (state & PLAYER_STATE_FALLING)
			{
				frameNumber = pUnit->frameNumber + PLAYER_FLY_AWAY_FRAMEINDEX;
			}
			else if (state & PLAYER_STATE_FIRING)
			{
				frameNumber = weaponsData[game.currentWeapon].shootFrameIndex[ (3 * (game.currentAimAngle + 128)) >> 8];
				frameNumber += pUnit->frameNumber;
			}
			//
			if ((game.currentTeam == t && u == pTeam->selectedUnit) || game.gameSubState == GAME_SUBSTATE_POST_FIRING_ANIMATION || game.gameSubState == GAME_SUBSTATE_FOLLOW_PHYSICS)
			{
				spriteNumber += putSprite(spriteNumber, xSp, ySp, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_CENTER | SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION | (pUnit->state & PLAYER_FACING_RIGHT ?  0 : SPRITE_FLAGS_FLIP_HORIZONTAL), frameNumber);
				// let's draw the aim pointer
				if (!(weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_NO_AIMPOINTER) && !((pTeam->type & TEAM_CPU) && game.gameSubState != GAME_SUBSTATE_FIRING))
				{
					int currentAngle;
					if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_HAS_DISCRETE_DIRECTIONS)
					{
						// NUMBER_OF_SINTABLE_ENTRIES = 2 * PI.   
						uint16_t dir = ((3 * (game.currentAimAngle + 128)) >> 8) * NUMBER_OF_SINTABLE_ENTRIES / 12  + NUMBER_OF_SINTABLE_ENTRIES / 6;
						currentAngle = (pUnit->state & PLAYER_FACING_RIGHT) ? dir  :  NUMBER_OF_SINTABLE_ENTRIES - dir;					
					}
					else
					{
						currentAngle = (pUnit->state & PLAYER_FACING_RIGHT ? game.currentAimAngle + 128 :  128 + 255 - game.currentAimAngle ) << 1;
					}
					int deltax, deltay;
					deltax = (FIXED_MULFRACT(AIM_POINTER_DISTANCE << FRACTIONAL_PART, sinTable[currentAngle]) + ((pUnit->state & PLAYER_FACING_RIGHT ) ? PROJECTILE_FIRING_DISTANCE : - PROJECTILE_FIRING_DISTANCE)) >> FRACTIONAL_PART;
					deltay = - FIXED_MULFRACT(AIM_POINTER_DISTANCE << FRACTIONAL_PART, sinTable[(currentAngle + 256) & 1023]) >> FRACTIONAL_PART;
					if ((pUnit->state & (PLAYER_STATE_IDLE | PLAYER_STATE_FIRING)) && (game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING || game.gameSubState == GAME_SUBSTATE_FIRING))
						spriteNumber += putSprite(spriteNumber, xSp + deltax, ySp + deltay, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_CENTER, WEAPON_POINTER_AIM_FRAMEINDEX);					
				}
			}
			else
				spriteNumber += putSprite(spriteNumber, xSp, ySp, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_CENTER | SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION | (pUnit->state & PLAYER_FACING_RIGHT ?  0 : SPRITE_FLAGS_FLIP_HORIZONTAL), PLAYER_WALK_FRAMEINDEX);
		}
	}
	// We plot the team indicator when: a) player is not moving (or is not moving but it is the current player and we asked to not do so) & b) the current state is not post_firing_animation.
	//if (! ((teams[game.currentTeam].type & TEAM_CPU) && game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING))
	{		
		for (int t = 0; t < game.numberOfTeams; t++)
		{
			for (int u = 0 ; u < game.numberOfUnitsPerTeam; u++)
			{
				team_t * pTeam = &teams[t];
				unit_t * pUnit = &playerUnits[pTeam->firstUnitIndex + u];
				int xSp = (pUnit->x >> FRACTIONAL_PART)  - x;
				int ySp =   (pUnit->y >> FRACTIONAL_PART)  - 4  - y;
				uint8_t state = pUnit->state;
				//  get frame number offset by animation state.
				if (!(state & PLAYER_STATE_DEAD))
				{
					if ((state & PLAYER_STATE_IDLE) /*&& (game.gameSubState != GAME_SUBSTATE_POST_FIRING_ANIMATION  && game.gameSubState != GAME_SUBSTATE_PRE_FIRING_ANIMATION && game.gameSubState != GAME_SUBSTATE_FIRING && game.gameSubState != GAME_SUBSTATE_REPEAT_FIRE)*/ || game.gameSubState == GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_SHOW_DAMAGES)
					{
						if (! (t == game.currentTeam && pTeam->selectedUnit == u && !game.showCurrentUnitIndicator))
						{
							spriteNumber += putSprite(spriteNumber, xSp, ySp - DELTA_Y_TEAM_INDICATOR, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_CENTER, teamsIndicator[t] + u);
							if (! ((teams[game.currentTeam].type & TEAM_CPU) && game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING)) 
								spriteNumber += putSprite(spriteNumber, xSp, ySp - DELTA_Y_HEALTH_INDICATOR, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_CENTER, HEALTH_VALUE_FRAMEINDEX + pTeam->health[u]);					
						}
					}
					if (game.gameSubState == GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_SHOW_DAMAGES && pTeam->damage[u] != 0)
					{
						spriteNumber += putSprite(spriteNumber, xSp, ySp - DELTA_Y_TEAM_INDICATOR - (game.stateTime >> 1), SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_CENTER, HEALTH_VALUE_FRAMEINDEX + pTeam->damage[u]);
					}				
				}
			}
		}		
	}
	//
	for (int p = 0 ; p < MAX_INGAME_PROJECTILES; p++)
	{
		unit_t *pUnit = &projectileUnits[p];
		if (pUnit->state != PROJECTILE_STATE_NOT_CREATED)
		{
			int xSp = (pUnit->x >> FRACTIONAL_PART)  - x;
			const projectile_t *pProjectile = &projectiles[pUnit->type & PROJECTILE_TYPE_MASK];
			if (pProjectile->flags & PROJECTILE_FLAGS_INVISIBLE)
				continue;
			int ySp =   (pUnit->y >> FRACTIONAL_PART) - y;
						
			if (pProjectile->flags & PROJECTILE_FLAGS_ANIM_HAS_SEQUENCES)
			{
				uint8_t frameNumber = (pUnit->frameNumber / pProjectile->projectileSequenceDuration) * pProjectile->projectileSequenceNumFrames + (pUnit->frameNumber % pProjectile->projectileSequenceNumFrames);
				spriteNumber += putSprite(spriteNumber, xSp, ySp, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION | SPRITE_FLAGS_HANDLE_CENTER | pUnit->frameFlags, pProjectile->projectileFrameIndex + frameNumber);			
			}
			else
			{
				spriteNumber += putSprite(spriteNumber, xSp, ySp, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION | SPRITE_FLAGS_HANDLE_CENTER | pUnit->frameFlags, pProjectile->projectileFrameIndex + pUnit->frameNumber);
			}
		}
	}	
	// static objects
	for (int i = 0; i < MAX_STATIC_UNITS; i++)
	{
		if (staticUnits[i].type != STATIC_UNIT_TYPE_NONE)
		{
			int xSp = staticUnits[i].x  - x;
			int ySp =   staticUnits[i].y - y;			
			spriteNumber += putSprite(spriteNumber, xSp, ySp, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION | SPRITE_FLAGS_HANDLE_CENTER, staticUnitFrames[staticUnits[i].type - 1]);			
		}
	}
	// at last we plot the water / clouds.
	// water
	static uint8_t waterAnim = 0;
	// x is where is positioned the camera. Since the water is on top of all,  it will move faster. We will assume 2 times faster.
	int xW = - (x & (WATER_SPRITE_WIDTH - 1));
	while (xW < VRAMX * TILE_SIZE_X)
	{
		spriteNumber += putSprite(spriteNumber,xW, WATER_LEVEL - y - 1, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_BOTTOMLEFT, WATER_WAVES_FRAMEINDEX + (waterAnim >> 2));
		xW += WATER_SPRITE_WIDTH;
	}
	waterAnim++;
	if (waterAnim >= (WATER_WAVES_NUMFRAMES << 2) )
		waterAnim = 0;		

	// add now: clouds!
	// Clouds have a) Their natural movement due to wind speed b) parallax scrolling.
	// let's make a smooth transition of the speed.	The maximum delta is 255. To avoid taking ages, we have a maximum step of 4 (about 64 frames, i.e. about 1 second) 
	static int16_t analogWindSpeed;
	uint8_t n =  ( (int16_t) game.windSpeed + 128) * 10 / 256;
	int16_t delta = (windSpeeds[n] * 256 - analogWindSpeed);
	analogWindSpeed += delta >> 4;
	int32_t error;
	if (n < NUMBER_OF_WIND_SPEEDS)
	{
		error = 256 * delta >> (4 - n);
	}
	else
	{
		error = 256 * delta >> (n - NUMBER_OF_WIND_SPEEDS);		
	}
	if (error > - 1024 && error < 1024)
	{
		// digital
		cloudPatternX += windSpeeds[n] << 13;
	}
	else
	{
		cloudPatternX += (analogWindSpeed << 5);
	}	
	// We have to draw clouds. The pattern is repeated in a cyclic way. The pattern is 320 pixel long
	int xPatternPos =  (((cloudPatternX >> FRACTIONAL_PART)  + (game.lastViewX >> 2)) % CLOUD_PATTERN_WIDTH );
	if (xPatternPos > 0)
		xPatternPos -= CLOUD_PATTERN_WIDTH;
	int yPatternPos = (y >> 1);		
	while (xPatternPos - x < VRAMX * TILE_SIZE_X)
	{
		for (int i = 0; i < NUMBER_OF_CLOUDS; i++)
		{
			spriteNumber += putSprite(spriteNumber, xPatternPos + cloudX[i] - x , cloudY[i] + yPatternPos - y, SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM | SPRITE_FLAGS_HANDLE_BOTTOMLEFT, cloudFrames[i]);			
		}
		xPatternPos += CLOUD_PATTERN_WIDTH;
	}
	// finally animate water  deformation
	const uint8_t deformationOFfsets[] = {0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 5, 5, 4, 3, 2, 2, 1, 1 };
	static uint16_t waterDeformationFrame = 0;
	uint8_t off = deformationOFfsets[waterDeformationFrame >> 2];
	// detect which are the tiles of the water. 
	int startY = (WATER_LEVEL  -  y + TILE_SIZE_Y - 1) / TILE_SIZE_Y; 
	for (int y = 0; y < startY && y < VRAMY; y++)
		videoData.ptrTileXScroll[y] = 0;
	for (int y = startY; y < VRAMY;  y++)
	{
		videoData.ptrTileXScroll[y] = off;
	}
	
	waterDeformationFrame++;
	if (waterDeformationFrame >= (sizeof (deformationOFfsets) << 2))
	{
		waterDeformationFrame = 0;
	}
	// last but not least: in GAME_SUBSTATE_POSITION_SELECTION, draw the pointer.
	if (game.gameSubState == GAME_SUBSTATE_POSITION_SELECTION || game.gameSubState == GAME_SUBSTATE_PLACE_GIRDERS)
		spriteNumber += putSprite(spriteNumber, (cameraUnit.x >> FRACTIONAL_PART) - x , (cameraUnit.y >> FRACTIONAL_PART) - y, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_HANDLE_BOTTOMRIGHT, AIRSTRIKE_ARROW_FRAMEINDEX);	
}
uint8_t playerHandler(unit_t* pUnit, uint8_t command, uint8_t isSimulating)
{
	uint8_t state = pUnit->state;
	uint8_t result = 0;
	if (state & PLAYER_STATE_DEAD)
		return 0;
	if ((state & (PLAYER_STATE_WALKING | PLAYER_STATE_IDLE)) && command == COMMAND_NONE)
	{
		// stop moving
		pUnit->state = state & PLAYER_FACING_RIGHT ?   PLAYER_STATE_IDLE_RIGHT : PLAYER_STATE_IDLE_LEFT;
		pUnit->frameNumber = 0;
		pUnit->vx = 0;
	}
	else if ((state == PLAYER_STATE_IDLE_LEFT || state == PLAYER_STATE_WALKING_LEFT) && command == COMMAND_RIGHT)
	{
		// turn right
		pUnit->state = PLAYER_STATE_IDLE_RIGHT;
		pUnit->frameNumber = 0;
		pUnit->vx = 0;
	}
	else if ((state == PLAYER_STATE_IDLE_RIGHT|| state == PLAYER_STATE_WALKING_RIGHT) && command == COMMAND_LEFT)
	{
		// turn left
		pUnit->state = PLAYER_STATE_IDLE_LEFT;
		pUnit->frameNumber = 0;
		pUnit->vx = 0;
	}
	else if (state == PLAYER_STATE_IDLE_LEFT && command == COMMAND_LEFT)
	{
		// turn right
		pUnit->state = PLAYER_STATE_WALKING_LEFT;
		pUnit->frameNumber = 0;
		pUnit->lastFrameTick = 0; // ticks;
		pUnit->vx = - PLAYER_WALK_SPEED;
		result = movePlayer(pUnit, isSimulating);
	}
	else if (state == PLAYER_STATE_IDLE_RIGHT && command == COMMAND_RIGHT)
	{
		// turn left
		pUnit->state = PLAYER_STATE_WALKING_RIGHT;
		pUnit->frameNumber = 0;
		pUnit->lastFrameTick = 0; //ticks;
		pUnit->vx = PLAYER_WALK_SPEED;
		result = movePlayer(pUnit, isSimulating);
	}
	else if (state == PLAYER_STATE_WALKING_RIGHT && command == COMMAND_RIGHT)
	{
		// try continue moving right
		result = movePlayer(pUnit, isSimulating);
	}
	else if (state == PLAYER_STATE_WALKING_LEFT && command == COMMAND_LEFT)
	{
		// try continue moving left
		result = movePlayer(pUnit, isSimulating);
	}
	else if ((state & (PLAYER_STATE_IDLE | PLAYER_STATE_WALKING)) && command == COMMAND_JUMP)
	{
		pUnit->state = PLAYER_STATE_JUMPING | (state & PLAYER_FACING_RIGHT);
		pUnit->frameNumber = 0;
		pUnit->lastFrameTick = 0; //ticks;
		pUnit->vy = JUMPING_INITIAL_SPEED;
		pUnit->vx = (state & PLAYER_FACING_RIGHT) ? JUMPING_HORIZONTAL_SPEED : -JUMPING_HORIZONTAL_SPEED;
	}
	else if (state & PLAYER_STATE_JUMPING)
	{
		if (!getMapPixel(pUnit->x >> FRACTIONAL_PART, (pUnit->y >> FRACTIONAL_PART) + 1) || pUnit->vy < 0)
			pUnit->vy += ((customLevelData_t *) levels[currentLevel].customData)->gravity;
		result = movePlayer(pUnit, isSimulating);
		// continue animation and based on current vertical speed, if it is too high, go to "PLAYER_STATE_FALLING"
	}
	else if (state & PLAYER_STATE_FALLING)
	{
		if (!getMapPixel(pUnit->x >> FRACTIONAL_PART, (pUnit->y >> FRACTIONAL_PART) + 1)  || pUnit->vy < 0)
			pUnit->vy += ((customLevelData_t *) levels[currentLevel].customData)->gravity;
		result = movePlayer(pUnit, isSimulating);		
	}	
	// now checck again the state, based on what happened
	state = pUnit->state;
	if (state & PLAYER_STATE_WALKING)
	{
		pUnit->lastFrameTick++;
//		if ( ((uint8_t) (ticks - pUnit->lastFrameTick)) > ANIMATION_WALKING_FRAME_DELAY)
		if (pUnit->lastFrameTick > ANIMATION_WALKING_FRAME_DELAY)	
		{
			pUnit->lastFrameTick = 0; //ticks;
			pUnit->frameNumber++;
			if (pUnit->frameNumber >= PLAYER_WALK_NUMFRAMES)
			{
				pUnit->frameNumber = 0;
			}
		}
	}
	else if (state & PLAYER_STATE_FALLING)
	{
		pUnit->lastFrameTick++;
//		if ( ((uint8_t) (ticks - pUnit->lastFrameTick)) > ANIMATION_WALKING_FRAME_DELAY)
		if (pUnit->lastFrameTick > ANIMATION_WALKING_FRAME_DELAY)
		{
			pUnit->lastFrameTick = 0; // ticks;
			pUnit->frameNumber++;
			if (pUnit->frameNumber >= PLAYER_FLY_AWAY_NUMFRAMES)
			{
				pUnit->frameNumber = 0;
			}
		}
	}
	return result;
}
void randomlyPlaceUnits()
{
	srand(getFrameTicks());
	for (int t = 0; t < game.numberOfTeams; t++)
	{
		for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
		{
			uint8_t placed = 0;
			while (!placed)
			{
				int64_t x, y;
				int freePixels = 0;
				x = X_NOSPAWN_BORDER + ((int64_t) rand() * (levels[currentLevel].pixelSizeX - 2 * X_NOSPAWN_BORDER)) / RAND_MAX;
				y = Y_NOSPAWN_BORDER + ((int64_t) rand() * (levels[currentLevel].pixelSizeY - Y_NOSPAWN_BORDER - WATER_HEIGHT) ) / RAND_MAX;
				int yTest;
				for (yTest = y - PLAYER_HEIGHT; yTest <= WATER_LEVEL; yTest++)
				{
					if (!getMapPixel(x, yTest))
						freePixels ++;
					else
						break;
				}
				if (freePixels >= PLAYER_HEIGHT && yTest < WATER_LEVEL)
				{
					y = yTest;
					playerUnits[teams[t].firstUnitIndex + u].x =  x << FRACTIONAL_PART;
					playerUnits[teams[t].firstUnitIndex + u].y = y << FRACTIONAL_PART;
					playerUnits[teams[t].firstUnitIndex + u].state = x > levels[currentLevel].pixelSizeX / 2 ?  PLAYER_STATE_IDLE_LEFT : PLAYER_STATE_IDLE_RIGHT;
					placed = 1;
				}
			}
			
		}
	}
}
void randomlyPlaceMines(int numMines)
{
	int attempts = 1000;
	for (int i = 0; i < numMines && attempts > 0; i++)
	{
		staticUnit_t *s = &staticUnits[i];
		while (attempts > 0)
		{
			int32_t x, y;
			int freePixels = 0;
			// to simplify things, we just check that the mine is horizontally far from any unit
			int good;
			do 
			{
				good = 1;
				x = X_NOSPAWN_BORDER + ((int64_t) rand() * (levels[currentLevel].pixelSizeX - 2 * X_NOSPAWN_BORDER)) / RAND_MAX;
				for (int t = 0; t < game.numberOfTeams && good == 1; t++)
				{
					for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
					{
						int xU = playerUnits[teams[t].firstUnitIndex + u].x >> FRACTIONAL_PART;
						if ((xU - x) * (xU - x) < projectiles[PROJECTILE_ACTIVE_MINE].proximitySetOffSquareRadius)
						{
							good = 0; // too close to an unit...
							break;
						}						
					}
				}
				if (good == 0)
					attempts--;				
			} while (attempts > 0 && good == 0);						
			y = Y_NOSPAWN_BORDER + ((int64_t) rand() * (levels[currentLevel].pixelSizeY - Y_NOSPAWN_BORDER - WATER_HEIGHT) ) / RAND_MAX;
			int yTest;
			for (yTest = y; yTest <= WATER_LEVEL; yTest++)
			{
				if (!getMapPixel(x, yTest))
					freePixels ++;
				else
					break;
			}
			if (freePixels >= 1 && yTest < WATER_LEVEL && attempts > 0)
			{
				y = yTest - 1;
				s->x =  x;
				s->y = y ;
				s->type = STATIC_UNIT_TYPE_MINE;
				break;
			}
		}
	}
}

uint8_t bresenhamCollisionCheckAgainstUnits(int x0, int y0, int x1, int y1, int *xColl, int *yColl, const projectile_t * pProjectile)
{
	int dx =  x1 > x0 ? (x1 - x0) : x0 - x1;
	int sx = x0 < x1 ? 1 : -1;
	int dy = y1 > y0 ? y0 -  y1 : y1 - y0;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx+dy;  /* error value e_xy */
	while (1)   /* loop */
	{

		for (uint t = 0; t < game.numberOfTeams; t++)
		{
			for (uint u = 0; u < game.numberOfUnitsPerTeam; u++)
			{
				unit_t *pCheckUnit = &playerUnits[teams[t].firstUnitIndex + u];
				if (!(pCheckUnit->state & PLAYER_STATE_DEAD)) // do not consider dead players
				{
					int xU = pCheckUnit->x >> FRACTIONAL_PART;
					//int yU = pCheckUnit->y >> FRACTIONAL_PART;
					int yU = (pCheckUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;
					int squareDistance = (x0 - xU) * (x0 - xU) + (y0 - yU) * (y0 - yU);
					if ( squareDistance <= pProjectile->proximitySetOffSquareRadius && !( t == game.currentTeam && u == teams[t].selectedUnit && (pProjectile->flags & PROJECTILE_FLAGS_NO_SELF_PROXIMITY_SET_OFF)))
					{
						*xColl = x0;
						*yColl = y0;
						return 1;
					}
				}
			}
		}
		if (x0 == x1 && y0 == y1)
			return 0;
		int e2 = 2*err;
		if (e2 >= dy)
		{
			err += dy;  /* e_xy+e_x > 0 */
			x0 += sx;
		}
		if (e2 <= dx) /* e_xy+e_y < 0 */
		{
			err += dx;
			y0 += sy;
		}
	}
}
uint8_t bresenhamCollisionCheckTimed(int x0, int y0, int x1, int y1, int *xColl, int *yColl, uint16_t maxFrameLine)
{
	int oldX = x0, oldY = y0;
	int dx =  x1 > x0 ? (x1 - x0) : x0 - x1;
	int sx = x0 < x1 ? 1 : -1;
	int dy = y1 > y0 ? y0 -  y1 : y1 - y0;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx+dy;  /* error value e_xy */
	while (1)   /* loop */
	{
		if (getMapPixel(x0, y0))
		{
			// determine in the old position we checked, where there was the obstacle. We actually need to check only 3 pixels.
			
			 *xColl = oldX;
			 *yColl = oldY;
			 uint8_t bounce = 0; 
			 if (getMapPixel(oldX + sx, oldY))
				bounce |= BOUNCE_HORIZONTAL;
			 if (getMapPixel(oldX, oldY + sy))
				bounce |= BOUNCE_VERTICAL;
			if (bounce == 0)
				bounce = BOUNCE_HORIZONTAL | BOUNCE_VERTICAL;	
			return bounce;
		}
		if (videoData.currentLineNumber >= MAX_FRAME_LINE)
		{
			return BOUNCED_TIMEOUT;
		}
		if (x0 == x1 && y0 == y1) 
			return 0;
		int e2 = 2*err;
		if (e2 >= dy)
		{
			err += dy;  /* e_xy+e_x > 0 */
			oldX = x0;
			x0 += sx;
		}
		if (e2 <= dx) /* e_xy+e_y < 0 */
		{    
			err += dx;
			oldY=y0;
			y0 += sy;
		}
	}
}
uint8_t bresenhamCollisionCheck(int x0, int y0, int x1, int y1, int *xColl, int *yColl)
{
	int oldX = x0, oldY = y0;
	int dx =  x1 > x0 ? (x1 - x0) : x0 - x1;
	int sx = x0 < x1 ? 1 : -1;
	int dy = y1 > y0 ? y0 -  y1 : y1 - y0;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx+dy;  /* error value e_xy */
	while (1)   /* loop */
	{
		if (getMapPixel(x0, y0))
		{
			// determine in the old position we checked, where there was the obstacle. We actually need to check only 3 pixels.
			
			 *xColl = oldX;
			 *yColl = oldY;
			 uint8_t bounce = 0; 
			 if (getMapPixel(oldX + sx, oldY))
				bounce |= BOUNCE_HORIZONTAL;
			 if (getMapPixel(oldX, oldY + sy))
				bounce |= BOUNCE_VERTICAL;
			if (bounce == 0)
				bounce = BOUNCE_HORIZONTAL | BOUNCE_VERTICAL;	
			return bounce;
		}
		if (x0 == x1 && y0 == y1) 
			return 0;
		int e2 = 2*err;
		if (e2 >= dy)
		{
			err += dy;  /* e_xy+e_x > 0 */
			oldX = x0;
			x0 += sx;
		}
		if (e2 <= dx) /* e_xy+e_y < 0 */
		{    
			err += dx;
			oldY=y0;
			y0 += sy;
		}
	}
}
uint8_t bresenhamCollisionCheckTiles(int x0, int y0, int x1, int y1)
{
	int dx =  x1 > x0 ? (x1 - x0) : x0 - x1;
	int sx = x0 < x1 ? 1 : -1;
	int dy = y1 > y0 ? y0 -  y1 : y1 - y0;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx+dy;  /* error value e_xy */
	while (1)   /* loop */
	{
		if (!(x0 < 0 || y0 < 0 || y0 >= MAX_MAP_SIZE_Y || x0 >= MAX_MAP_SIZE_X))
		{
			if (getRamMapTileFromTileCoordinates(x0, y0))
			{
				// determine in the old position we checked, where there was the obstacle. We actually need to check only 3 pixels.
				return 1;
			}			
		}
		if (x0 == x1 && y0 == y1)
			return 0;
		int e2 = 2*err;
		if (e2 >= dy)
		{
			err += dy;  /* e_xy+e_x > 0 */
			x0 += sx;
		}
		if (e2 <= dx) /* e_xy+e_y < 0 */
		{
			err += dx;
			y0 += sy;
		}
	}
}

void clearDamages()
{
	for (int t = 0; t < game.numberOfTeams; t++)
	{
		for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
		{
			teams[t].damage[u] = 0;
		}
	}
}
void roundInit()
{
	changeLevelEx(1 + (rand() %  (getNumberOfLevels() - 1) ), 256);
	for (int t = 0; t < MAX_TEAMS; t++)
		memset(teams[t].health,  0, sizeof (teams[t].health));
	// force update fixed section to health 0, so that it will slowly increase
	updateFixedSection(0, NULL, 0, 1);	
	uint8_t unitIndex = 0;	
//	memset(&teams, 0, sizeof(teams));
	memset(&projectileUnits, 0, sizeof(projectileUnits));
	memset(&staticUnits, 0, sizeof(staticUnits));
	while (!createRandomLevel(rand()));
	// allocate units based on number of teams.
	for (int t = 0; t < game.numberOfTeams; t++)
	{
		teams[t].firstUnitIndex = unitIndex;
		for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
		{
			teams[t].health[u] = 100;
			teams[t].damage[u] = 0;
			teams[t].selectedUnit = game.numberOfUnitsPerTeam - 1;  // in this way, the next unit will be always unit 0 (i.e. 1).
			unitIndex++;
		}
		for (int w = 0; w < NUMBER_OF_WEAPONS; w++)
		{
			int8_t n;
			switch (w)
			{
				default:
					 n = - 1;		// infinite ammo in genera
					break;
				case WEAPON_DYNAMITE:
					n = 1;
					break;
				case WEAPON_CLUSTER_BOMB:
					n = 5;
					break;
				case WEAPON_MINE:
					n = 2;
					break;
				case WEAPON_AIRSTRIKE:
					n = 1;
					break;
				case WEAPON_TELEPORT:
					n = 2;
					break;
				case WEAPON_DEVASTATOR:
					n = 1;
					break;

				case WEAPON_FLAGS_BLOWTORCH:	
					n = 5;
					break;
				case WEAPON_MACHINEGUN:
					n = 2;
					break;
			}
			teams[t].inventory[w] = n;
		}
	}
	randomlyPlaceUnits();
	randomlyPlaceMines(MAX_INITIAL_MINES);		// 
	usedDestroyedTileNumber = 0;
	memset(usedDestroyedTiles, 0, sizeof(usedDestroyedTiles));			
	game.fireRound = 0;
	game.windSpeed = rand();
	game.deadMask = 0;			// mark all players as alive (not dead)
}
int getNextAvailableTeamUnit(int teamNumber)
{
	// reset turn time
	game.turnTime = MATCH_TURN_TIME;
	game.tickTurnTime = 0;
	game.aiData.aiState = AI_STATE_CHOOSE_ACTION;
	game.aiData.aiStateParam = 0;
	// cycle through the team's unit
	for (int i = 0; i < game.numberOfUnitsPerTeam; i++)
	{
		int u = (teams[teamNumber].selectedUnit + i + 1) % game.numberOfUnitsPerTeam; // hopefully the compiler, for 2's power, will recognize that an "AND" operation is enoguh...
		if (!(playerUnits[teams[teamNumber].firstUnitIndex + u].state & PLAYER_STATE_DEAD))
		{
			teams[teamNumber].selectedUnit = u;
			return u;
		}		
	}
	// team is dead.
	return -1;
}
int16_t evaluateAIProjectileDamage(uint16_t  bonusEnemyDeath, uint16_t bonusEnemyDamage, uint16_t malusTeamDeath, uint16_t malusTeamDamage, uint8_t *finished, uint16_t maxFrameLine)
{
	uint16_t oldFrameLine = videoData.currentLineNumber;
	uint8_t weaponType = game.aiData.simulationWeapon;
	const projectile_t *pProjectile = &projectiles[weaponsData[weaponType].projectileType];
	uint8_t remainingCurrentTeamMembers = 0;
	uint16_t enemyDamage = 0;
	uint16_t teamDamage = 0;
	uint8_t teamDeaths = 0;
	uint8_t enemyDeaths= 0;
	uint16_t iteration = game.aiData.simulationIterationNumber;
	int vx = game.aiData.projectile.vx;
	int vy = game.aiData.projectile.vy; 
	int ux = game.aiData.projectile.x;
	int uy = game.aiData.projectile.y;
	int16_t x = ux >> FRACTIONAL_PART;
	int16_t y = uy >> FRACTIONAL_PART;
	uint8_t setOffRequest = 0;
	for (int i = 0; i < game.numberOfUnitsPerTeam; i++)
	{
		if (teams[game.currentTeam].health[i])
			remainingCurrentTeamMembers++;
	}
	while (!setOffRequest &&  videoData.currentLineNumber < maxFrameLine && iteration < MAX_AI_PROJECTION_ITERATIONS && oldFrameLine <= videoData.currentLineNumber)
	{
		oldFrameLine = videoData.currentLineNumber;
		int32_t newX = ux + vx;
		int32_t newY = uy + vy;
		int32_t nx = newX >> FRACTIONAL_PART;
		int32_t ny = newY >> FRACTIONAL_PART;
		uint32_t flags = pProjectile->flags;
		if (newY > (levels[currentLevel].pixelSizeY << FRACTIONAL_PART) || newX < - (pProjectile->halfWidth << FRACTIONAL_PART) || newX > ((pProjectile->halfWidth  + levels[currentLevel].pixelSizeX) << FRACTIONAL_PART) || (newY < 0 && vy < 0 && !(flags & PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY)))
		{	// projectile went outside the play area. No points.
			*finished = 1;
			return 0;
		} 
		if (flags & PROJECTILE_FLAGS_SUBJECTED_TO_WIND)
		{
			vx += game.windSpeed * WIND_SPEED_FACTOR;
		}		
		uint8_t bounced = 0;
		if (flags & (PROJECTILE_FLAGS_BOUNCES | PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN))
		{
			int bx, by;
			if (bresenhamCollisionCheckTiles(x / TILESIZEX, y / TILE_SIZE_Y, nx, ny)) // for fast projectiles, it's better this check too, to avoid checking a lot of cells
				bounced = bresenhamCollisionCheckTimed(x , y, nx, ny,&bx, &by, maxFrameLine);
			if (bounced == BOUNCED_TIMEOUT)
				break;		//we had no time to compute collisions. We will try next time-slot					
			if (bounced)
			{
				// has collided. We need to check which direction should bounce.
				if (flags & PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN)
				{
					setOffRequest = 1;
				}
				if (bounced & BOUNCE_VERTICAL)  // we must bounce back in y;
				{
					vy =  - FIXED_MULFRACT(vy, pProjectile->yBounceFract);	
					if (vy < UNIT_MIN_SPEED && vy > - UNIT_MIN_SPEED)
					{
						vy = 0;							
					} 
					if (!(bounced & BOUNCE_HORIZONTAL))  // avoid counting two times...
					{
						vx =  FIXED_MULFRACT(vx,  pProjectile->xBounceFract);
						if (vx < UNIT_MIN_SPEED && vx > - UNIT_MIN_SPEED)
						{
							vx = 0;
						}							
					}
				}
				if (bounced & BOUNCE_HORIZONTAL)  // we must bounce back in x
				{
					vx =  - FIXED_MULFRACT(vx, pProjectile->xBounceFract);
					if (vx < UNIT_MIN_SPEED && vx > - UNIT_MIN_SPEED)
					{
						vx = 0;
					}
					if (!(bounced & BOUNCE_VERTICAL))  // avoid counting two times...
					{
						vy =  FIXED_MULFRACT(vy,  pProjectile->yBounceFract);
						if (vy < UNIT_MIN_SPEED && vy > - UNIT_MIN_SPEED)
						{
							vy = 0;
						}	
					}
				}
				newX = bx << FRACTIONAL_PART;
				newY = by << FRACTIONAL_PART;					
			}
		}
		if (flags & PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY)
		{
			if (!((bounced & BOUNCE_VERTICAL) && vy >= 0) && !((vy >= 0) && getMapPixel(nx, ny + 1)))
			{
				vy += ((customLevelData_t *) levels[currentLevel].customData)->gravity;
			}
		}			
		if (flags & PROJECTILE_FLAGS_PROXIMITY_SETOFF)
		{
			// check for proximity with players. 
			for (int t = 0; t < game.numberOfTeams && !setOffRequest; t++)
			{
				for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
				{
					unit_t *pCheckUnit = &playerUnits[teams[t].firstUnitIndex + u];
					if (!(pCheckUnit->state & PLAYER_STATE_DEAD)) // do not consider dead players
					{
						int xU = pCheckUnit->x >> FRACTIONAL_PART;
						//int yU = pCheckUnit->y >> FRACTIONAL_PART;
						int yU = (pCheckUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;
						int xP = newX >> FRACTIONAL_PART;
						int yP = newY >> FRACTIONAL_PART;
						int squareDistance = (xP - xU) * (xP - xU) + (yP - yU) * (yP - yU);
						if ( squareDistance <= pProjectile->proximitySetOffSquareRadius && !(t == game.currentTeam && teams[t].selectedUnit == u && (flags & PROJECTILE_FLAGS_NO_SELF_PROXIMITY_SET_OFF)))
						{
							setOffRequest = 1;
							break;
						}
					}
				}
			}
		}
		if (flags & PROJECTILE_FLAGS_IMMEDIATE_SETOFF)
			setOffRequest = 1;
		if (iteration > (pProjectile->autoSetOffTime * ((flags & PROJECTILE_FLAGS_CONFIGURABLE_SET_OFF_TIME) ? game.aiData.simulationConfigTimeMultiplier : 1 )) && (flags & PROJECTILE_FLAGS_TIME_SETOFF))
			setOffRequest = 1;
		if (setOffRequest)
		{
			for (int t = 0; t < game.numberOfTeams; t++)
			{
				for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
				{
					uint32_t damage = 0;					
					unit_t *pCheckUnit = &playerUnits[teams[t].firstUnitIndex + u];
					if (flags & PROJECTILE_FLAGS_NO_SELF_DAMAGE)
					{
						if (t == game.currentTeam && u == teams[t].selectedUnit)
						{
							continue;		// do not do any self damage in this case (teleport, blowtorch, etc.).
						}
					}
					if (!(pCheckUnit->state & PLAYER_STATE_DEAD)) // do not consider dead players
					{
						int xU = pCheckUnit->x >> FRACTIONAL_PART;
						int yU = (pCheckUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;
						int xP = newX >> FRACTIONAL_PART;
						int yP = newY >> FRACTIONAL_PART;
						int squareDistance = (xU - xP) * (xU - xP) + (yU - yP) * (yU - yP);
						if ( squareDistance <= pProjectile->directHitSquaredRadius)
						{
							damage = pProjectile->directHitDamage;
								
						}
						else if (squareDistance < pProjectile->damageSquaredRadius)
						{
							damage = pProjectile->directHitDamage  -  pProjectile->directHitDamage * (squareDistance - pProjectile->directHitSquaredRadius ) / (pProjectile->damageSquaredRadius  - pProjectile->directHitSquaredRadius);
						}
						// now compute the total damage each unit will achieve
						if (damage > teams[t].health[u])
						{
							damage = teams[t].health[u];
							if (t == game.currentTeam)
							{
								teamDeaths++;
							}
							else
							{
								enemyDeaths++;
							}
						}
						if (t == game.currentTeam)
						{
							teamDamage += damage;
						}
						else
						{
							enemyDamage += damage;
						}
					}
				}
			}
		}
		ux = newX;
		uy = newY;
		iteration++;
	}
	game.aiData.simulationIterationNumber = iteration;
	if (setOffRequest)
	{  // if the projectile has set off, then the simualation is over
		*finished = 1;
		if (teamDeaths >= remainingCurrentTeamMembers)
		{
			return -32768;  // worst case ever
		}
		else
		{
			int16_t points = enemyDeaths * bonusEnemyDeath + enemyDamage * bonusEnemyDamage;
			points -= ( teamDeaths * malusTeamDeath + teamDamage * malusTeamDamage);
			return points;
		}
	}
	else
	{
		if (game.aiData.simulationIterationNumber < MAX_AI_PROJECTION_ITERATIONS)
		{
			// save data for later use
			game.aiData.projectile.x = ux;
			game.aiData.projectile.y = uy;
			game.aiData.projectile.vx = vx;
			game.aiData.projectile.vy = vy;
			*finished = 0;			
		}
		else
		{
			*finished = 1;
		}
		return 0;
	}
}
uint16_t aiHandler(uint8_t t, uint8_t u)
{
	if (game.gameSubState == GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_SHOW_DAMAGES)
	{
		game.aiData.aiState = AI_STATE_CHOOSE_ACTION;
		game.aiData.aiStateParam = 0;		
	}
	if (teams[t].type == TEAM_CPU_DUMB)
	{
		unit_t *pUnit = &playerUnits[teams[t].firstUnitIndex + u];
		switch (game.aiData.aiState)
		{
			case AI_STATE_CHOOSE_ACTION:
			{
				if (game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING && (pUnit->state & (PLAYER_STATE_IDLE | PLAYER_STATE_WALKING)))
				{
					// lets evaluate what to do and in case, fire
					// let's decide if we move, jump. In every case we evaluate a random shoot value and if it does something positive, do it!
					uint16_t decision = rand();
					game.currentAimAngle = 0;
					// reset simulation
					game.aiData.simulationIterationNumber = 0;
					game.aiData.simulationForce = (rand() & 63) << 2;
					game.aiData.simulationAngle = rand() & 255;
					if (game.fireRound == 0)
					{
						// let's find a weapon
						uint8_t maxW = 0;
						for (uint8_t w = 0; w < sizeof(aiWeapons); w++)
						{
							if (teams[t].inventory[aiWeapons[w]] != 0)
								maxW++;
						}
						uint8_t selectedWeapon = rand() % maxW;
						maxW = 0;
						for (uint8_t w = 0; w < sizeof(aiWeapons); w++)
						{
							if (teams[t].inventory[aiWeapons[w]] != 0)
							{
								if (maxW == selectedWeapon)
								{
									game.aiData.simulationWeapon =  aiWeapons[w];
									break;
								}							
								maxW++;
							}
						}						
					}
					game.aiData.simulationConfigTimeMultiplier = 1 + (rand() % 5);
					// 
					//
					uint8_t command = COMMAND_NONE;
					uint8_t moveResult = 0;
					if (decision < AI_MOVEMENT_BIN_PROBABILITY || game.aiData.aiStateParam > 0)
					{
						// we want to move.
						uint16_t moveDecision = rand(); 
						if (moveDecision < AI_JUMP_BIN_PROBABILITY /*&& !game.aiData.aiStateParam > 0*/)	
							command = COMMAND_JUMP;
						else
						{
							command = game.aiData.goingRight ? COMMAND_RIGHT : COMMAND_LEFT;
						}
						// now: create a copy of the movement player.
						unit_t tempUnit;
						tempUnit.x = pUnit->x;
						tempUnit.y = pUnit->y;
						tempUnit.vx = pUnit->vx;
						tempUnit.vy = pUnit->vy;
						tempUnit.state = pUnit->state;
						moveResult = evaluateAiMovement(&tempUnit, command, MAX_MOVEMENT_FRAME_LINE);
						if (moveResult)
						{
							game.aiData.goingRight = !game.aiData.goingRight;
							game.aiData.aiStateParam = 0;
							command = COMMAND_NONE;
						}
						else
						{
							if (game.aiData.aiStateParam == 0 && command != COMMAND_JUMP)
								game.aiData.aiStateParam = PLAYER_WALK_NUMFRAMES;
							else if (game.aiData.aiStateParam > 0)
								game.aiData.aiStateParam--;
						}
					}
					// in any case launch a fire simulation
					uint8_t finished = 0;		
					calculateInitialProjectileData(&game.aiData.projectile, &playerUnits[(teams[t].firstUnitIndex + u)], game.aiData.simulationAngle, playerUnits[(teams[t].firstUnitIndex + u)].state & PLAYER_FACING_RIGHT, game.aiData.simulationWeapon, game.aiData.simulationForce );
					int16_t value = evaluateAIProjectileDamage(256, 1, 512, 2, &finished, MAX_FRAME_LINE);
					if (finished)
					{
						if (value > 0)
						{		
							// abort player movement
							game.aiData.aiStateParam = 0;
							// set weapons data
							game.currentWeapon = game.aiData.simulationWeapon;
							game.currentAimAngle = game.aiData.simulationAngle;
							game.configTimeMultiplier = game.aiData.simulationConfigTimeMultiplier;
							game.configBounceMultiplier = 1;
							// fire!
							game.aiData.aiState = AI_STATE_FIRE;
							return 'g';
						}
					}
					else
					{
						// the simulation did not finish on time. Let's continue later if our initial decision was not to move or jump
						if (!((decision < AI_MOVEMENT_BIN_PROBABILITY || game.aiData.aiStateParam > 0) && (moveResult == 0)))
						{
							// are we running out of time? Play our last card!
							if (game.turnTime == 0 && (rand() & 65535) < ADVANCED_STRATEGY_BIN_PROBABILITY && game.fireRound == 0)
							{
								// if we ran out of time, we should use advanced weapons like airstrike or teleport.
								uint8_t remainingEnemies = 0;
								int32_t playerX = playerUnits[teams[t].firstUnitIndex + u].x;
								int32_t farthestDistance = 0;
								unit_t * farthestUnit = NULL;
								for (uint8_t team = 0; team < game.numberOfTeams; team++)
								{
									if (t == team)
										continue;
									for (uint8_t unit = 0; unit < game.numberOfUnitsPerTeam; unit++)
									{
										if (teams[team].health[unit])
										{
											unit_t *pTestUnit = &playerUnits[teams[team].firstUnitIndex + unit];
											int32_t x = pTestUnit->x ;
											remainingEnemies++;
											int32_t distance = x - playerX > 0 ? x - playerX : playerX - x;
											if (distance >= farthestDistance)
											{
												farthestDistance = distance;
												farthestUnit = pTestUnit;	
											}
										}
									}
								}
								//
								if (teams[t].inventory[WEAPON_AIRSTRIKE] && (rand() & 65535) < AIRSTRIKE_BIN_PROBABILITY)
								{
									
									// abort player movement
									game.aiData.aiStateParam = 0;
									// set weapons data
									game.currentWeapon = WEAPON_AIRSTRIKE;
									game.currentAimAngle = 0;
									game.configTimeMultiplier = 1;
									game.configBounceMultiplier = 1;
									game.xPointer = farthestUnit->x >> FRACTIONAL_PART;
									game.yPointer = farthestUnit->y >> FRACTIONAL_PART;
									// fire!
									game.gameSubState = GAME_SUBSTATE_FOCUS_ON_AIRSTRIKE_DESTINATION;
									game.stateTime = 0;
									game.automaticRoundsFired = 0;									
									return '0';									
								}
								else if (remainingEnemies <= ((uint8_t) teams[t].inventory[WEAPON_TELEPORT]))
								{  // teleport might be an option
									game.aiData.aiStateParam = 0;
									// set weapons data
									game.currentWeapon = WEAPON_TELEPORT;
									game.currentAimAngle = 0;
									game.configTimeMultiplier = 1;
									game.configBounceMultiplier = 1;
									game.xPointer = farthestUnit->x >> FRACTIONAL_PART;
									game.yPointer = farthestUnit->y >> FRACTIONAL_PART;
									// fire!
									//game.aiData.aiState = AI_STATE_FIRE;
									game.gameSubState = GAME_SUBSTATE_FOCUS_ON_AIRSTRIKE_DESTINATION;
									game.stateTime = 0;
									game.automaticRoundsFired = 0;
									return '0';									
								}
							}
							game.aiData.aiState = AI_STATE_CONTINUE_SIMULATION;
							return 0;							
						}
					}
					// if we are here, either the simulation did not complete on time, or the simulation did complete but it gave a negative result. So we move (if we have to) or we idle for next frame
					if (command == COMMAND_JUMP)
						return 'f';
					else if (command == COMMAND_LEFT)
					{
						return 'a';
					}
					else if (command == COMMAND_RIGHT)
					{
						return 'd';						
					}
				}
				return 0;
				break;
			}
			case AI_STATE_CONTINUE_SIMULATION:
				if (game.gameSubState != GAME_SUBSTATE_PLAYER_MOVING)
				{
					game.aiData.aiState = AI_STATE_CHOOSE_ACTION;
					return 0;
				}
				else
				{
					uint8_t finished = 0;
					int16_t value = evaluateAIProjectileDamage(256, 1, 512, 2, &finished, MAX_FRAME_LINE);
					if (finished)
					{
						if (value > 0)
						{
							game.currentWeapon = game.aiData.simulationWeapon;
							game.currentAimAngle = game.aiData.simulationAngle ;
							game.configTimeMultiplier = game.aiData.simulationConfigTimeMultiplier;
							game.aiData.aiState = AI_STATE_FIRE;
							game.configBounceMultiplier = 1;
							return 'g';
						}
						else
						{
							game.aiData.aiState = AI_STATE_CHOOSE_ACTION;	
							return 0;
						}
					}
				}
				break;
			case AI_STATE_FIRE: // firing
				if (game.gameSubState == GAME_SUBSTATE_FIRING)
				{
					if (game.firingForce >= game.aiData.simulationForce)
					{
						game.aiData.aiState = AI_STATE_CHOOSE_ACTION;
						return 0;		// release the key
					}
					else
					{	// continue pressing
						return 'g';
					}					
				}	
				else 
				{
					game.aiData.aiState = AI_STATE_CHOOSE_ACTION;
				}
				return 0;			
				break;			
		}
	}
	else if (teams[t].type == TEAM_CPU_SMART)
	{
/*				switch (game.aiData.aiState)
				{
					case AI_STATE_EVALUATE_DAMAGES:
					// we place a nominal explosion into the selected player and we evaluate the damage on the other units. Then we sort the array by damage.
					
					break;
				}
*/
	}
	return 0;
}

unit_t * convertStaticToProjectile(staticUnit_t * sU)
{
	unit_t *pUnit = NULL;
	for (int n = 1; n < MAX_INGAME_PROJECTILES; n++)		// projectile 0 is always the player current's projectile
	{
		if (projectileUnits[n].state == PROJECTILE_STATE_NOT_CREATED)
		{
			pUnit = &projectileUnits[n];
			break;
		}
	}
	if (pUnit == NULL)  // not found. Return null
		return NULL;
	if (sU->type == STATIC_UNIT_TYPE_MINE)
	{
		pUnit->type = PROJECTILE_ACTIVE_MINE;
	}
	else if (sU->type == STATIC_UNIT_TYPE_AMMO_CRATE)
	{
		pUnit->type = PROJECTILE_EXPLODING_CRATE;
	}
	else if (sU->type == STATIC_UNIT_TYPE_MEDIPACK)
	{
		pUnit->type = PROJECTILE_EXPLODING_MEDIPACK;		
	}
	pUnit->x = sU->x << FRACTIONAL_PART;
	pUnit->y = sU->y << FRACTIONAL_PART;
	pUnit->vx = 0;
	pUnit->vy = 0;

	sU->type = STATIC_UNIT_TYPE_NONE;
	pUnit->state = PROJECTILE_STATE_FLYING;
	return pUnit;
}
void convertProjectileToStatic(unit_t *pUnit)
{
	int staticType = 0;
	if (pUnit->type == PROJECTILE_INACTIVE_MINE || pUnit->type == PROJECTILE_ACTIVE_MINE)
	{
		staticType = STATIC_UNIT_TYPE_MINE;
	}
	else if (pUnit->type == PROJECTILE_AMMO_CRATE)
	{
		staticType = STATIC_UNIT_TYPE_AMMO_CRATE;
	}
	else if (pUnit->type == PROJECTILE_MEDIPACK)
	{
		staticType = STATIC_UNIT_TYPE_MEDIPACK;
	}

	for (int i = 0; i < MAX_STATIC_UNITS; i++ )
	{
		if (staticUnits[i].type == STATIC_UNIT_TYPE_NONE)
		{
			staticUnits[i].x = pUnit->x >> FRACTIONAL_PART;
			staticUnits[i].y = pUnit->y >> FRACTIONAL_PART;
			staticUnits[i].type = staticType;
			pUnit->state = PROJECTILE_STATE_NOT_CREATED;
			return;
		}
	}
}
uint8_t physics(unit_t **ppfastestMovingUnit, uint8_t doNotCalculatePlayers, uint16_t *message, uint32_t *messageParam)
{
	unit_t *oldFocusedUnit = * ppfastestMovingUnit;
	uint16_t nullMessage;
	uint32_t nullParam;
	if (message == 0)
		message = &nullMessage;
	if (messageParam == 0)
		messageParam = &nullParam;
	int topSquareSpeed = 0;
	uint8_t somethingHappened = 0;
	if (game.screenShake)
	{
		if (game.lastViewY < SCREEN_SIZE_Y / 2)
			game.yExplosionShakeOffset = explosionScreenOscillation[game.screenShake];
		else 
			game.yExplosionShakeOffset = -explosionScreenOscillation[game.screenShake];
		game.screenShake++;
		somethingHappened = 1;
		// show screen shake effects
		if (game.screenShake >= sizeof (explosionScreenOscillation))
			game.screenShake = 0;
	}
	// static units
	int xNp = 1;
	for (int s = 0; s < MAX_STATIC_UNITS; s++)
	{
		uint8_t proximityDetected = 0;
		// we deal with different static objects differently
		if (staticUnits[s].type == STATIC_UNIT_TYPE_MINE)
		{
			// check if there is any player in the nearby vicinity
			for (int t = 0; t < game.numberOfTeams && proximityDetected == 0; t++)
			{
				for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
				{
					unit_t *pUnit = &playerUnits[teams[t].firstUnitIndex + u];
					if (pUnit->state != PLAYER_STATE_DEAD)
					{
						int xU = pUnit->x >> FRACTIONAL_PART;
						//int yU = pUnit->y >> FRACTIONAL_PART;
						int yU = (pUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;
						int xS = staticUnits[s].x;
						int yS = staticUnits[s].y;
						int squareDistance = (xS - xU) * (xS - xU) + (yS - yU) * (yS - yU);
						if ( squareDistance <= projectiles[PROJECTILE_EXPLODING_MINE].proximitySetOffSquareRadius)
						{
							somethingHappened = 1;
							// find a free projectile unit
							for ( ; xNp < MAX_INGAME_PROJECTILES; xNp++)
							{
								unit_t *pPu = &projectileUnits[xNp];
								if (pPu->state == PROJECTILE_STATE_NOT_CREATED)
								{
									// create projectile
									pPu->state = PROJECTILE_STATE_FLYING;
									pPu->frameFlags = 0;
									pPu->frameNumber = 0;
									pPu->lastFrameTick = 0;
									pPu->type = PROJECTILE_EXPLODING_MINE;
									pPu->vx = 0;
									pPu->vy = 0;
									pPu->x = xS << FRACTIONAL_PART;
									pPu->y = yS << FRACTIONAL_PART;
									// free current static object
									staticUnits[s].type = STATIC_UNIT_TYPE_NONE;
									proximityDetected = 1;
									game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS; // we got an explosion...
									break;
								}
							}
							break;
						}	
					}
				}
			}
		}
		else if (staticUnits[s].type == STATIC_UNIT_TYPE_AMMO_CRATE || staticUnits[s].type == STATIC_UNIT_TYPE_MEDIPACK)
		{
			// check if there is any player in the nearby vicinity
			int8_t closerT = -1;
			int8_t closerU;
			uint32_t closerDistance = 0xFFFFFFFF;
			for (int t = 0; t < game.numberOfTeams; t++)
			{
				for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
				{
					unit_t *pUnit = &playerUnits[teams[t].firstUnitIndex + u];
					if (pUnit->state != PLAYER_STATE_DEAD)
					{
						int xU = pUnit->x >> FRACTIONAL_PART;
						//int yU = pUnit->y >> FRACTIONAL_PART;
						int yU = (pUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;						
						int xS = staticUnits[s].x;
						int yS = staticUnits[s].y;
						int squareDistance = (xS - xU) * (xS - xU) + (yS - yU) * (yS - yU);
						if ( squareDistance <= projectiles[PROJECTILE_EXPLODING_MINE].proximitySetOffSquareRadius && closerDistance > squareDistance)
						{
							closerU = u;
							closerT = t;
							closerDistance = squareDistance;							
							somethingHappened = 1;
						}
					}
				}
			}
			if (closerT != -1)
			{
				if (staticUnits[s].type == STATIC_UNIT_TYPE_AMMO_CRATE)	
				{
					// determine how many ammo have limited ammo
					uint8_t wn = 0;
					for (uint8_t w = 0; w < NUMBER_OF_WEAPONS; w++)
					{
						int8_t a = teams[closerT].inventory[w];
						if ( a!= -1 && a < 9)
							wn++;
					}
					uint8_t selectedWeapon = rand() % wn;
					wn = 0;
					for (uint8_t w = 0; w < NUMBER_OF_WEAPONS; w++)
					{
						int8_t a = teams[closerT].inventory[w];
						if ( a!= -1 && a < 9)
						{
							if (selectedWeapon == wn)
							{
								teams[closerT].inventory[w]++;
								*messageParam = w;
								*message |= MESSAGE_BONUS_WEAPON;
								break;
							}
							wn++;													
						}
					}
					triggerFx(PATCH_RELOAD, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);				
				}
				else
				{					
					triggerFx(PATCH_MEDIPACK, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);
					teams[closerT].health[closerU] += MEDIPACK_HEALTH;
					if (teams[closerT].health[closerU] >= 100)
						teams[closerT].health[closerU] = 100;
					*message |= MESSAGE_BONUS_HEALTH; 
				}
				// consume the bonus.
				staticUnits[s].type = STATIC_UNIT_TYPE_NONE;
			}
		}

	}
	// projectiles and explosions. These are treated as single-pixel shaped.
	for (int i = 0; i < MAX_INGAME_PROJECTILES; i++)
	{
		uint8_t setOffRequest = 0;
		unit_t *pUnit = &projectileUnits[i];
		if (pUnit->state != PROJECTILE_STATE_NOT_CREATED && (!(pUnit->type & PROJECTILE_TYPE_EFFECT))  ) // we need to consider only existing projectiles.
		{
			// this projectile is active. Let's see which projectile it is
			int32_t newX = pUnit->x + pUnit->vx;
			int32_t newY = pUnit->y + pUnit->vy;	
			int16_t nx = newX >> FRACTIONAL_PART;
			int16_t ny = newY >> FRACTIONAL_PART;		
			int16_t x = pUnit->x >> FRACTIONAL_PART;
			int16_t y = pUnit->y >> FRACTIONAL_PART;
			int type = projectileUnits[i].type;
			const projectile_t *pProjectile = &projectiles[type];
			uint32_t flags = pProjectile->flags;
			if (pUnit->state & PROJECTILE_STATE_FLYING && !(flags & PROJECTILE_FLAGS_INVISIBLE) )		// focus on flying projectile, if visible.
				*ppfastestMovingUnit = pUnit;
			if (newY > (levels[currentLevel].pixelSizeY << FRACTIONAL_PART) || newX < - (pProjectile->halfWidth << FRACTIONAL_PART) || newX > ((pProjectile->halfWidth  + levels[currentLevel].pixelSizeX) << FRACTIONAL_PART) || (newY < 0 && pUnit->vy < 0 && !(flags & PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY)))
			{
				// remove off-screen objects, or those flying up, not subjected to gravity...
				pUnit->state = PROJECTILE_STATE_NOT_CREATED;
				if (pUnit->type == PROJECTILE_DYNAMITE)
					stopLoopingFx(WEAPON_SOUND_CHANNEL, 0);
				continue;
			}	
			if (pUnit->state != PROJECTILE_STATE_NOT_CREATED && (flags & PROJECTILE_FLAGS_INVISIBLE))
			{
				somethingHappened = 1;
			}
			int squareSpeed = (pUnit->vx >> FRACTIONAL_PART) * (pUnit->vx >> FRACTIONAL_PART) + (pUnit->vy >> FRACTIONAL_PART) * (pUnit->vy >> FRACTIONAL_PART);
			if (squareSpeed > topSquareSpeed && !(flags & PROJECTILE_FLAGS_INVISIBLE))
			{
				*ppfastestMovingUnit = pUnit;
			}
			if (flags & PROJECTILE_FLAGS_SUBJECTED_TO_WIND)
			{
				pUnit->vx += game.windSpeed * WIND_SPEED_FACTOR;
			}
			uint8_t bounced = 0;
			if (flags & (PROJECTILE_FLAGS_BOUNCES | PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN))
			{
				int bx, by;
				bounced = bresenhamCollisionCheck(x , y, nx, ny,&bx, &by);
				if (bounced)
				{
					// has collided. We need to check which direction should bounce.
					if (flags & PROJECTILE_FLAGS_SETOFF_WITH_TERRAIN)
					{
						setOffRequest = 1;
					}
					triggerFx(pProjectile->bounceSoundPatch,255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);			
					if (bounced & BOUNCE_VERTICAL)  // we must bounce back in y;
					{
						pUnit->vy =  - FIXED_MULFRACT(pUnit->vy, ( (flags & PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE) ? pProjectile->yBounceFract * game.configBounceMultiplier : pProjectile->yBounceFract));	
						if (pUnit->vy < UNIT_MIN_SPEED && pUnit->vy > - UNIT_MIN_SPEED)
						{
							pUnit->vy = 0;							
						} 
						if (!(bounced & BOUNCE_HORIZONTAL))  // avoid counting two times...
						{
							pUnit->vx =  FIXED_MULFRACT(pUnit->vx, ( (flags & PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE) ? pProjectile->xBounceFract * game.configBounceMultiplier :  pProjectile->xBounceFract));
							if (pUnit->vx < UNIT_MIN_SPEED && pUnit->vx > - UNIT_MIN_SPEED)
							{
								pUnit->vx = 0;
							}							
						}
					}
					if (bounced & BOUNCE_HORIZONTAL)  // we must bounce back in x
					{
						pUnit->vx =  - FIXED_MULFRACT(pUnit->vx, ( (flags & PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE) ? pProjectile->xBounceFract * game.configBounceMultiplier :  pProjectile->xBounceFract));
						if (pUnit->vx < UNIT_MIN_SPEED && pUnit->vx > - UNIT_MIN_SPEED)
						{
							pUnit->vx = 0;
						}
						if (!(bounced & BOUNCE_VERTICAL))  // avoid counting two times...
						{
							pUnit->vy =  FIXED_MULFRACT(pUnit->vy, ( (flags & PROJECTILE_FLAGS_CONFIGURABLE_BOUNCE) ? pProjectile->yBounceFract * game.configBounceMultiplier : pProjectile->yBounceFract));
							if (pUnit->vy < UNIT_MIN_SPEED && pUnit->vy > - UNIT_MIN_SPEED)
							{
								pUnit->vy = 0;
							}	
						}
					}
					newX = bx << FRACTIONAL_PART;
					newY = by << FRACTIONAL_PART;					
				}
			}
			if (flags & PROJECTILE_FLAGS_SUBJECTED_TO_GRAVITY)
			{
				//if (!((bounced & BOUNCE_VERTICAL) || (pUnit->vy >= 0) && getMapPixel(nx, ny + 1)))
				if (!((bounced & BOUNCE_VERTICAL) && pUnit->vy >= 0) && !((pUnit->vy >= 0) && (getMapPixel(nx, ny + 1) | getMapPixel(nx, ny))))
				{
					pUnit->vy += ((customLevelData_t *) levels[currentLevel].customData)->gravity;
					// TODO: maybe these should be put outside...
					if ((pUnit->y >> FRACTIONAL_PART) < WATER_LEVEL && (newY >> FRACTIONAL_PART) >= WATER_LEVEL)
						triggerFx(PATCH_WATER, 255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);
					if ((pUnit->y >> FRACTIONAL_PART) >= WATER_LEVEL)
					{
						pUnit->vx = FIXED_MULFRACT(pUnit->vx, PROJECTILE_WATER_DRAG_FACTOR);
						if (pUnit->vy >= PROJECTILE_WATER_TERMINAL_VELOCITY)		// this could be also implemented as multiplication...
							pUnit->vy  = PROJECTILE_WATER_TERMINAL_VELOCITY;
					}
					somethingHappened = 1;
				}
			}			
			if (pUnit->vx > UNIT_MIN_SPEED || pUnit->vx < - UNIT_MIN_SPEED || pUnit->vy > UNIT_MIN_SPEED || pUnit->vy < - UNIT_MIN_SPEED )
				somethingHappened = 1;
			else if ((flags & PROJECTILE_FLAGS_BECOMES_STATIC) && (game.bombDeployTime >= pProjectile->autoSetOffTime) && (getMapPixel(nx, ny + 1) | getMapPixel(nx, ny)))
			{
				convertProjectileToStatic(pUnit);
			}			
			if (flags & PROJECTILE_FLAGS_PROXIMITY_SETOFF)
			{
				int bx, by;
				if (bresenhamCollisionCheckAgainstUnits(x , y, nx, ny,&bx, &by, pProjectile)) 
				{
					setOffRequest = 1;
					newX = bx << FRACTIONAL_PART;
					newY = by << FRACTIONAL_PART;
				}
			}
			if (flags & PROJECTILE_FLAGS_IMMEDIATE_SETOFF)
				setOffRequest = 1;
			if (flags & PROJECTILE_FLAGS_TIME_SETOFF)
			{
				somethingHappened = 1; // we need to wait until the bomb sets off.
				if (game.bombDeployTime > pProjectile->autoSetOffTime *((flags & PROJECTILE_FLAGS_CONFIGURABLE_SET_OFF_TIME) ? game.configTimeMultiplier : 1 ))
					setOffRequest = 1;	
			}
			// animate projectile if required.
			pUnit->lastFrameTick++;
			if (pUnit->lastFrameTick > pProjectile->projectileDelayBetweenFrames)
			{
				pUnit->lastFrameTick = 0;
				pUnit->frameNumber++;
			}
			// check for end of anim. And set-off if required
			if (pUnit->frameNumber >= pProjectile->projectileNumberOfFrames)
			{
				pUnit->frameNumber = 0;
				if (flags & PROJECTILE_FLAGS_END_OF_ANIM_SETOFF)
					setOffRequest = 1;
			}			
			if (flags & PROJECTILE_FLAGS_END_OF_ANIM_SETOFF)
				somethingHappened = 1;	
			if (setOffRequest)
			{		
				if (pUnit->type == PROJECTILE_DYNAMITE)
					stopLoopingFx(WEAPON_SOUND_CHANNEL, 0);  // stop fuse...
				triggerFx(pProjectile->setOffSoundPatch,255, FX_FLAGS_RETRIG | FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);	
				if (flags & PROJECTILE_FLAGS_SETOFF_TYPE_EXPLOSION)
				{
					game.screenShake = 1;
					if (game.lastViewY < SCREEN_SIZE_Y / 2)
						game.yExplosionShakeOffset = explosionScreenOscillation[0];
					else
						game.yExplosionShakeOffset = -explosionScreenOscillation[0];
				}
				*ppfastestMovingUnit = pUnit; // here we cheat, as explosions are interesting...
				// do damage calculation.
				somethingHappened = 1;
				destroyPlayground(newX >> FRACTIONAL_PART, newY >> FRACTIONAL_PART, pProjectile->damageHole);				
				//pUnit->state = PROJECTILE_STATE_NOT_CREATED;
				pUnit->type = pProjectile->setOffProjectileType;
				if (pProjectile->setOffProjectileType == PROJECTILE_NONE)
				{
					pUnit->state = PROJECTILE_STATE_NOT_CREATED;
				}
				int numberOfSetOffSecondaryProjectiles = pProjectile->setOffSecondaryProjectileNumber;
				int pNi = 1;
				for (int pN = 0; pN < numberOfSetOffSecondaryProjectiles; pN++)
				{
					// try to find the first available projectile
					for (; pNi < MAX_INGAME_PROJECTILES; pNi++)		
					{
						unit_t *pSecProjectile = &projectileUnits[pNi];
						if ((pSecProjectile->state == PROJECTILE_STATE_NOT_CREATED))
						{
							pSecProjectile->state = PROJECTILE_STATE_FLYING;
							pSecProjectile->x = pUnit->x;
							pSecProjectile->y = pUnit->y;
							pSecProjectile->frameNumber = 0;
							pSecProjectile->type = pProjectile->setOffSecondaryProjectileType;
							pSecProjectile->lastFrameTick = 0;
							const projectile_t *pSecProj = &projectiles[pProjectile->setOffSecondaryProjectileType & PROJECTILE_TYPE_MASK];
							pSecProjectile->vx =  (rand() % ( 2 *  (pSecProj->maxSpeed << (FRACTIONAL_PART - PROJECTILE_MAXSPEED_FRACTIONAL_PART) ) ) ) - (pSecProj->maxSpeed << (FRACTIONAL_PART - PROJECTILE_MAXSPEED_FRACTIONAL_PART) );
							pSecProjectile->vy = - (rand() % (  (pSecProj->maxSpeed << (FRACTIONAL_PART - PROJECTILE_MAXSPEED_FRACTIONAL_PART) ) ) );
							break;
						}
					}
				}
				pUnit->frameNumber = 0;
				pUnit->lastFrameTick = 0;		// actually we do not need exact tick.
				// first check players
				for (int t = 0; t < game.numberOfTeams; t++)
				{
					for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
					{		
						if (flags & PROJECTILE_FLAGS_NO_SELF_DAMAGE)
						{
							if (t == game.currentTeam && u == teams[t].selectedUnit)
							{
								continue;		// do not do any self damage in this case (teleport, blowtorch, etc.).
							}
						}							
						unit_t *pCheckUnit = &playerUnits[teams[t].firstUnitIndex + u];	
						if (!(pCheckUnit->state & PLAYER_STATE_DEAD)) // do not consider dead players
						{
							int xU = pCheckUnit->x >> FRACTIONAL_PART;
							int yU = (pCheckUnit->y >> FRACTIONAL_PART) - PLAYER_HEIGHT / 2;
							int xP = newX >> FRACTIONAL_PART;
							int yP = newY >> FRACTIONAL_PART;
							int squareDistance = (xU - xP) * (xU - xP) + (yU - yP) * (yU - yP);
							int damage = 0;
							if ( squareDistance <= pProjectile->directHitSquaredRadius)
							{
								damage = pProjectile->directHitDamage;
							
							}
							else if (squareDistance < pProjectile->damageSquaredRadius)
							{
								damage = pProjectile->directHitDamage  -  pProjectile->directHitDamage * (squareDistance - pProjectile->directHitSquaredRadius ) / (pProjectile->damageSquaredRadius  - pProjectile->directHitSquaredRadius);
							}
							if (damage)
							{
								uint16_t angle = (fastApproxAtan2((xU - xP),  (yU - yP)) >> (32 - 10)) & 1023;
								pCheckUnit->vx += FIXED_MULFRACT(DAMAGE_VELOCITY_COEFFICIENT * damage, sinTable[(angle + 256) & 1023] );
								pCheckUnit->vy += FIXED_MULFRACT(DAMAGE_VELOCITY_COEFFICIENT* damage , sinTable[(angle) & 1023]);
								pCheckUnit->state =  PLAYER_STATE_FALLING | (pCheckUnit->state & PLAYER_FACING_RIGHT);								
							}
							// now compute the total damage each unit will achieve
							damage += teams[t].damage[u];
							if (damage > teams[t].health[u])
								damage = teams[t].health[u];
							teams[t].damage[u] = damage;						
						}	
					}
				}
				// now let's see if there are static objects like mines
				for (int s = 0; s < MAX_STATIC_UNITS; s++)
				{
					if (staticUnits[s].type == STATIC_UNIT_TYPE_NONE)
					{
						continue;
					}
					staticUnit_t *sU = &staticUnits[s];
					int xU = sU->x;
					int yU = sU->y;
					int xP = newX >> FRACTIONAL_PART;
					int yP = newY >> FRACTIONAL_PART;
					int squareDistance = (xU - xP) * (xU - xP) + (yU - yP) * (yU - yP);
					int damage = 0;
					if ( squareDistance <= pProjectile->directHitSquaredRadius)
					{
						damage = pProjectile->directHitDamage;
					}
					else if (squareDistance < pProjectile->damageSquaredRadius)
					{
						damage = pProjectile->directHitDamage  -  pProjectile->directHitDamage * (squareDistance - pProjectile->directHitSquaredRadius ) / (pProjectile->damageSquaredRadius  - pProjectile->directHitSquaredRadius);
					}
					if (damage)
					{
						unit_t *newProjectileUnit;
						// create an unit
						uint8_t sUType = sU->type;
						newProjectileUnit = convertStaticToProjectile(sU);
						if (newProjectileUnit != NULL)
						{
							if (sUType == STATIC_UNIT_TYPE_MINE)
							{
								// is it a mine? let's move it!
								uint16_t angle = (fastApproxAtan2((xU - xP),  (yU - yP)) >> (32 - 10)) & 1023;
								newProjectileUnit->vx += FIXED_MULFRACT(DAMAGE_VELOCITY_COEFFICIENT * damage, sinTable[(angle + 256) & 1023] );
								newProjectileUnit->vy += FIXED_MULFRACT(DAMAGE_VELOCITY_COEFFICIENT* damage , sinTable[(angle) & 1023]);
							}
						}
					}					
				}
			}
			pUnit->x = newX;
			pUnit->y = newY;
			if (flags & PROJECTILE_FLAGS_HAS_DIRECTIONS)
			{
				uint8_t angle = 0xF & (( (1 << 27) + fastApproxAtan2(pUnit->vx, pUnit->vy)) >> 28);
				pUnit->frameNumber = directionalSprites16[angle].frame;				
				pUnit->frameFlags = directionalSprites16[angle].flags;
			}
		}
		else if ((pUnit->state != PROJECTILE_STATE_NOT_CREATED) && (pUnit->type & PROJECTILE_TYPE_EFFECT)) // we need to consider only existing projectiles.
		{  				
			// this is an effect, e.g. explosion
			int type = pUnit->type & ~(PROJECTILE_TYPE_EFFECT);
			const projectile_t *pProjectile = &projectiles[type];
			pUnit->lastFrameTick++;  // animate effect
			if (pUnit->lastFrameTick > pProjectile->projectileDelayBetweenFrames)
			{
				pUnit->lastFrameTick = 0;
				pUnit->frameNumber++;
			}
			if (pUnit->frameNumber >= pProjectile->projectileNumberOfFrames)
			{
				pUnit->state = PROJECTILE_STATE_NOT_CREATED;			
			}
			else
			{
				somethingHappened = 1;
				*ppfastestMovingUnit = pUnit; 
			}
		}
	}
	// players
	if (!doNotCalculatePlayers)
	{
		for (int t = 0; t < game.numberOfTeams; t++)
		{
			for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
			{
				unit_t *pUnit = &playerUnits[teams[t].firstUnitIndex + u];
				if (pUnit->state != PLAYER_STATE_DEAD)
				{
					// add gravity if there is no pixel underneath the player...
					if (!getMapPixel(pUnit->x >> FRACTIONAL_PART, (pUnit->y >> FRACTIONAL_PART) + 1) || pUnit->vy < 0)
						pUnit->vy += ((customLevelData_t *) levels[currentLevel].customData)->gravity;
					somethingHappened |= movePlayer(pUnit, 0);
					// kill players that are below the water level.
					if ((pUnit->y >> FRACTIONAL_PART) >= WATER_LEVEL + PLAYER_HEIGHT)
					{
						teams[t].damage[u] = teams[t].health[u];
					}
					if (pUnit->state & PLAYER_STATE_FALLING)
					{
						pUnit->lastFrameTick++;		// animate player falling
						if (pUnit->lastFrameTick > ANIMATION_WALKING_FRAME_DELAY)
						{
							pUnit->lastFrameTick = 0;
							pUnit->frameNumber++;
							if (pUnit->frameNumber >= PLAYER_FLY_AWAY_NUMFRAMES)
							{
								pUnit->frameNumber = 0;
							}
						}
						int squareSpeed = (pUnit->vx >> FRACTIONAL_PART) * (pUnit->vx >> FRACTIONAL_PART) + (pUnit->vy >> FRACTIONAL_PART) * (pUnit->vy >> FRACTIONAL_PART);
						if (squareSpeed > topSquareSpeed)
						{
							*ppfastestMovingUnit = pUnit;
							topSquareSpeed = squareSpeed + FASTEST_UNIT_DELTA_SQUARE_SPEED; // this should prevent the camera from jumping back and forth
						}
					}
				}
			
			}
		}	
	}
	// now, to avoid to shuffle the camera too many time, once we found a moving object, we do not change 
	if (oldFocusedUnit != NULL && (oldFocusedUnit->vx != 0 || oldFocusedUnit->vy != 0) && oldFocusedUnit->state != PROJECTILE_STATE_NOT_CREATED && oldFocusedUnit->state != PLAYER_STATE_DEAD /*&& !(oldFocusedUnit->state & PLAYER_STATE_FIRING)*/)
	{
		
		*ppfastestMovingUnit = oldFocusedUnit;
	}
	game.bombDeployTime++;	
	return somethingHappened;
}
int getNextTeam()
{
	int team = 0;	// actually game.numberOfTeams is > 0, but we need to suppress warnings.
	for (int t = 0 ; t < game.numberOfTeams; t++)
	{
	    team = (t + game.currentTeam + 1) % game.numberOfTeams;
		int u =	getNextAvailableTeamUnit(team);
		if (u >= 0)
		{
			break;
		}
	}
	game.fireRound = 0;
	return team;
}


void fireCurrentWeapon(unit_t *pUnit, uint8_t shootRight)
{	
	int8_t weapon = game.currentWeapon;
	int32_t speed;
	int8_t projectileType = weaponsData[weapon].projectileType;
	int proj;
	for (proj = 0; proj < MAX_INGAME_PROJECTILES; proj ++)
	{
		if (projectileUnits[proj].state == PROJECTILE_STATE_NOT_CREATED)
			break;
	}
	if (proj == MAX_INGAME_PROJECTILES)
	{
		return;		// we cannot deploy another projectile
	}
	speed = projectiles[projectileType].maxSpeed << (FRACTIONAL_PART - PROJECTILE_MAXSPEED_FRACTIONAL_PART);
	if (!(projectiles[projectileType].flags & PROJECTILE_FLAGS_FIXED_LAUNCH_SPEED))
	{
		speed = (speed * game.firingForce) >> 8;
	}
	if (weaponsData[weapon].flags & WEAPON_FLAGS_AIRDROPPED)
	{
		// calculate the time the projectile would take reaching the deisgned y position.
		// The speed of the airstrike is: positive is the destination is > MAP_SIZE_X * TILE_SIZE_X /2. Negative otherwhise
		if (game.xPointer < MAX_MAP_SIZE_X * TILE_SIZE_X / 2)
			speed = - speed;
		int32_t f2 = 2 * ((game.yPointer - AIRDROP_Y) << FRACTIONAL_PART) / ((customLevelData_t *) levels[currentLevel].customData)->gravity;
		int32_t f = fastApproximateSqrt(f2 << FRACTIONAL_PART) >> FRACTIONAL_PART;
		// now, calculate the deltaX as speed * f + the position of the round
		int32_t deltaX = - speed * f + speed * weaponsData[weapon].delayBetweenRounds * (2 * game.automaticRoundsFired - weaponsData[weapon].numberOfAutomaticRounds + 1) / 2;
		projectileUnits[proj].x = (game.xPointer << FRACTIONAL_PART) + deltaX;
		projectileUnits[proj].y = AIRDROP_Y << FRACTIONAL_PART;
		projectileUnits[proj].vx = speed;
		projectileUnits[proj].vy = 0;
	}
	else
	{
		int currentAngle = (shootRight ? game.currentAimAngle + 128 :  128 + 255 - game.currentAimAngle ) << 1;
		if (weaponsData[weapon].maxAngleError)
		{
			currentAngle = (currentAngle + ((rand() % (weaponsData[weapon].maxAngleError << 1) ) - weaponsData[weapon].maxAngleError ) ) & (NUMBER_OF_SINTABLE_ENTRIES - 1);
		}
		int deltaX, deltaY = 0;
		if (weaponsData[weapon].flags & WEAPON_FLAGS_NO_AIMPOINTER)
		{
			deltaX = shootRight  ? PROJECTILE_FIRING_DISTANCE : - PROJECTILE_FIRING_DISTANCE;
			deltaY = - PROJECTILE_FIRING_DISTANCE;			
		}
		else
		{
			deltaX = (FIXED_MULFRACT(PROJECTILE_FIRING_DISTANCE, sinTable[currentAngle]) + (shootRight ? PROJECTILE_FIRING_DISTANCE : - PROJECTILE_FIRING_DISTANCE));
			deltaY = - FIXED_MULFRACT(PROJECTILE_FIRING_DISTANCE, sinTable[(currentAngle + 256) & 1023]);	
		}
		if (weaponsData[weapon].flags & WEAPON_FLAGS_TELEPORT)
		{
			projectileUnits[proj].x = game.xPointer << FRACTIONAL_PART;
			projectileUnits[proj].y = (game.yPointer - PLAYER_HEIGHT / 2) << FRACTIONAL_PART;			
		}
		else if (weaponsData[weapon].flags & WEAPON_FLAGS_BLOWTORCH)
		{
			projectileUnits[proj].x = pUnit->x;
			projectileUnits[proj].y = pUnit->y - ((PLAYER_HEIGHT / 2 + 1) << FRACTIONAL_PART);			
		}
		else if (weaponsData[weapon].flags & WEAPON_FLAGS_DRILL)
		{
			projectileUnits[proj].x = pUnit->x + (1 << FRACTIONAL_PART);
			projectileUnits[proj].y = pUnit->y; //+ (1 << FRACTIONAL_PART);			
		}
		else
		{
			projectileUnits[proj].x = pUnit->x + deltaX;
			projectileUnits[proj].y = pUnit->y - ((PLAYER_HEIGHT / 2) << FRACTIONAL_PART) + deltaY;			
		}
		projectileUnits[proj].vx = FIXED_MULFRACT(speed, sinTable[currentAngle]);
		projectileUnits[proj].vy = - FIXED_MULFRACT(speed , sinTable[(currentAngle + 256) & 1023]);
	}
	if (weaponsData[weapon].flags & WEAPON_FLAGS_BLOWTORCH)
	{
		uint8_t angle = ((3 * (game.currentAimAngle + 128)) >> 8) + 3 * (0 != shootRight) ; 
		switch (angle)
		{
			case 0:	// left up
				projectileUnits[proj].type = PROJECTILE_BLOWTORCH_UP_LEFT;
				break;
			case 1: // left straight
				projectileUnits[proj].type = PROJECTILE_BLOWTORCH;
				break;
			case 2: // left down
				projectileUnits[proj].type = PROJECTILE_BLOWTORCH_DOWN_LEFT;
				break;			
			case 3: // right up
				projectileUnits[proj].type = PROJECTILE_BLOWTORCH_UP_RIGHT;
				break;
			case 4: // right straight
				projectileUnits[proj].type = PROJECTILE_BLOWTORCH;
				break;
			case 5: // right down
				projectileUnits[proj].type = PROJECTILE_BLOWTORCH_DOWN_RIGHT;
				break;
		}
	}
	else
	{
		projectileUnits[proj].type = projectileType;	
	}
	projectileUnits[proj].frameFlags = 0;
	projectileUnits[proj].frameNumber = 0;
	projectileUnits[proj].state = PROJECTILE_STATE_CREATED | PROJECTILE_STATE_FLYING;
	game.bombDeployTime = 0;			
}
uint8_t placeGirder(int16_t newX, int16_t newY, int16_t *oldX, int16_t *oldY, uint8_t templateNumber, uint8_t oldTemplateNumber)
{
	// first, check if we had a valid placed girder
	if (*oldX >= 0 && *oldY >= 0)
	{
		// remove old girder
		uint8_t sx = girderTemplates[oldTemplateNumber].sizeX;
		uint8_t sy = girderTemplates[oldTemplateNumber].sizeY;
		for (int y = 0; y < sy; y++)
		{
			for (int x = 0; x < sx; x++)
			{
				if (girderTemplates[oldTemplateNumber].tiles[x + y * sx])
				{
					setRamMapTileFromTileCoordinates(x + *oldX, y + *oldY,0);
				}
			}
		}
	}
	// now check if we can put a new template
	uint8_t sx = girderTemplates[templateNumber].sizeX;
	uint8_t sy = girderTemplates[templateNumber].sizeY;
	// boundary check (even if newX and newY should not be less than 0).
	*oldX = -1;
	*oldY = -1;
	if (newX < 0 || newY < 0 || (newX + sx) > MAX_MAP_SIZE_X || (newY + sy) > MAX_MAP_SIZE_Y)
	{
		return 0;
	}
	for (int y = 0; y < sy; y++)
	{
		for (int x = 0; x < sx; x++)
		{
			// we cannot place girders where both the map and the girders are not empty
			if (girderTemplates[templateNumber].tiles[x + y * sx] && getRamMapTileFromTileCoordinates(x + newX, y + newY ))
			{
				return 0;
			}
		}
	}
	// now actually place
	for (int y = 0; y < sy; y++)
	{
		for (int x = 0; x < sx; x++)
		{
			// we cannot place girders where both the map and the girders are not empty
			if (girderTemplates[templateNumber].tiles[x + y * sx] && !getRamMapTileFromTileCoordinates(x + newX, y + newY ))
			{
				setRamMapTileFromTileCoordinates(x + newX, y + newY, girderTemplates[templateNumber].tiles[x + y * sx]);
			}
		}
	}
	*oldX = newX;
	*oldY = newY;
	return 1;
}
uint8_t dropBonus()
{
	// first check if we have enough static unit free
	uint8_t nStatic = 0;
	game.bombDeployTime = 0;	
	for (int s = 0; s < MAX_STATIC_UNITS; s++)
	{
		if (staticUnits[s].type == STATIC_UNIT_TYPE_NONE)
			nStatic++;
	}
	if (nStatic < MAX_INITIAL_MINES)
		return 0;
	// if yes, drop bonus
	uint8_t proj = 0;
	projectileUnits[proj].x = (rand() % (MAX_MAP_SIZE_X * TILE_SIZE_X)) << FRACTIONAL_PART;
	projectileUnits[proj].y = AIRDROP_Y << FRACTIONAL_PART;
	projectileUnits[proj].vx = 0;
	projectileUnits[proj].vy = ((customLevelData_t *) levels[currentLevel].customData)->gravity;
	projectileUnits[proj].state = PROJECTILE_STATE_FLYING;
	projectileUnits[proj].frameNumber = 0;
	projectileUnits[proj].frameFlags = 0;
	projectileUnits[proj].type = PROJECTILE_AMMO_CRATE + (rand() & 1); // RANDOM
	return 1;
}
void reasonablyFastUint16ToArray(uint16_t  n, uint8_t *array)
{
	/*
		Well this function is reasonably fast.  Other variants were tested, converting all the numbers between 0 and 65535.
		snprintf: 3.82s
		itoa 2.03s
		using divisions and "%": 1.79s
		this one: 0.37 s
	*/
	int number = n;
	int mult = 10000;
	int digit;
	int i = 0;
	for (digit = 0; digit < 10 && number >= 0; digit++)
	{
		number -= mult;
	}
	number += mult;
	array[i++] = digit - 1;
	mult = 1000;
	for (digit = 0; digit < 10 && number >= 0; digit++)
	{
		number -= mult;
	}
	number += mult;
	array[i++] =  digit - 1;
	mult = 100;
	for (digit = 0; digit < 10 && number >= 0; digit++)
	{
		number -= mult;
	}
	number += mult;
	array[i++] = digit - 1;
	mult = 10;
	for (digit = 0; digit < 10 && number >= 0; digit++)
	{
		number -= mult;
	}
	number += mult;
	array[i++] = digit - 1;
	mult = 1;
	for (digit = 0; digit < 10 && number >= 0; digit++)
	{
		number -= mult;
	}
	number += mult;
	array[i++] = digit -1;
}
void clearMenuPage(uint8_t menuPage)
{
	if (menuPage == MENU_PAGE_MAIN)
	{
		// clear the screen on the bottom left quadrant
		for (int y = (TITLE_MENU_Y_OFFSET + GAME_LOGO_TILE_HEIGHT) ; y < DELTA_TILE_Y_MENU + 25; y++)
		{
			for (int x = 0; x < SCREEN_SIZE_X / TILE_SIZE_X; x++)
				setRamMapTileFromTileCoordinates(x, y, 0);
		}
	}
	else
	{
		// clear the screen on the bottom right quadrant
		for (int y = DELTA_TILE_Y_MENU; y < DELTA_TILE_Y_MENU + 25; y++)
		{
			for (int x = 40; x < 80; x++)
				setRamMapTileFromTileCoordinates(x, y, 0);
		}
	}
}
void matchInit()
{
	game.gameSubState = GAME_SUBSSTATE_ROUND_INIT;
	game.numberOfRoundsPlayed = 0;
	for (int t = 0 ; t < MAX_TEAMS; t++)
	{
		teams[t].wonRounds = 0;
	}
}
void gameLoop(uint16_t *keyBuffer, uint8_t aiTime)
{
	if (aiTime && (game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING || game.gameSubState == GAME_SUBSTATE_FIRING) && game.gameState == GAME_STATE_PLAYING && (teams[game.currentTeam].type & TEAM_CPU))
	{
		game.aiKey = aiHandler(game.currentTeam, teams[game.currentTeam].selectedUnit);
		return;
	}
	else if (aiTime)
		return;				// don't do AI in the other cases
	uint16_t messageToShow = 0;
	uint32_t messageParam;
	uint8_t command = COMMAND_NONE;	
	static uint16_t oldKey = 0;
	uint16_t key = keyBuffer[0];		
	team_t * pTeam = &teams[game.currentTeam];
	unit_t * pUnit = &playerUnits[pTeam->firstUnitIndex + pTeam->selectedUnit];
	uint8_t manualMoveCamera = 0;
	// debug keys
	if (DEBUG_KEYS && key == 'l')	// change level
		game.gameSubState = GAME_SUBSSTATE_ROUND_INIT;
	if (key == 'c' && oldKey == 'm')	// switch from AI to human and vice versa
	{
		for (int i = 0; i < MAX_TEAMS; i++)
		{
			if (teams[i].type == TEAM_CPU)
				teams[i].type = 0;
			else
				teams[i].type = TEAM_CPU;
		}
		game.aiKey = 0;
		oldKey = 'c';
		return;
	}
	if (DEBUG_KEYS && key == 't' && game.gameState == GAME_STATE_PLAYING)  // test map for debug
	{
		for (int i = 0; i < MAX_PLAYER_UNITS; i++)
		{
			playerUnits[i].vx = 0;
			playerUnits[i].vy = 0;
			playerUnits[i].y = (WATER_LEVEL) << FRACTIONAL_PART;
			playerUnits[i].x = (MAX_MAP_SIZE_X * TILE_SIZE_X - 320 + 20 * i + 150) << FRACTIONAL_PART;
			playerUnits[i].state = PLAYER_STATE_IDLE;			
		}
		for (int t = 0; t < game.numberOfTeams; t++)
		{
			for (int u  = 0 ; u < game.numberOfUnitsPerTeam; u++)
			{
				teams[t].health[u]  = 100;	
			}
		}
		for (int i = 0; i < MAX_STATIC_UNITS; i++)
		{
			staticUnits[i].type = STATIC_UNIT_TYPE_MEDIPACK;
			staticUnits[i].x = MAX_MAP_SIZE_X * TILE_SIZE_X - 320 + 20 * i + 130;
			staticUnits[i].y = (MAX_MAP_SIZE_Y - 10) * TILE_SIZE_Y;
		}
		memset(&gameRamMap, 0, sizeof(gameRamMap));
		for (int x = 64; x < MAX_MAP_SIZE_X; x++)
		{
			setRamMapTileFromTileCoordinates(x, MAX_MAP_SIZE_Y - 3, 1);
			setRamMapTileFromTileCoordinates(x, MAX_MAP_SIZE_Y - 10, 1);
		}
	}
	switch (game.gameState)
	{
		case GAME_STATE_TITLE_SCREEN:
			{	
				int px = 0, py = 0;	
				switch (game.gameSubState)
				{
					case GAME_SUBSTATE_TITLE_PRESS_ANY_KEY:
						game.stateTime++;
						if (game.stateTime > 4 * FPS)
						{
							printLine("Just kidding. Press any key!", 20, 18, TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_COLOR_HIGHLIGHT);
							game.gameSubState = GAME_SUBSTATE_TITLE_WAIT_FOR_ANY_KEY;
						}
					break;
					case GAME_SUBSTATE_TITLE_WAIT_FOR_ANY_KEY:
						if (key)
						{
							game.stateTime = 0;
							game.gameSubState = GAME_SUBSTATE_TITLE_SCROLL_UP;
							// clear menu region
							clearMenuPage(MENU_PAGE_MAIN);			
							drawMenuPage(MENU_PAGE_MAIN, 0);
						}
					break;
					case GAME_SUBSTATE_TITLE_SCROLL_UP:
						game.stateTime++;
						py = game.stateTime;
						if (game.stateTime == TILE_SIZE_Y * DELTA_TILE_Y_MENU)
						{
							game.gameState = GAME_STATE_MENU;
							game.gameSubState = GAME_SUBSTATE_MENU_MAIN;
							game.selectedMenuItem = 0;
							game.menuPage = MENU_PAGE_MAIN;
						}
						break;
				}
				setViewPosition(&px, &py, 1);
			}
			break;
		case GAME_STATE_MENU:
			{
				int px = 0, py = 0;
				int nRow = 0;
				int numberOfItems = 0;
				// get the number of seletable items.
				while ((pMenuPages[game.menuPage] != NULL ) && (pMenuPages[game.menuPage])[nRow].text != NULL )
				{
					if ((pMenuPages[game.menuPage])[nRow].flags & TEXT_FLAGS_IS_ITEM)
						numberOfItems++;
					nRow++;
				}
				// clear menuPage
				clearMenuPage(game.menuPage);
				if (key != oldKey)
				{
					if (EXBUFF_TO_OEM(key) == USB_KEY_UP || key == 'w')
					{
						triggerFx(PATCH_MOVE_MENU, 255, FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);
						if (game.selectedMenuItem > 0)
							game.selectedMenuItem--;
						else
							game.selectedMenuItem = (numberOfItems - 1);
					}
					else if (EXBUFF_TO_OEM(key) == USB_KEY_DOWN  || key == 's')
					{
						triggerFx(PATCH_MOVE_MENU, 255, FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY, 1 << FRACTIONAL_PART);
						if (game.selectedMenuItem < (numberOfItems - 1))
							game.selectedMenuItem++;
						else
							game.selectedMenuItem = 0;
					}
				}
				// we assume that each page has a back item.
				if ((key == 'f' || key == '\r' ) && game.gameSubState != GAME_SUBSTATE_MENU_MAIN && game.selectedMenuItem == (numberOfItems - 1) && oldKey != key )
				{
					game.gameSubState = goToMenuPage(MENU_PAGE_MAIN);
					game.stateTime =  0;					
					triggerFx(PATCH_BACK_MENU, 255, FX_FLAGS_RETRIG, 1 << FRACTIONAL_PART);
				}
				py = 48;
				switch (game.gameSubState)
				{
					case GAME_SUBSTATE_MENU_MAIN:
						if ((key == 'f' || key == '\r' ) && oldKey != key)		// enter
						{
							triggerFx(PATCH_ENTER_MENU, 255, FX_FLAGS_RETRIG, 1 << FRACTIONAL_PART);							
							if (game.selectedMenuItem == 0)
							{
								initFixedSection(1);
								matchInit();
								game.gameState = GAME_STATE_PLAYING;
								game.gameSubState = GAME_SUBSSTATE_ROUND_INIT;						
							}
							else
							{
								game.gameSubState = goToMenuPage(game.selectedMenuItem + MENU_PAGE_MAIN);	
								game.stateTime =  0;
							}
						}
						break;
					case GAME_SUBSTATE_MENU_GO_RIGHT:
						game.stateTime ++;
						px = 20 * game.stateTime;
						if (px >= 320)
						{
							game.gameSubState = game.menuPage;
						}
						break;
					case GAME_SUBSTATE_MENU_GO_LEFT:
						game.stateTime++;
						px = 320 - 20 * game.stateTime;
						if (px == 0)
						{
							game.gameSubState = GAME_SUBSTATE_MENU_MAIN;
							game.menuPage = MENU_PAGE_MAIN;
						}
						break;
					case GAME_SUBSTATE_MENU_OPTIONS:
						px = 320;				
						if ((key == 'f' || key == '\r' ) && oldKey != key)
						{
							triggerFx(PATCH_ENTER_MENU, 255, FX_FLAGS_RETRIG, 1 << FRACTIONAL_PART);							
							switch (game.selectedMenuItem)
							{
								case 0:		// number of Teams
									game.numberOfTeams++;
									if (game.numberOfTeams > MAX_TEAMS)
										game.numberOfTeams = 2;
									game.numberOfUnitsPerTeam = (game.numberOfTeams == 2) ? 4 : 2;										
									break;
								case 1:	   // select team RED CPU/HUMAN
								case 2: // select team BLUE CPU/HUMAN
								case 3:  // select team YELLOW CPU/HUMAN
								case 4: // select team GRAY CPU/HUMAN
									teams[game.selectedMenuItem - 1].type =  teams[game.selectedMenuItem - 1].type ^ TEAM_CPU;
									break;
								case 5:
									game.numberOfRoundsPerMatch++;
									if (game.numberOfRoundsPerMatch > 9)
										game.numberOfRoundsPerMatch = 1;
									break;
							}
						}		
						// print info
						{
							printLine(decimalStrings[game.numberOfTeams], pMenuPages[MENU_PAGE_OPTIONS][1].col + strlen(pMenuPages[MENU_PAGE_OPTIONS][1].text),pMenuPages[MENU_PAGE_OPTIONS][1].row, TEXT_FLAGS_ALIGN_LEFT );
							for (int i = 2; i < 6; i++)
								printLine(teamTypeStrings[0 != (teams[i - 2].type & TEAM_CPU)], pMenuPages[MENU_PAGE_OPTIONS][i].col + strlen(pMenuPages[MENU_PAGE_OPTIONS][i].text),pMenuPages[MENU_PAGE_OPTIONS][i].row, TEXT_FLAGS_ALIGN_LEFT );	
							printLine(decimalStrings[game.numberOfRoundsPerMatch], pMenuPages[MENU_PAGE_OPTIONS][6].col + strlen(pMenuPages[MENU_PAGE_OPTIONS][6].text),pMenuPages[MENU_PAGE_OPTIONS][6].row, TEXT_FLAGS_ALIGN_LEFT );
						}

						break;
					case GAME_SUBSTATE_MENU_HELP:
						px = 320;
						break;
					case GAME_SUBSTATE_MENU_CREDITS:
						px = 320;
						break;
					case GAME_SUBSTATE_MENU_PLOT:
						px = 320;
						break;						
					case GAME_SUBSTATE_MENU_MATCH_SUMMARY:
						{
							px = 320;
							// order the teams in terms of rounds won, and write the table. Since there are at most 4 teams, a crappy sorting algorithm will do the job.
							uint8_t bestTeams[4] = {0, 1, 2, 3};			
							for (uint8_t t = 0; t <  game.numberOfTeams; t++)
							{
								uint8_t bestTeam = t;
								uint8_t maxWon = 0;
								for (uint8_t tm = t; tm <  game.numberOfTeams; tm++)
								{
									uint8_t roundsWon = teams[bestTeams[tm]].wonRounds;
									if (roundsWon > maxWon)
									{
										maxWon = roundsWon;
										bestTeam = tm;
									}
								}
								// now swap
								uint8_t oldTeam = bestTeams[t];
								bestTeams[t] = bestTeams[bestTeam];
								bestTeams[bestTeam] = oldTeam;
							}
							// 		print the team stats
							if (teams[bestTeams[0]].wonRounds == teams[bestTeams[1]].wonRounds)
							{
								printLine("DRAW MATCH", 60, DELTA_TILE_Y_MENU + 3, TEXT_FLAGS_COLOR_HIGHLIGHT | TEXT_FLAGS_ALIGN_CENTER);
							}
							else
							{
								printLine(matchWinnerString[bestTeams[0]], 60, DELTA_TILE_Y_MENU + 3, TEXT_FLAGS_COLOR_HIGHLIGHT | TEXT_FLAGS_ALIGN_CENTER);
							}
							// now print the table
							printLine("   TEAM        ROUNDS WON", 60, DELTA_TILE_Y_MENU + 5, TEXT_FLAGS_COLOR_HIGHLIGHT | TEXT_FLAGS_ALIGN_CENTER);
							for (uint8_t t = 0; t < game.numberOfTeams; t++)
							{
								printLine(teamNames[bestTeams[t]], 53, DELTA_TILE_Y_MENU + 7 + t, TEXT_FLAGS_COLOR_NORMAL | TEXT_FLAGS_ALIGN_CENTER);
								printLine(decimalStrings[teams[bestTeams[t]].wonRounds], 67, DELTA_TILE_Y_MENU + 7 + t, TEXT_FLAGS_COLOR_NORMAL | TEXT_FLAGS_ALIGN_CENTER);
							}	
						}
						break;
				}
				drawMenuPage(game.menuPage, game.selectedMenuItem);
				setViewPosition(&px, &py, 1);				
			}
			break;
		case GAME_STATE_PLAYING:
		{
			// restore indicator of current player, except when moving
			if (game.gameSubState != GAME_SUBSTATE_PLAYER_MOVING && game.gameSubState != GAME_SUBSTATE_PLAYER_MOVING_AFTER_DEPLOYMENT)
				game.showCurrentUnitIndicator = 1;
			if (game.gameState == GAME_SUBSTATE_CONFIRM_SURRENDER || game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING || game.gameSubState == GAME_SUBSTATE_POSITION_SELECTION || game.gameSubState == GAME_SUBSTATE_PLACE_GIRDERS)
			{
				if (game.tickTurnTime == 0)
				{
					if (game.turnTime > 0)
					{
						game.tickTurnTime = FPS - 1;
						game.turnTime--;						
					}
				}
				else
				{
					game.tickTurnTime--;
				}
				if (game.turnTime == 0 && game.tickTurnTime == 0)
				{
					if (game.gameSubState == GAME_SUBSTATE_PLACE_GIRDERS)
					{  // remove Girder by placing one at an invalid position
						placeGirder(-1, -1, &game.xPointer, &game.yPointer, 0, game.configTimeMultiplier -1);
					}
					messageToShow = MESSAGE_TIMEUP;
					game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
					game.fireRound = weaponsData[game.currentWeapon].numberOfManualRounds;
				}
			}
			//pTeam->type = TEAM_CPU;
			if ((pTeam->type & TEAM_CPU) && (game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING || game.gameSubState == GAME_SUBSTATE_FIRING))
			{
				// remove keys from user
				if (!aiTime)
				{
					for (int k = 0; k < 6; k++)
						keyBuffer[k] = 0;
					// let the ai simulate keystrokes.
					key = game.aiKey;
					game.aiKey = 0;
				}
			}
			switch (game.gameSubState)
			{
				case GAME_SUBSTATE_CONFIRM_SURRENDER:
				{
					messageToShow = MESSAGE_CONFIRM_SURRENDER;
					if (key == 'Y' || key == 'y')
					{
						for (int i = 0; i < game.numberOfUnitsPerTeam; i++)
							pTeam->damage[i] = pTeam->health[i];
						game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
						game.stateTime = 0;
					}
					else if  (key == 'N' || key == 'n' || key == ASCII_ESCAPE)
					{
						game.gameSubState = GAME_SUBSTATE_PLAYER_MOVING;	
					}
					drawScene(NULL, 0);
				}
				break;
				case GAME_SUBSTATE_POSITION_SELECTION:
					// wasd move the pointer.  (we use gameCamera with only 0,0, MAX,MAX limits.
					// ESC: cancel the process (return to player moving)
					// Y: to confirm
					// 
					manualMoveCamera = 0;
					for (uint8_t k = 0;  k < 6; k++)
					{
						//if (EXBUFF_TO_OEM(keyBuffer[k]) >= USB_KEY_RIGHT && EXBUFF_TO_OEM(keyBuffer[k]) <= USB_KEY_UP)
						uint16_t kv = keyBuffer[k];
						if (kv == 'd' || kv == 'D')
						{
							cameraUnit.x += 3 << FRACTIONAL_PART;
							manualMoveCamera = 1;
						}
						else if (kv == 'a' || kv == 'A')
						{
							cameraUnit.x -= 3 << FRACTIONAL_PART;
							manualMoveCamera = 1;
						}
						else if (kv == 's' || kv == 'S')
						{
							cameraUnit.y += 3 << FRACTIONAL_PART;
							manualMoveCamera = 1;
						}
						else if (kv == 'w' || kv == 'W')
						{
							cameraUnit.y -= 3 << FRACTIONAL_PART;
							manualMoveCamera = 1;
						}
					}
					if (manualMoveCamera)
					{
						if ((cameraUnit.x >> FRACTIONAL_PART) >= MAX_MAP_SIZE_X * TILE_SIZE_X)
						{
							cameraUnit.x = (MAX_MAP_SIZE_X * TILE_SIZE_X - 1) << FRACTIONAL_PART;
						}
						else if ((cameraUnit.x >> FRACTIONAL_PART) < 0 )
						{
							cameraUnit.x = 0;
						}
						if ((cameraUnit.y >> FRACTIONAL_PART) >= MAX_MAP_SIZE_Y * TILE_SIZE_Y )
						{
							cameraUnit.y = (MAX_MAP_SIZE_Y * TILE_SIZE_Y  - 1) << FRACTIONAL_PART;
						}
						else if ((cameraUnit.y >> FRACTIONAL_PART) < 0 )
						{
							cameraUnit.y = 0;
						}
					}
					else if (key == ASCII_ESCAPE) // ESC
					{
						game.gameSubState = GAME_SUBSTATE_PLAYER_MOVING;
					}		
					else if (key == 'y' || key == 'Y')	// we use different key to launch the strike
					{
						// FIRE THE weapon
						game.xPointer = cameraUnit.x >> FRACTIONAL_PART;
						game.yPointer = cameraUnit.y >> FRACTIONAL_PART;						
						game.stateTime = 0;
						game.automaticRoundsFired = 0;
						game.gameSubState = GAME_SUBSTATE_FOCUS_ON_AIRSTRIKE_DESTINATION;						
					}			
					game.focusedUnit = (unit_t*) &cameraUnit;
					unit_t *pInterestingUnit = pUnit;
					physics(&pInterestingUnit, 1, &messageToShow, &messageParam);
					drawScene((unit_t*)&cameraUnit, 0);
					break;
				case GAME_SUBSTATE_PLACE_GIRDERS:
					{
						// wasd move the pointer.  (we use gameCamera with only 0,0, MAX,MAX limits.
						// ESC: cancel the process (return to player moving)
						// Y: to confirm
						//
						manualMoveCamera = 0;
						uint8_t girderModified = 0;
						uint8_t oldTemplate = game.configTimeMultiplier - 1;
						for (uint8_t k = 0;  k < 6; k++)
						{
							uint16_t kv = keyBuffer[k];
							if (kv == 'd' || kv == 'D')
							{
								cameraUnit.x += 3 << FRACTIONAL_PART;
								manualMoveCamera = 1;
							}
							else if (kv == 'a' || kv == 'A')
							{
								cameraUnit.x -= 3 << FRACTIONAL_PART;
								manualMoveCamera = 1;
							}
							else if (kv == 's' || kv == 'S')
							{
								cameraUnit.y += 3 << FRACTIONAL_PART;
								manualMoveCamera = 1;
							}
							else if (kv == 'w' || kv == 'W')
							{
								cameraUnit.y -= 3 << FRACTIONAL_PART;
								manualMoveCamera = 1;
							}
						}
						if (manualMoveCamera)
						{
							girderModified = 1;
							if ((cameraUnit.x >> FRACTIONAL_PART) >= MAX_MAP_SIZE_X * TILE_SIZE_X)
							{
								cameraUnit.x = (MAX_MAP_SIZE_X * TILE_SIZE_X - 1) << FRACTIONAL_PART;
							}
							else if ((cameraUnit.x >> FRACTIONAL_PART) < 0 )
							{
								cameraUnit.x = 0;
							}
							if ((cameraUnit.y >> FRACTIONAL_PART) >= MAX_MAP_SIZE_Y * TILE_SIZE_Y )
							{
								cameraUnit.y = (MAX_MAP_SIZE_Y * TILE_SIZE_Y  - 1) << FRACTIONAL_PART;
							}
							else if ((cameraUnit.y >> FRACTIONAL_PART) < 0 )
							{
								cameraUnit.y = 0;
							}
						}
						else if (key == ASCII_ESCAPE) // ESC
						{
							game.gameSubState = GAME_SUBSTATE_PLAYER_MOVING;
						
						}
						else if (key == 'y' || key == 'Y')	// we use different key to place girder
						{
							// placing girders won't affect physics, so we already go to the next round
							if (game.xPointer >= 0 && game.yPointer >= 0)
							{
								pUnit->state = PLAYER_STATE_IDLE | (pUnit->state & PLAYER_FACING_RIGHT);
								game.stateTime = 0;
								game.gameSubState = GAME_POST_DAMAGE_SECOND_WAIT;								
							}
							else
							{
								messageToShow = MESSAGE_NO_SPACE_FOR_GIRDER;
							}
						}
						else if ((key == 'f' || key == 'F') && !(oldKey == 'f' || oldKey == 'F'))
						{
							girderModified = 1;
							game.configTimeMultiplier++;
							if (game.configTimeMultiplier > (sizeof (girderTemplates) / sizeof(girderTemplate_t)))
							{
								game.configTimeMultiplier = 1;
							}
						}
						if (girderModified)
						{
							placeGirder((cameraUnit.x >> FRACTIONAL_PART) / TILE_SIZE_X, (cameraUnit.y >> FRACTIONAL_PART) / TILE_SIZE_Y, &game.xPointer, &game.yPointer, game.configTimeMultiplier - 1, oldTemplate);
						}
						// we use the bomb time to select which girder to show...
						game.focusedUnit = (unit_t*) &cameraUnit;
						//physics(&pInterestingUnit, 1); No physics to be calculate when placing girders
						drawScene((unit_t*)&cameraUnit, 0);
					}
					break;
				case GAME_SUBSSTATE_ROUND_INIT:
					roundInit();
					game.gameSubState = GAME_SUBSTATE_FOCUS_TO_ACTIVE_UNIT;
					game.stateTime = 0;
					game.currentTeam = rand() % game.numberOfTeams ;
					getNextAvailableTeamUnit(game.currentTeam);
					pTeam = &teams[game.currentTeam];
					pUnit = &playerUnits[pTeam->firstUnitIndex + pTeam->selectedUnit];					
					cameraUnit.x = pUnit->x;
					cameraUnit.y = pUnit->y;
					game.focusedUnit = pUnit;
					drawScene(pUnit, 1); // force redraw
					game.showCurrentUnitIndicator = 1;
					break;
				case GAME_SUBSTATE_FOCUS_TO_ACTIVE_UNIT:
					game.stateTime++;
					messageToShow = MESSAGE_GET_THEM;
					if (game.stateTime > FOCUS_TO_ACTIVE_UNIT_TIME )
						game.gameSubState = GAME_SUBSTATE_PLAYER_MOVING;
					drawScene(NULL, 0);
					break;
				case GAME_SUBSTATE_PLAYER_MOVING_AFTER_DEPLOYMENT:
					{
						game.showCurrentUnitIndicator  = 0;
						game.stateTime++;
						if (game.stateTime >= weaponsData[game.currentWeapon].afterFirePlayerControlTime)
						{
							unit_t *pInterestingUnit = pUnit; 
							playerHandler(pUnit, COMMAND_NONE, 0);
							physics(&pInterestingUnit, 1, &messageToShow, &messageParam);
							drawScene(pUnit, 0);
							game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
							break;
						}
					}		// NO BREAK: this continues!
				case GAME_SUBSTATE_PLAYER_MOVING:
					game.firingForce = 0;			
					if (pUnit->state & PLAYER_STATE_DEAD)
					{
						game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						playerHandler(pUnit, COMMAND_NONE, 0);
						physics(&pInterestingUnit, 1, &messageToShow, &messageParam);						
						drawScene(pUnit, 0);			
						break;			
					}
					else
					{
						// kill players that are below the water level.
						if ((pUnit->y >> FRACTIONAL_PART) >= WATER_LEVEL + PLAYER_HEIGHT)
						{
							pTeam->damage[pTeam->selectedUnit] = pTeam->health[pTeam->selectedUnit];
							game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
							unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
							playerHandler(pUnit, COMMAND_NONE, 0);
							physics(&pInterestingUnit, 1, &messageToShow, &messageParam);							
							drawScene(pUnit, 0);							
						}
					}		
				/*	*/
					// no keyboard control during jumping
					if (!(pUnit->state & PLAYER_STATE_JUMPING) && !(pUnit->state & PLAYER_STATE_FALLING))
					{
						for (uint8_t k = 0;  k < 6; k++)
						{
							if (EXBUFF_TO_OEM(keyBuffer[k]) >= USB_KEY_RIGHT && EXBUFF_TO_OEM(keyBuffer[k]) <= USB_KEY_UP)
							{
								manualMoveCamera = 1;
								switch (EXBUFF_TO_OEM(keyBuffer[k]))
								{
									case USB_KEY_RIGHT:
										cameraUnit.x += 3 << FRACTIONAL_PART;
										break;
									case USB_KEY_LEFT:
										cameraUnit.x -= 3 << FRACTIONAL_PART;
										break;
									case USB_KEY_DOWN:
										cameraUnit.y += 3 << FRACTIONAL_PART;
										break;
									case USB_KEY_UP:
										cameraUnit.y -= 3 << FRACTIONAL_PART;
										break;
								}
							}												
						}
						if (manualMoveCamera)
						{
							// first we need to check the camera boundaries, for units too close to the screen.
							if ((cameraUnit.x >> FRACTIONAL_PART) >= MAX_MAP_SIZE_X * TILE_SIZE_X  - SCREEN_SIZE_X / 2 )
							{
								cameraUnit.x = (MAX_MAP_SIZE_X * TILE_SIZE_X  - SCREEN_SIZE_X / 2 - 1) << FRACTIONAL_PART;
							}
							else if ((cameraUnit.x >> FRACTIONAL_PART) < SCREEN_SIZE_X / 2 )
							{
								cameraUnit.x = (SCREEN_SIZE_X / 2) << FRACTIONAL_PART;
							}
							if ((cameraUnit.y >> FRACTIONAL_PART) >= MAX_MAP_SIZE_Y * TILE_SIZE_Y  - (SECTION_LIMIT - 1) * TILE_SIZE_Y / 2  )
							{
								cameraUnit.y = (MAX_MAP_SIZE_Y * TILE_SIZE_Y  -  (SECTION_LIMIT - 1) * TILE_SIZE_Y / 2 - 1) << FRACTIONAL_PART;
							}
							else if ((cameraUnit.y >> FRACTIONAL_PART) < SCREEN_SIZE_Y / 2 )
							{
								cameraUnit.y = (SCREEN_SIZE_Y / 2) << FRACTIONAL_PART;
							}
							game.focusedUnit = (unit_t*) &cameraUnit;
							unit_t *pInterestingUnit = pUnit;
							physics(&pInterestingUnit, 1, &messageToShow, &messageParam);
							drawScene((unit_t*)&cameraUnit, 0);
						}
						else if ((EXBUFF_TO_OEM(key) >= USB_KEY_F1 && EXBUFF_TO_OEM(key) < USB_KEY_F1 + 12) && game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING && game.fireRound == 0 && key != oldKey)
						{
							// determine which weapon slot 0 to 11 we selected.
							messageToShow = MESSAGE_NEW_WEAPON;
							uint8_t weapSlot = EXBUFF_TO_OEM(key) - USB_KEY_F1;
							// if we selected again the same weapon slot we previously selected, then it means that we want to change weapon of the same slot, so add 12 to the currentWeapon
							if (((game.currentWeapon - 1) % 12) == weapSlot)
								game.currentWeapon += 12;
							else
								game.currentWeapon = weapSlot + 1;
							// if the current selected weapon is larger than the number of defined weapons, then reset to the first weapon of the slot
							if (game.currentWeapon >= NUMBER_OF_WEAPONS)
								game.currentWeapon = weapSlot + 1;
							game.configTimeMultiplier = 3;
							game.configBounceMultiplier = 1;
							// choose weapon 
						}
						else if (key == 'z' && oldKey != key)
						{  // switch betwee grenade timings
							game.configTimeMultiplier++;
							if (game.configTimeMultiplier > 5)
								game.configTimeMultiplier = 1;
						}
						else if (key == '+' && oldKey != key)
						{
							game.currentWeapon++;
							if (game.currentWeapon >= NUMBER_OF_WEAPONS)
								game.currentWeapon = 1;
							messageToShow = MESSAGE_NEW_WEAPON;	
							game.configTimeMultiplier = 3;
							game.configBounceMultiplier = 1;							
						}
						else if (key == '-' && oldKey != key)
						{
							if (game.currentWeapon > 1)
								game.currentWeapon--;
							else
								game.currentWeapon = NUMBER_OF_WEAPONS - 1;
							messageToShow = MESSAGE_NEW_WEAPON;
							game.configTimeMultiplier = 3;
							game.configBounceMultiplier = 1;
						}
						else if (key >= '1' && key <= '5' && game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING)
						{
							game.configTimeMultiplier = key - '0';
						}
						else if (key == 'w' || key == 'W')
						{
							int a = game.currentAimAngle ; 
							if (!(weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_HAS_DISCRETE_DIRECTIONS))
							{
								a -= 3;
								if (a < -128)
									a = -128;
							}
							else if (oldKey != 'w' && oldKey != 'W')
							{
									
								if (a > (-256 / 3 + 128))
									a = 0;			
								else if (a > (256 / 3 - 128))
									a =  - 128;						
							}
							game.currentAimAngle = a;
							// aim up
						}
						else if ((key == 'S' || key == 's'))
						{
							// aim down
							int a = game.currentAimAngle;
							if (!(weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_HAS_DISCRETE_DIRECTIONS))
							{
								a += 3;
								if (a > 127)
									a = 127;					
							}
							else if (oldKey != 's' && oldKey != 'S')
							{
								if (a < (256 / 3 - 128))
									a =  0;
								else if (a < (-256 / 3 + 128))
									a = 127;
							}
							game.currentAimAngle = a;
						}
						else if (key == 'A' || key == 'a')
						{
							// try to go left
							game.showCurrentUnitIndicator = 0;
							command = COMMAND_LEFT;
						}
						else if (key == 'D' || key == 'd')
						{
							// try going right
							game.showCurrentUnitIndicator = 0;
							command = COMMAND_RIGHT;
						}
						else if (key == 'F' || key == 'f')
						{
							// jump
							game.showCurrentUnitIndicator = 0;
							command = COMMAND_JUMP;
						}
						else if ((key == 'g' || key == 'G') && game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING && pTeam->inventory[game.currentWeapon] != 0)
						{
							// fire
							game.gameSubState = GAME_SUBSTATE_FIRING;
							game.firingForce = 0;
							pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
							command = COMMAND_FIRE;
							game.automaticRoundsFired = 0;
							if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_POINTER_REQUIRED)
							{
								game.gameSubState = GAME_SUBSTATE_POSITION_SELECTION;							
							}
							else if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_GIRDERS)
							{
								game.xPointer = -1;
								game.yPointer = -1;
								game.gameSubState = GAME_SUBSTATE_PLACE_GIRDERS;
							}
						}
						else if ((key == 'g' || key == 'G') && game.gameSubState == GAME_SUBSTATE_PLAYER_MOVING && pTeam->inventory[game.currentWeapon] == 0)
						{
							messageToShow = MESSAGE_NO_ENOUGH_AMMO;
						}							
						else if ((key == 'r' || key == 'R') && DEBUG_KEYS)
						{
							pUnit->y = 100;
						}
						else if (key == ' ')
						{
							matchInit();
							break;
						}
							
					}
					if (!manualMoveCamera)
					{
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						playerHandler(pUnit, command, 0);
						physics(&pInterestingUnit, 1, &messageToShow, &messageParam);
						if (key)
						{
							drawScene(pUnit, 0);							
						}
						else
						{
							drawScene(NULL, 0);
						}
					}
					break;
				case GAME_SUBSTATE_REPEAT_FIRE:
					{
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(NULL, 0);						
					}
					game.stateTime++;
					if (game.stateTime >= weaponsData[game.currentWeapon].delayBetweenRounds)
					{
						game.stateTime = 0;
						game.gameSubState = GAME_SUBSTATE_FIRING;
					}					
					// if damaged, abort the round
					if (pTeam->damage[pTeam->selectedUnit])
						game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
					break;
				case GAME_SUBSTATE_FOCUS_ON_AIRSTRIKE_DESTINATION:
					game.stateTime++;
					cameraUnit.x = game.xPointer << FRACTIONAL_PART;
					cameraUnit.y = game.yPointer << FRACTIONAL_PART;
					drawScene((unit_t*)&cameraUnit, 0);
					if (game.stateTime >= FOCUS_AIRSTRIKE_TIME)
					{
						game.stateTime = 0;
						game.gameSubState = GAME_SUBSTATE_FIRING;
					}
					break;
				case GAME_SUBSTATE_FIRING:
					{
						if ((key == 'g' || key == 'G') && game.firingForce < 255 && !(weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_FIXED_LAUNCH_FORCE))
						{
							pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
							uint16_t force = game.firingForce;
							force += 4;
							if (force > 255)
								force = 255;
							game.firingForce = force;			
						}
						else
						{	
							if (pTeam->inventory[game.currentWeapon] != -1 && game.automaticRoundsFired == 0)
							{
								// note: we should never get in this state if the number of rounds is 0
								pTeam->inventory[game.currentWeapon]--;
							}							
							if ((weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_SKIP_TURN))
							{
								game.gameSubState = GAME_POST_DAMAGE_SECOND_WAIT;
								game.stateTime = 0;								
							}	
							else if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_SURRENDER)
							{
								game.gameSubState = GAME_SUBSTATE_CONFIRM_SURRENDER;
							}				
							else if (!weaponsData[game.currentWeapon].afterFirePlayerControlTime)
							{															
								pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_FIRING;
								game.gameSubState = GAME_SUBSTATE_PRE_FIRING_ANIMATION;
								game.stateTime = 0;
								if (!(weaponsData[game.currentWeapon].flags & (WEAPON_FLAGS_BLOWTORCH | WEAPON_FLAGS_DRILL)))
								{
									triggerNote(WEAPON_SOUND_CHANNEL, weaponsData[game.currentWeapon].preShootSoundPatch, 40, 255);
								}								
								pUnit->lastFrameTick = 0; // ticks;
								pUnit->frameNumber = 0;							
							}
							else
							{
								fireCurrentWeapon(pUnit, (pUnit->state  & PLAYER_FACING_RIGHT) );
								triggerNote(WEAPON_SOUND_CHANNEL, weaponsData[game.currentWeapon].preShootSoundPatch, 40, 255);								
								pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
								game.gameSubState = GAME_SUBSTATE_PLAYER_MOVING_AFTER_DEPLOYMENT;
								game.stateTime = 0;
							}
						}				
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(NULL, 0);
					}
					break;
				case GAME_SUBSTATE_PRE_FIRING_ANIMATION:
					{
						game.stateTime++;
						if (game.stateTime > weaponsData[game.currentWeapon].fireAnimDelay)
						{
							pUnit->frameNumber++;
							game.stateTime = 0;
						}
						if (pUnit->frameNumber >= weaponsData[game.currentWeapon].preShootFrameNumbers[ (3 * (game.currentAimAngle + 128)) >> 8])
						{
							//fire
							if (!(weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_BLOWTORCH))
								fireCurrentWeapon(pUnit, (pUnit->state  & PLAYER_FACING_RIGHT));
							if (pUnit->frameNumber >= weaponsData[game.currentWeapon].postShootFrameNumbers[ (3 * (game.currentAimAngle + 128)) >> 8])
							{			
								triggerNote(WEAPON_SOUND_CHANNEL, weaponsData[game.currentWeapon].postShootSoundPatch, 40, 255);											
								pUnit->frameNumber = 0;
								pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
								game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
								if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_AUTOMATIC)
								{
									game.automaticRoundsFired++;
									if (weaponsData[game.currentWeapon].numberOfAutomaticRounds > game.automaticRoundsFired)
										game.gameSubState = GAME_SUBSTATE_REPEAT_FIRE;		// wait there until a new round has to be fired...
								}								
							}
							else // do we have post firing animation?
							{
								if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_BLOWTORCH)
								{
									// first, we need to round up player x and y.
									pUnit->x = (pUnit->x >> FRACTIONAL_PART) << FRACTIONAL_PART;
									pUnit->y = (pUnit->y >> FRACTIONAL_PART) << FRACTIONAL_PART;
									// get angle.
									uint8_t shootRight = (pUnit->state  & PLAYER_FACING_RIGHT);
									uint8_t angle = ((3 * (game.currentAimAngle + 128)) >> 8);
									// then we need to create the first two holes...
									// if player is using a blowtorch, we need a special case and we need to fire ALSO this round.
									game.gameSubState = GAME_SUBSTATE_BLOWTORCH_ACTION;
									pUnit->state |= PLAYER_STATE_WALKING | PLAYER_STATE_FIRING;
									camera_unit_t dummyUnit;
									dummyUnit.x = pUnit->x + ((shootRight ? + 2 : -1) << FRACTIONAL_PART);
									dummyUnit.y = pUnit->y + ((angle - 1)  << FRACTIONAL_PART );								
									triggerNote(WEAPON_SOUND_CHANNEL, weaponsData[game.currentWeapon].duringShootSoundPatch, 40, 255);				
									fireCurrentWeapon((unit_t*)&dummyUnit, shootRight);
									dummyUnit.x = pUnit->x + ((shootRight ? + 4 : - 3) << FRACTIONAL_PART);
									dummyUnit.y = pUnit->y + ((2 * (angle - 1))  << FRACTIONAL_PART ); 
									fireCurrentWeapon((unit_t*)&dummyUnit, shootRight);
									game.stateTime = 0; 
									pUnit->lastFrameTick = 0;
								}
								else if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_DRILL)
								{
									game.gameSubState = GAME_SUBSTATE_DRILL_ACTION;
									pUnit->state |= PLAYER_STATE_WALKING | PLAYER_STATE_FIRING;
									camera_unit_t dummyUnit;
									dummyUnit.x = pUnit->x;
									dummyUnit.y = pUnit->y +((((3 * (game.currentAimAngle + 128)) >> 8) - 1)  << FRACTIONAL_PART );
									fireCurrentWeapon((unit_t*)&dummyUnit, (pUnit->state  & PLAYER_FACING_RIGHT));
									triggerNote(WEAPON_SOUND_CHANNEL, weaponsData[game.currentWeapon].duringShootSoundPatch, 40, 255);
									game.stateTime = 0;
									pUnit->lastFrameTick = 0;
								}
								else
								{
									game.gameSubState = GAME_SUBSTATE_POST_FIRING_ANIMATION;
									triggerNote(WEAPON_SOUND_CHANNEL, weaponsData[game.currentWeapon].postShootSoundPatch, 40, 255);
									if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_TELEPORT)
									{   // teleport...
										pUnit->x = game.xPointer << FRACTIONAL_PART;
										pUnit->y = game.yPointer << FRACTIONAL_PART;
									}								
								}
							}
						}
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(NULL, 0);
						// if damaged, abort the round
						if (pTeam->damage[pTeam->selectedUnit])
							game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
					}						
					break;
				case GAME_SUBSTATE_DRILL_ACTION:
					{
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						game.stateTime++;
						physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(NULL, 0);
						// we call first the physics so that the first hole will be already created.
						if (game.stateTime > weaponsData[game.currentWeapon].delayBetweenRounds)
						{
							game.stateTime = 0;
							//fireCurrentWeapon(pUnit);
							fireCurrentWeapon(pUnit, (pUnit->state  & PLAYER_FACING_RIGHT));
						}
						pUnit->lastFrameTick++;
						if (!(pUnit->state & PLAYER_STATE_FIRING) || (pUnit->state & PLAYER_STATE_DEAD) )
						{  // did we die or fall ?
							game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
							stopLoopingFx(WEAPON_SOUND_CHANNEL, 0);						
						}
						else if (pUnit->lastFrameTick >= weaponsData[game.currentWeapon].fireAnimDelay)
						{
							pUnit->frameNumber++;
							if (pUnit->frameNumber >= weaponsData[game.currentWeapon].duringShootFrameNumbers[0])
							{
								game.automaticRoundsFired++;
								if (game.automaticRoundsFired >= weaponsData[game.currentWeapon].numberOfAutomaticRounds)
								{
									game.gameSubState = GAME_SUBSTATE_POST_FIRING_ANIMATION;
									pUnit->vx = 0;
								}
								else
								{
									pUnit->frameNumber = weaponsData[game.currentWeapon].preShootFrameNumbers[0];
								}
							}
							pUnit->lastFrameTick = 0;
						}
						if ((key == 'g' || key == 'G') && !(oldKey == 'g' || oldKey == 'G'))
						{	// stop
							game.gameSubState = GAME_SUBSTATE_POST_FIRING_ANIMATION;
							pUnit->vx = 0;
							game.automaticRoundsFired = weaponsData[game.currentWeapon].numberOfAutomaticRounds;
						}
					}				
					break;
				case GAME_SUBSTATE_BLOWTORCH_ACTION:
					{
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						pUnit->vx = (pUnit->state & PLAYER_FACING_RIGHT) ? PLAYER_BLOWTORCH_SPEED : -PLAYER_BLOWTORCH_SPEED ;
						game.stateTime++;
						physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(NULL, 0);
						// we call first the physics so that the first hole will be already created.
						if (game.stateTime > weaponsData[game.currentWeapon].delayBetweenRounds)
						{
							game.stateTime = 0;
							camera_unit_t dummyUnit;
							// be sure to clear old position
							uint8_t angle = ((3 * (game.currentAimAngle + 128)) >> 8);
							if (angle == 0)
							{
								dummyUnit.x = pUnit->x + (((pUnit->state & PLAYER_FACING_RIGHT) ? + 2 : - 1) << FRACTIONAL_PART);
								dummyUnit.y = pUnit->y + (-1  << FRACTIONAL_PART );
								
							}
							else if (angle == 1)
							{
								dummyUnit.x = pUnit->x + (((pUnit->state & PLAYER_FACING_RIGHT) ? + 3 : - 3) << FRACTIONAL_PART);
								dummyUnit.y = pUnit->y + (0  << FRACTIONAL_PART );
								
							}
							else if (angle == 2)
							{
								dummyUnit.x = pUnit->x + (((pUnit->state & PLAYER_FACING_RIGHT) ? + 2 : - 1) << FRACTIONAL_PART);
								dummyUnit.y = pUnit->y + ( 0 << FRACTIONAL_PART );						
							}
							fireCurrentWeapon((unit_t*)&dummyUnit, (pUnit->state  & PLAYER_FACING_RIGHT));
							// now clear new position
							if (angle == 0)
							{
								dummyUnit.x = pUnit->x + (((pUnit->state & PLAYER_FACING_RIGHT) ? + 4 : - 3) << FRACTIONAL_PART);
								dummyUnit.y = pUnit->y + (-1  << FRACTIONAL_PART );
								
							}
							else if (angle == 1)
							{
								dummyUnit.x = pUnit->x + (((pUnit->state & PLAYER_FACING_RIGHT) ? + 5 : - 5) << FRACTIONAL_PART);
								dummyUnit.y = pUnit->y + (0  << FRACTIONAL_PART );
								
							}
							else if (angle == 2)
							{
								dummyUnit.x = pUnit->x + (((pUnit->state & PLAYER_FACING_RIGHT) ? + 4 : - 3) << FRACTIONAL_PART);
								dummyUnit.y = pUnit->y + (0  << FRACTIONAL_PART );
							}

							fireCurrentWeapon((unit_t*)&dummyUnit, (pUnit->state  & PLAYER_FACING_RIGHT));
						}
						pUnit->lastFrameTick++;
						if (!(pUnit->state & PLAYER_STATE_FIRING))
						{
							game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
							stopLoopingFx(WEAPON_SOUND_CHANNEL, 0);		
						}
						else if (pUnit->lastFrameTick >= weaponsData[game.currentWeapon].fireAnimDelay)
						{
							pUnit->frameNumber++;
							uint8_t direction =  (3 * (game.currentAimAngle + 128)) >> 8;
							if (pUnit->frameNumber >= weaponsData[game.currentWeapon].duringShootFrameNumbers[direction])
							{
								game.automaticRoundsFired++;
								if (game.automaticRoundsFired >= weaponsData[game.currentWeapon].numberOfAutomaticRounds)
								{
									game.gameSubState = GAME_SUBSTATE_POST_FIRING_ANIMATION;
									pUnit->vx = 0;
								}
								else
								{
									pUnit->frameNumber = weaponsData[game.currentWeapon].preShootFrameNumbers[direction];								
								}
							}
							pUnit->lastFrameTick = 0;
						}
						if ((key == 'g' || key == 'G') && !(oldKey == 'g' || oldKey == 'G'))
						{	// stop
							game.gameSubState = GAME_SUBSTATE_POST_FIRING_ANIMATION;
							pUnit->vx = 0;
							game.automaticRoundsFired = weaponsData[game.currentWeapon].numberOfAutomaticRounds;
						}
					}
					break;
				case GAME_SUBSTATE_POST_FIRING_ANIMATION:		// here we deal with the projectile until everything is quiet.	
					// ANIMATE ACTIVE projectiles and players
					{
						unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(NULL, 0);
						if (pUnit->state & PLAYER_STATE_FIRING)
						{
							game.stateTime++;
							if (game.stateTime > weaponsData[game.currentWeapon].fireAnimDelay)
							{
								pUnit->frameNumber++;
								game.stateTime = 0;
							}
							if ((pUnit->frameNumber >= weaponsData[game.currentWeapon].postShootFrameNumbers[ (3 * (game.currentAimAngle + 128)) >> 8]))
							{
								pUnit->frameNumber = 0;
								pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
								game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
								if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_AUTOMATIC)
								{
									game.automaticRoundsFired++;
									if (weaponsData[game.currentWeapon].numberOfAutomaticRounds > game.automaticRoundsFired)
										game.gameSubState = GAME_SUBSTATE_REPEAT_FIRE;		// wait there until a new round has to be fired...
									else
									{
										game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;	
									}
								}								
							}							
						}
						else 
						{
							game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
						}
					}					
					// if damaged, abort the round
					if (pTeam->damage[pTeam->selectedUnit])
					{
						game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;					
					}
					// 	
					if (game.gameSubState == GAME_SUBSTATE_FOLLOW_PHYSICS && (weaponsData[game.currentWeapon].flags & (WEAPON_FLAGS_BLOWTORCH | WEAPON_FLAGS_DRILL)))
						stopLoopingFx(WEAPON_SOUND_CHANNEL, 0);
					break;
				case GAME_SUBSTATE_FOLLOW_PHYSICS:
					{
						uint8_t somethingHappened  = 0;
						// continue last firing animation
						if (pUnit->state & PLAYER_STATE_FIRING)
						{
							game.stateTime++;
							if (game.stateTime > weaponsData[game.currentWeapon].fireAnimDelay)
							{
								pUnit->frameNumber++;
								game.stateTime = 0;
							}
							if ((pUnit->frameNumber >= weaponsData[game.currentWeapon].postShootFrameNumbers[ (3 * (game.currentAimAngle + 128)) >> 8]))
							{
								pUnit->frameNumber = 0;
								pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;	
							}
							else
								somethingHappened = 1;
						}
						unit_t *pInterestingUnit = game.focusedUnit;  // this is to get the most interesting unit in which we have to center our view
						somethingHappened |= physics(&pInterestingUnit, 0, &messageToShow, &messageParam);
						drawScene(pInterestingUnit, 0);
						if (!somethingHappened)
						{
							game.gameSubState = GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_DAMAGE_FIRST_PAUSE;
							game.stateTime = 0;
							if (!(pUnit->state & PLAYER_STATE_DEAD))
								pUnit->state = (pUnit->state  & PLAYER_FACING_RIGHT) | PLAYER_STATE_IDLE;
						}
					}
					break;
				case GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_DAMAGE_FIRST_PAUSE:
					{						
						//
						game.stateTime++;
						if ( game.stateTime > POST_FIRE_CHECK_PLAYER_DAMAGE_FIRST_PAUSE)
						{ 
							game.gameSubState = GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_SHOW_DAMAGES;
							for (int t = 0; t < game.numberOfTeams; t++)
							{
								for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
								{
									if (teams[t].damage[u] && playerUnits[teams[t].firstUnitIndex + u].state != PLAYER_STATE_DEAD)
									{
										teams[t].health[u] -= teams[t].damage[u];
										// TODO: put a logic to decide which unit we should focus to. The one with the largest damage? The one with the smallest remaining health?
										game.focusedUnit = &playerUnits[teams[t].firstUnitIndex + u];										
									}
									else if (playerUnits[teams[t].firstUnitIndex + u].state == PLAYER_STATE_DEAD)
									{
										teams[t].health[u] = 0;
										teams[t].damage[u] = 0;						
									}
								}
							}
							game.stateTime = 0;
						}						
						//unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						//physics(&pInterestingUnit);
						drawScene(NULL, 0);
					}
					break;
				case GAME_SUBSTATE_POST_FIRE_CHECK_PLAYER_SHOW_DAMAGES:
					{
						uint8_t newDeath = 0;
						game.stateTime++;
						//unit_t *pInterestingUnit = pUnit;  // this is to get the most interesting unit in which we have to center our view
						//physics(&pInterestingUnit);
						drawScene(NULL, 0);						
						if (game.stateTime > DAMAGE_SHOW_TIME)
						{
							// check if someone has died.
							for (int t = 0; t < game.numberOfTeams; t++)
							{
								for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
								{
									// remove damage status.
									teams[t].damage[u] = 0;
								}
							}
							// now, check if there are some alive players with 0 health: they, one per time, have to explode. This could cause collateral damage, so after that we will go back to the physics state.
							for (int t = 0; t < game.numberOfTeams && !newDeath; t++)
							{
								for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
								{
									if (teams[t].health[u] == 0 && !(playerUnits[teams[t].firstUnitIndex + u].state & PLAYER_STATE_DEAD))
									{
										// deploy a self destruct projectile
										projectileUnits[0].x = playerUnits[teams[t].firstUnitIndex + u].x;
										projectileUnits[0].y = playerUnits[teams[t].firstUnitIndex + u].y - (PLAYER_HEIGHT/2 << FRACTIONAL_PART);
										projectileUnits[0].vx = 0;
										projectileUnits[0].vy = 0;
										projectileUnits[0].frameFlags = playerUnits[teams[t].firstUnitIndex + u].state & PLAYER_STATE_FALLING_RIGHT ?  0 : SPRITE_FLAGS_FLIP_HORIZONTAL;
										projectileUnits[0].frameNumber = 0;
										projectileUnits[0].lastFrameTick = 0;
										projectileUnits[0].type = PROJECTILE_DUMMY_DEATH;									
										projectileUnits[0].state = PROJECTILE_STATE_FLYING; // ok it is not flying, but it is worth of attention. 
										playerUnits[teams[t].firstUnitIndex + u].state = PLAYER_STATE_DEAD;
										newDeath = 1;
										break;
									}
								}
							}
							if (newDeath)
							{
								game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;								
							}
							else
							{
								game.gameSubState = GAME_POST_DAMAGE_SECOND_WAIT;
								game.stateTime = 0;
							}
						}
					}
					break;
				case GAME_POST_DAMAGE_SECOND_WAIT:
				{
					game.stateTime++;
					if (game.stateTime > POST_FIRE_CHECK_PLAYER_DAMAGE_FIRST_PAUSE)
					{
						// decrease round number if it was not negative. And if the time did not expired...
						game.fireRound++;
						// now let's see if there are at least two teams with alive units. If yes continue the game, otherwise end it
						uint8_t remainingTeams = 0;
						int8_t lastTeam = - 1;
						for (uint8_t t = 0; t < game.numberOfTeams; t++)
						{
							for (uint8_t u = 0; u < game.numberOfUnitsPerTeam; u++)
							{
								if (teams[t].health[u])
								{
									lastTeam = t;
									remainingTeams++;
									break;
								}
							}
							if (remainingTeams > 1)
							break;
						}
						if (remainingTeams > 1)
						{
							if (weaponsData[game.currentWeapon].numberOfManualRounds > game.fireRound && pTeam->health[pTeam->selectedUnit] != 0)
							{
								//game.gameSubState = GAME_SUBSTATE_PLAYER_MOVING;
								game.focusedUnit = pUnit;
								game.gameSubState = GAME_SUBSTATE_FOCUS_TO_ACTIVE_UNIT;
								game.stateTime = 0;
							}
							else
							{
								// before shifting turn, let's see if we can drop a bonus
								uint16_t r = rand();
								if (r < (BONUS_PROBABILITY_BIN) && dropBonus())
								{
									game.gameSubState = GAME_SUBSTATE_FOLLOW_PHYSICS;
								}
								else
								{
									int team = getNextTeam();
									game.currentTeam = team;
									game.focusedUnit = &playerUnits[teams[team].firstUnitIndex + teams[team].selectedUnit];
									game.gameSubState = GAME_SUBSTATE_FOCUS_TO_ACTIVE_UNIT;
									game.stateTime = 0;
									game.windSpeed  = rand();
								}
							}
						}
						else
						{
							game.numberOfRoundsPlayed++;
							// update number of round won
							if (lastTeam != -1)
							{
								teams[lastTeam].wonRounds++;
							}
							// is there the opportunity for any other teams to win the match? Let's find the first and the second and let's see if the difference is smaller than the number of rounds 
							int8_t numberOfRemainingRounds = game.numberOfRoundsPerMatch - game.numberOfRoundsPlayed;
							uint8_t maxWon = teams[0].wonRounds;
							uint8_t bestTeam = 0;
							for (uint8_t t = 1; t < game.numberOfTeams; t++)
							{
								if (teams[t].wonRounds > maxWon)
								{
									bestTeam = t;
									maxWon = teams[t].wonRounds;
								}
							}
							// now let's see the second
							uint8_t secondTeamCanStillWinOrDraw = 0;
							for (uint8_t t = 0; t < game.numberOfTeams; t++)
							{
								if (t == bestTeam)
									continue;
								if (teams[t].wonRounds + numberOfRemainingRounds >= maxWon)
								{
									secondTeamCanStillWinOrDraw = 1;
								}
							}
							if (numberOfRemainingRounds > 0 && secondTeamCanStillWinOrDraw)
							{
								game.gameSubState = GAME_SUBSSTATE_END_OF_ROUND;
								messageToShow = MESSAGE_ROUND_WON;
								messageParam = lastTeam + 1;

							}
							else
							{ // no round available. However, if secondTeamCanStillWinOrDraw == 1, then there are at least two teams with the same points.
								if (secondTeamCanStillWinOrDraw)
								{
									messageParam = 0;
								} 
								else
								{
									messageParam = bestTeam + 1;	
								}
								game.gameSubState = GAME_SUBSTATE_END_OF_MATCH;
								messageToShow = MESSAGE_MATCH_WON;						
							}
							game.stateTime = 0;
						}																					
					}
					drawScene(NULL, 0);	
				}								
				break;
				case GAME_SUBSSTATE_END_OF_ROUND:
					game.stateTime++;
					if (game.stateTime > GAME_END_OF_MATCH_SHOW_TIME)
					{
						game.gameSubState = GAME_SUBSSTATE_ROUND_INIT;
					}
					drawScene(NULL, 0);
					break;				
				case GAME_SUBSTATE_END_OF_MATCH:
					game.stateTime++;
					if (game.stateTime > GAME_END_OF_MATCH_SHOW_TIME)
					{
							menuInit();
							game.gameSubState = GAME_SUBSTATE_MENU_MATCH_SUMMARY;
							game.gameState = GAME_STATE_MENU;
							game.menuPage = MENU_PAGE_MATCH_SUMMARY;
							game.selectedMenuItem = 0;
							return;			// this is the only case of premature return...			
					}
					drawScene(NULL, 0);
				break;
			}
			// fixed section handler and text state machine
			char tmpText[MAX_FIXED_SECTION_TEXT + 1];
			// check for dead units
			uint8_t deathVal = 1;
			uint8_t color = 7;			
			for (int t = 0; t < game.numberOfTeams; t++)
			{
				if (messageToShow & MESSAGE_DEAD_UNIT)
					break;
				for (int u = 0; u < game.numberOfUnitsPerTeam; u++)
				{
					if ((playerUnits[teams[t].firstUnitIndex + u].state & PLAYER_STATE_DEAD) && !(game.deadMask & deathVal))
					{
						game.deadMask |= deathVal;
						uint8_t i;
						for (i = 0; teamNames[t][i] != 0; i++ )
							tmpText[i] = teamNames[t][i];
						// notify this death
						tmpText[i++] = '-';
						tmpText[i++] = '1' + u;
						const uint8_t isDeadString []= " is dead";
						const uint8_t * p = isDeadString;
						for (; *p != 0; p++, i++)
						{
							tmpText[i] = *p;
						}
						tmpText[i] = '\0';
						messageToShow |= MESSAGE_DEAD_UNIT;
						color = teamColors[t + 1];
						break;
					}
				}
				deathVal = deathVal << 1;
			}
			const char *pText = "";
			static uint8_t messageToShowCounter = 0;
			if (messageToShow)
			{
				if (messageToShow & MESSAGE_GET_THEM)
				{
					const uint8_t killem [] = {"KILL'EM "};
					uint8_t i;
					for (i = 0; killem[i] != 0; i++ )
						tmpText[i] = killem[i];
					for (uint8_t n = 0; teamNames[game.currentTeam][n] != 0; n++, i++ )
						tmpText[i] = teamNames[game.currentTeam][n];
					tmpText[i++] = '-';
					tmpText[i++] = '1' + teams[game.currentTeam].selectedUnit;
					tmpText[i++] = 0;
					pText = tmpText;		
					messageToShowCounter = 2 * FPS;
					color = teamColors[game.currentTeam + 1];
				}
				else if (messageToShow & MESSAGE_MATCH_WON)
				{
					messageToShowCounter = 4 * FPS;
					pText = matchWonStrings[messageParam];		
					color = teamColors[messageParam];					
				}
				else if (messageToShow & MESSAGE_ROUND_WON)
				{
					messageToShowCounter = 4 * FPS;
					pText = roundWonStrings[messageParam];
					color = teamColors[messageParam];
				}
				else if (messageToShow & MESSAGE_DEAD_UNIT)
				{
					messageToShowCounter = 2 * FPS;
					pText = tmpText;
				}
				else if (messageToShow & MESSAGE_NEW_WEAPON)
				{
					uint8_t  i;
					for (i = 0; i < MAX_FIXED_SECTION_TEXT; i++)
					{
						tmpText [i] = weaponsData[game.currentWeapon].name[i];					
						if (weaponsData[game.currentWeapon].name[i] == 0)
							break;
					}
					if (pTeam->inventory[game.currentWeapon] != -1 && pTeam->inventory[game.currentWeapon] <= 9 && i <= (MAX_FIXED_SECTION_TEXT - 2))  // if there are no infinite ammo, then print them	 
					{
						tmpText[i++] = '-';
						tmpText[i++] = '0' + pTeam->inventory[game.currentWeapon];
						tmpText[i++] = 0;
						if (pTeam->inventory[game.currentWeapon] == 0)
							color = 10;
					}					
					messageToShowCounter = 3 * FPS;	
					pText = tmpText;	
				}
				else if (messageToShow & MESSAGE_NO_ENOUGH_AMMO)
				{
					color = 10;
					pText = "NO ENOUGH AMMO!";
					messageToShowCounter = 3 * FPS;
				}
				else if (messageToShow & MESSAGE_CONFIRM_SURRENDER)
				{
					color = 10;
					pText = "SURRENDER? Y/N?";					
				}
				else if (messageToShow & MESSAGE_TIMEUP)
				{
					color = 9;
					pText = "TIME IS UP!";
					messageToShowCounter = 2 * FPS;
				}		
				else if (messageToShow & MESSAGE_NO_SPACE_FOR_GIRDER)
				{
					color = 9;
					pText = "NO SPACE HERE!";
					messageToShowCounter = 2 * FPS;					
				}		
				else if (messageToShow & MESSAGE_BONUS)
				{
					color = 14;
					pText = "PRESENT FOR YOU!";
					messageToShowCounter = 2 * FPS;
				}				
				else if (messageToShow & MESSAGE_BONUS_WEAPON)
				{
					int i;
					for (i = 0; i < MAX_FIXED_SECTION_TEXT; i++)
					{
						tmpText [i] = weaponsData[messageParam].name[i];
						if (weaponsData[messageParam].name[i] == 0)
							break;
					}
					if (pTeam->inventory[messageParam] <= 9 && i <= (MAX_FIXED_SECTION_TEXT - 2))  // if there are no infinite ammo, then print them
					{
						tmpText[i++] = '+';
						tmpText[i++] = '1';
						tmpText[i++] = 0;
					}
					messageToShowCounter = 3 * FPS;
					pText = tmpText;
				}
				else if (messageToShow & MESSAGE_BONUS_HEALTH)
				{
					color = 14;
					pText = "HEAL YOURSELF!";
					messageToShowCounter = 2 * FPS;
				}				
				updateFixedSection(1, pText, color, 0);
			}
			else
			{
				if (messageToShowCounter)
				{
					messageToShowCounter--;				
					updateFixedSection(0, NULL, color, 0);
				}
				else
				{
	#if DEBUG_TIME
		static uint16_t longest = 0;
		if (gLine > longest)
			longest = gLine;
		reasonablyFastUint16ToArray(longest,(uint8_t*) tmpText);
		//reasonablyFastUint16ToArray(game.gameSubState,tmpText);
		//reasonablyFastUint16ToArray(pUnit->state * 100 + game.gameSubState, (uint8_t *) tmpText);
		for (int i = 0; i < 5; i++)
			tmpText[i] += '0';
		tmpText[5] = 0;
		updateFixedSection(1, tmpText, 6, 0);	
	#else
					static uint8_t oldShownTime = 0;
					if (oldShownTime != game.turnTime)
					{
						const uint8_t secondsLeft [] = "Seconds left: ";
						for (int i = 0; i < sizeof(secondsLeft) - 1; i++)
						{
							tmpText[i] = secondsLeft[i];
						}
						tmpText[sizeof(secondsLeft) - 1] = '0' + game.turnTime / 10;
						tmpText[sizeof(secondsLeft)] = '0' + game.turnTime % 10;						
						tmpText[sizeof(secondsLeft) + 1] = 0;
	
						updateFixedSection(1, tmpText, 6, 0);	
					}
					else
						updateFixedSection(0, NULL, 6, 0);
					oldShownTime = game.turnTime;

	#endif
				}
				// print turn time.
				
			}
		}
		break;
	}
	oldKey = key;
}

inline void initFramePalettes(int remapVerticalOffset, const uint16_t *pCheckpointData)
{
	uint8_t * pCurrentLevelColorToChangeIndex = (uint8_t*) ((customLevelData_t*)levels[currentLevel].customData)->colorToChangeIndex;
	uint8_t * pCurrentLevelNewColorTable = (uint8_t*)((customLevelData_t*)levels[currentLevel].customData)->newColorTable;
	uint8_t * pCurrentLevelRowPaletteIndexes = (uint8_t *) ((customLevelData_t*)levels[currentLevel].customData)->rowPaletteIndexes;
	int16_t * pCurrentLevelPaletteRowRemap	= (int16_t*) ((customLevelData_t*)levels[currentLevel].customData)->paletteRowRemap;

	// first we set the palette according to the checkpoint
	int checkpoint = (remapVerticalOffset >>  CHECKPOINT_PERIOD_LOG2);
	pCheckpointData += checkpoint * CHANGED_PALETTE_ENTRIES * CHECKPOINT_PALETTES;
	uint16_t *pPalette = ((uint16_t*) videoData.pPalette);
	for (int paletteN = 0; paletteN < CHECKPOINT_PALETTES; paletteN++)
	{
		for (int paletteEntry = 0; paletteEntry < CHANGED_PALETTE_ENTRIES; paletteEntry++)
		{
			uint16_t value = (*pCheckpointData++) & ~( 1 <<10);
			for (int i = 0; i < 16; i++)
			{
				pPalette[paletteEntry * 2 + 32 * i] = value;
				pPalette[paletteEntry * 32 + 2 * i + 1] = value;
			}
		}
		pPalette += 512;
	}
	// recalculate palettes in the remainder of the rows before actual line
	// for instance if the checkpoint period is 16, and we are at row 19, we have already performed checkpoint for line 16, but we need now to calculare lines 16, 17, 18 (19 will be done in the frame)
	int startRow = remapVerticalOffset & ~( ( 1 << CHECKPOINT_PERIOD_LOG2) - 1);
	// restore pPalette address
	for (int row = startRow; row < remapVerticalOffset; row++)
	{
		pPalette = ((uint16_t*) videoData.pPalette) + 512 * (pCurrentLevelRowPaletteIndexes[row] >> 4);
		uint16_t value = ((pCurrentLevelNewColorTable[row] * 1025) | 512) & ~(1 << 10);   // |512 is to set the correct gpio value. *1025 is to set the correct color signals.
		// NOTE: this works only when bytes are not packed.
		int paletteEntry = pCurrentLevelColorToChangeIndex[row];
		for (int i = 0; i < 16; i++)
		{
			pPalette[paletteEntry * 2 + 32 * i] = value;
			pPalette[paletteEntry * 32 + 2 * i + 1] = value;
		}
	} 
	
	// put the correct pointers
	videoData.pNewColorChangeIndexTable = &pCurrentLevelColorToChangeIndex[remapVerticalOffset];
	videoData.pNewColorChangeTable = &pCurrentLevelNewColorTable[remapVerticalOffset];
	videoData.pPaletteIdx = &pCurrentLevelRowPaletteIndexes[remapVerticalOffset];
	videoData.pPaletteRemappingRowOffsets = &pCurrentLevelPaletteRowRemap[remapVerticalOffset * 2 + WATER_HEIGHT * 2 ];
	// if explosion is in progress
	if (game.screenShake > 0 && game.screenShake < 5)
	{
		uint16_t *pPalette = ((uint16_t*) videoData.pPalette);
		for (int paletteN = 0; paletteN < CHECKPOINT_PALETTES; paletteN++)
		{
			for (int paletteEntry = 2; paletteEntry < CHANGED_PALETTE_ENTRIES; paletteEntry++)
			{
				uint16_t value = ((customLevelData_t *) levels[currentLevel].customData)->explosionColors[paletteEntry];
				for (int i = 0; i < 16; i++)
				{
					pPalette[paletteEntry * 2 + 32 * i] = value;
					pPalette[paletteEntry * 32 + 2 * i + 1] = value;
				}
			}
			pPalette += 512;
		}		
	}
}
void setViewPosition(int *px, int * py, uint8_t forceRedraw)
{
	int defaultPx = 0, defaultPy = 0;
	if (px == 0)
		px = &defaultPx;
	if (py == 0)
		py = &defaultPy;
	// this function merely changes the y and x screen position. 
	// First: clip the requested position.
	if (*py > (MAX_MAP_SIZE_Y * TILE_SIZE_Y - SCREEN_SIZE_Y - 1 - (SECTION_LIMIT * TILE_SIZE_Y - SCREEN_SIZE_Y)))
		*py = MAX_MAP_SIZE_Y * TILE_SIZE_Y - SCREEN_SIZE_Y - 1 - (SECTION_LIMIT * TILE_SIZE_Y - SCREEN_SIZE_Y);
	if (*px > (MAX_MAP_SIZE_X * TILE_SIZE_X - SCREEN_SIZE_X - 1))
		*px = (MAX_MAP_SIZE_X * TILE_SIZE_X - SCREEN_SIZE_X - 1);
	if (*px < 0)
		*px = 0;
	if (*py < 0)
		*py = 0;	
	if (!forceRedraw)
	{
		// Now, instead of abruptly going to the target position, we go slowly.
		*px = game.lastViewX + FIXED_MULFRACT(*px - game.lastViewX, 0x1000);
		*py = game.lastViewY + FIXED_MULFRACT(*py - game.lastViewY, 0x1000) + game.yExplosionShakeOffset;	
	}
	game.lastViewX = *px;  
	game.lastViewY = *py;
	drawRamMap(*px, *py, forceRedraw);			
	// now, based on y, we need to change the background colors.
	initFramePalettes(*py, ((customLevelData_t*)levels[currentLevel].customData)->checkpointData);
}
inline void setRamMapTileFromTileCoordinates(uint16_t xTile,  uint16_t yTile, uint16_t tileNumber)
{
	int n = xTile + yTile * MAX_MAP_SIZE_X;	
	gameRamMap.loTileNumber[n] = tileNumber;
	if (tileNumber & (1 << 8))
		gameRamMap.highBitTileNumber[n >> 3] |= 1 << (n & 7);
	else
		gameRamMap.highBitTileNumber[n >> 3] &= ~(1 << (n & 7));	
	// Update VRAM if necessary. This allows to avoid force redraw
	if ((game.lastViewX <= xTile * TILE_SIZE_X) && (xTile*TILE_SIZE_X <= (game.lastViewX + SCREEN_SIZE_X)) && (game.lastViewY <= yTile * 8) && (yTile * 8 <= (game.lastViewY + SCREEN_SIZE_Y)))
	{
		int vXt = xTile - (game.lastViewX >> LOG2_TILE_SIZE_X);
		int vYt = yTile - (game.lastViewY >> LOG2_TILE_SIZE_Y);
		vram[VRAMX * vYt + vXt] = (uint32_t) &tiles[tileNumber];
	}
}
void drawRamMap(uint16_t xOffset, uint16_t yOffset, uint8_t forceRedraw)
{
	static uint16_t m_old_xStart = 0xFFFF;
	static uint16_t m_old_yStart = 0xFFFF;
	// first, restore the background tiles. The sprites will be redrawn later.
	restoreBackgroundTiles();
	// then copy the tiles required.
	// we just need to copy part of the map in vram.
	int lastXtile =  VRAMX, lastYtile = VRAMY;		// to the MCU from accessing undefined areas
	if (xOffset > (MAX_MAP_SIZE_X * TILE_SIZE_X) - SCREEN_SIZE_X)
	{
		xOffset = MAX_MAP_SIZE_X * TILE_SIZE_X - SCREEN_SIZE_X;
	}
	#if USE_SECTION == BOTTOM_FIXED_SECTION
		if (yOffset >= ( MAX_MAP_SIZE_Y * TILE_SIZE_Y ) - SCREEN_SIZE_Y - (SECTION_LIMIT * TILE_SIZE_Y - SCREEN_SIZE_Y))
		{
			yOffset =  MAX_MAP_SIZE_Y * TILE_SIZE_Y  - SCREEN_SIZE_Y - (SECTION_LIMIT * TILE_SIZE_Y - SCREEN_SIZE_Y);
		}
	#else
		if (yOffset >= (  MAX_MAP_SIZE_Y * TILE_SIZE_Y  ) - SCREEN_SIZE_Y)
		{
			yOffset =  MAX_MAP_SIZE_Y * TILE_SIZE_Y  - SCREEN_SIZE_Y;
		}
	#endif
	uint16_t xStart = xOffset >> LOG2_TILE_SIZE_X;
	uint16_t yStart = yOffset >> LOG2_TILE_SIZE_Y;
	if (m_old_xStart != xStart || m_old_yStart != yStart || forceRedraw)
	{
		for (int y = 0; y < lastYtile; y++)
		{
			for (int x = 0; x < lastXtile; x++)
			{
				*((uint16_t*)(&vram[x + y * VRAMX])) = (uint32_t) &tiles[getRamMapTileFromTileCoordinates(x + xStart, y + yStart) & TILEMASK][0];
			}
		}
		m_old_xStart = xStart;
		m_old_yStart = yStart;
	}
	videoData.xScroll = xOffset & (TILE_SIZE_X - 1);
	videoData.yScroll = yOffset & (TILE_SIZE_Y - 1);
	videoData.spriteTilesRemoved = 1;
	
}

void setDestroyedTileIndexUsedState(int index, int state)
{
	int bitPos = index & 31;
	int wordPos = index >> 5;
	if (state)
		usedDestroyedTiles[wordPos] |= (1 << bitPos);
	else
		usedDestroyedTiles[wordPos] &= ~(1 << bitPos);
}
int getFirstAvailableDestroyedTile()
{
	for (int i = 0; i < sizeof(usedDestroyedTiles) / 4; i++)
	{
		if (usedDestroyedTiles[i] != 0xFFFFFFFF)
		{
			for (int j = 0; j < 32; j++)
			{
				if ((usedDestroyedTiles[i] & (1 << j )) == 0)
					return j + i * 32;
			}
		}
	}
	return -1;
}
uint32_t destroyPlayground(int32_t xSpr, int32_t ySpr, uint32_t nf)
{
	/* The algorithm is similar to the sprite drawing. Instead of having a sprite frame, with actual data to be drawn,
			we have the mask, which will be AND-'d with the actual tile data.
					As in the sprite drawing routine, we need to determine the rectangle to be copied into tiles.
					Initial calculations are performed with the unshifted frame.
					The parameters of the mask are expressed in pixels. Note that, due to byte-alignemnt, if the width is odd, 
					the actual number of bytes per mask line is (width + 1) / 2 (i.e. the increment is (width + (width & 1)) >> 1 )
					//
					Note! This is based on the algorithm with per tile x scroll enabled!

	*/
 	nf = nf << 1;
    // initial calculations are performed with the unshifted frame.
	frame_t *pFrame = (frame_t*) &frameData[nf];
    int32_t wSpr = pFrame->w;
    int32_t hSpr = pFrame->h;
	xSpr -= wSpr >> 1;
	ySpr -= hSpr >> 1;
    //
    int32_t maxYsprite;
    int32_t minYsprite = ySpr;
	//boundary check
	if (ySpr >= MAX_MAP_SIZE_Y * TILE_SIZE_Y)
	{
		return 0;   // frame outide the play area: actually this should never happen, as when the projectile is outside of the play area, it gets already deleted.
	}
	else if (ySpr < 0)
	{
		minYsprite = 0;
	}
	maxYsprite = (ySpr + hSpr);
	if (maxYsprite < 0)
	{
		return 0;		// frame outside the play area. This actually will never happen, as if something is outside the play area, it gets deleted.
	}
	else if (maxYsprite > MAX_MAP_SIZE_Y * TILE_SIZE_Y)
	{
		// let's calculate how much the sprite is offscreen
		maxYsprite = MAX_MAP_SIZE_Y * TILE_SIZE_Y ;
	}
	const uint8_t *p =  0;				// these are initialized just to avoid warnings
	int verticalIncrement = 0;
	// if independent PER_TILE_X_SCROLL is not used, we can now calculate, depending on the orientation, the increments we need to add to each pointer, as well as the pointer start location.
	int32_t tileYstart = minYsprite >> LOG2_TILE_SIZE_Y;
	int32_t tileYstop = (maxYsprite - 1) >> LOG2_TILE_SIZE_Y;
	uint32_t *destroyedTileStart = tiles[levels[currentLevel].numberOfTilesToCopyInRam];

	int32_t maxXsprite2;
	//
	pFrame = (frame_t*) &frameData[nf + (xSpr & 1)];
	wSpr = pFrame->w;
	wSpr += wSpr & 1;
	xSpr = xSpr >> 1;
	wSpr = wSpr >> 1;
	maxXsprite2 = (xSpr + wSpr);
	if (maxXsprite2 < 0)
	{
		return 0;		// sprite not visible in this tile number
	}
	else if (maxXsprite2 > MAX_MAP_SIZE_X * TILE_SIZE_X / 2)
	{
		maxXsprite2 = MAX_MAP_SIZE_X * TILE_SIZE_X / 2;
	}
	int32_t minXsprite = xSpr;
	if (xSpr >= MAX_MAP_SIZE_X * TILE_SIZE_X / 2 )
	{
		return 0;		// sprite not visible in this tile number
	}
	else if (xSpr < 0)
	{
		minXsprite = 0;
	}
	p = &pFrame->pData[0]; // point to the top left pixel.
	verticalIncrement  = wSpr ;
	int32_t tileXstart = minXsprite >> (LOG2_TILE_SIZE_X - 1);
	int32_t tileXstop = (maxXsprite2 - 1) >> (LOG2_TILE_SIZE_X - 1);
	for (int yt = tileYstart; yt <= tileYstop; yt++)
	{
		//
		int32_t minY = (yt * TILE_SIZE_Y  < minYsprite) ? minYsprite : (yt * TILE_SIZE_Y);
		int32_t maxY = ((yt + 1) * TILE_SIZE_Y ) > maxYsprite ? maxYsprite : ((yt + 1) * TILE_SIZE_Y);
		int32_t offsetY =   minY  - yt * TILE_SIZE_Y;

		for (int xt = tileXstart; xt <= tileXstop ; xt++)
		{
			int32_t minX = (xt * (TILE_SIZE_X / 2) < minXsprite) ? minXsprite : (xt * (TILE_SIZE_X / 2));	
			int32_t deltaX = (((xt + 1) * (TILE_SIZE_X / 2)) > maxXsprite2 ? (maxXsprite2 - minX)  : ((xt + 1) * (TILE_SIZE_X / 2)) - minX);// >> 1;
			int32_t offsetX =   minX - xt * (TILE_SIZE_X / 2);
			// now we are in one tile. Let's copy the sprite there the data (if any)
			int x, y;
			// let us first see if we are already writing on a sprite tile or if we need to reserve it first
			uint8_t *addr = (uint8_t*) &tiles[getRamMapTileFromTileCoordinates(xt, yt)][0];//((vram[VRAMX * yt + xt]) | HIGH_ADDRESS);
			int32_t vi = verticalIncrement  - deltaX ; // MODDATO >> 1
			/*
				Now we need to know if we have to actually do something.
			*/
			if (addr == (uint8_t*) &tiles[0])
				continue;			// the tile 0 is already completely transparent. Nothing to do!
			// Note! We just check if the tile has a higher address than the last tile, as sprites are going to be drawn later 
			uint8_t tileFound = ( ((uint32_t)addr) >= ((uint32_t) destroyedTileStart )) ;
			#if 1

			if (!tileFound )
			{
				int destroyedTileNumber = getFirstAvailableDestroyedTile();
				if (destroyedTileNumber != -1)
				{
					uint8_t *pp = (uint8_t*) &p[(minY - ySpr) * verticalIncrement + (((minX - xSpr)))];
					for (y = minY; y < maxY; y++)
					{
						// note it is ok here to use this to determine the pixel, regardless the horizontal shift. After all pixels are drawn 2 at a time
						for (x = 0; x < deltaX; x ++)
						{
							if (*pp != 0xFF)
							{
								uint32_t *s = (uint32_t *) addr;
								uint32_t *d = destroyedTileStart + 8 * destroyedTileNumber; // MODDATO 8
								setRamMapTileFromTileCoordinates(xt, yt, destroyedTileNumber + levels[currentLevel].numberOfTilesToCopyInRam);
								// copy old tile to temp sprite tile. 
								for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 8; i++) // MODDATO / 8
								{
									*d++= *s++;
								}
								setDestroyedTileIndexUsedState(destroyedTileNumber, 1);
								usedDestroyedTileNumber++;
								tileFound = 1;
								break;
							}
							pp ++;
						}
						if (tileFound)
							break;
						pp +=  vi;//verticalIncrement - (maxX - minX) * horizontalIncrement;
					}
				}
				else
				{
					// we have no more room for tiles! let's use entire blocks... 
					// TODO: we can try and remove those tiles which have the largest number of removed pixels.
					setRamMapTileFromTileCoordinates(xt, yt, 0);	
				}
			}
			// now, either the tile has been allocated, or it was already allocated
			if (tileFound)
			{
				uint8_t *pp = (uint8_t*) &p[(minY - ySpr) * verticalIncrement + (((minX - xSpr)) )];
				uint8_t *dest =    ((uint8_t*)( &tiles[getRamMapTileFromTileCoordinates(xt, yt)][0]) + ((offsetY * (TILE_SIZE_X / 2) + offsetX) )) ;
				int32_t hdi = (TILE_SIZE_X / 2) - deltaX ;
				for (int yy = minY; yy < maxY; yy++)
				{
					for (int xx = 0; xx < deltaX; xx++)
					{
						*dest &= *pp;
						dest++;
						pp++;
					}
					dest += hdi;
					pp += verticalIncrement - deltaX;
				}
				// check how many pixels are cleared in this tile
				int pixelsCleared = 0;
				uint32_t *dest32 =   (uint32_t*)  &tiles[getRamMapTileFromTileCoordinates(xt, yt)][0];
				for (int i = 0; i < 8; i++)
				{
					uint32_t pix = *dest32;
					if (pix == 0)
						pixelsCleared += 8;
					else 
					{
						//
						for (int j = 0; j < 4; j++)
						{
							if (0 == (pix & 0xFF))
								pixelsCleared+= 2;
							else if ((pix & 0xF0) == 0 ||  ((pix & 0x0F) == 0))
								pixelsCleared++;
							pix >>= 8;
						}
					}
					dest32++;	
				}
				// if the tile has too many pixels removed, then let's free it
				if (pixelsCleared > MIN_PIXEL_TO_CLEAR)
				{
					int tileIndex =  getRamMapTileFromTileCoordinates(xt, yt) - levels[currentLevel].numberOfTilesToCopyInRam;
					setDestroyedTileIndexUsedState(tileIndex, 0); 
					setRamMapTileFromTileCoordinates(xt, yt, 0);	
					usedDestroyedTileNumber--;		
				}
				// warning: gestire pixel cleared!
			}
			#endif
		}	
	}
	return 1;
}
void drawMenuPage(uint8_t menuPage, uint8_t selectedItem)
{
	const menuPageLine_t *pMenuPageLines = pMenuPages[menuPage];
	if (pMenuPageLines == NULL )
	{
		return; // nothing to do
	}
	uint8_t nRow = 0;
	int8_t nItem = -1;
	while (pMenuPageLines[nRow].text != NULL)
	{
		if (pMenuPageLines[nRow].flags & TEXT_FLAGS_IS_ITEM) 
			nItem++;
		printLine(pMenuPageLines[nRow].text, pMenuPageLines[nRow].col, pMenuPageLines[nRow].row, pMenuPageLines[nRow].flags | (nItem == selectedItem ? TEXT_FLAGS_COLOR_HIGHLIGHT : 0));
		nRow++;
	}
}
uint8_t goToMenuPage(uint8_t menuPage)
{
	game.menuPage = menuPage;
	game.selectedMenuItem = 0;
	drawMenuPage(menuPage, 0);
	clearMenuPage(menuPage);
	if (menuPage == MENU_PAGE_MAIN)
	{
		// print choices
		return GAME_SUBSTATE_MENU_GO_LEFT;
	}
	else
	{
		// print choices
		return GAME_SUBSTATE_MENU_GO_RIGHT;
	}		
}
void printLine (const char * text, int8_t col, uint8_t row, uint8_t flags)
{
	uint16_t charTileSet = levels[0].numberOfTilesToCopyInRam - ' ' + ('z' - ' ') * (0 != (flags & TEXT_FLAGS_COLOR_HIGHLIGHT));
	if (flags & TEXT_FLAGS_ALIGN_RIGHT)
		col -= strlen(text);
	else if (flags & TEXT_FLAGS_ALIGN_CENTER)
		col = col - strlen(text) / 2;
	if (col < 0)
		col = 0;
	const uint8_t *pText = (uint8_t*) text;
	for (uint8_t c = col; c < MAX_MAP_SIZE_X && *pText != 0; c++)
		setRamMapTileFromTileCoordinates(c, row, charTileSet + *pText++);		
}
void menuInit(void)
{
	initFixedSection(0);
	changeLevelEx(0, MAX_TILES - levels[0].numberOfTilesToCopyInRam - MAX_TEMP_SPRITE_TILES); // "reserved tiles" for ASCII characters
	copyMapToRam(0, 0, 0, 0, TITLE_MENU_Y_OFFSET, 40, 8);
	uint16_t charNumber = levels[0].numberOfTilesToCopyInRam ;
	for (int col =  FIRST_TEXT_COLOR; col <= SECOND_TEXT_COLOR;  col += SECOND_TEXT_COLOR - FIRST_TEXT_COLOR)
	{
		for (int ch = ' '; ch < 'z'; ch++ )
			putCharInTile(NULL, ch, col, 0, 0, (uint8_t*) &tiles[charNumber++][0], 0);
	}
}
void gameInit(void)
{
	menuInit();
	memset(&game, 0, sizeof(game));
	game.currentWeapon = 1;
	game.numberOfRoundsPerMatch = 1;
	game.focusedUnit = &playerUnits[0];
	game.gameState =   GAME_STATE_TITLE_SCREEN; //*/ GAME_STATE_PLAYING;
	game.gameSubState = GAME_SUBSSTATE_ROUND_INIT;
	game.numberOfTeams = 4;
	game.numberOfUnitsPerTeam = 2;
	memset(&teams, 0, sizeof(teams));
	memset(&playerUnits, 0, sizeof(teams));
	memset(&projectileUnits, 0, sizeof(projectileUnits));
	memset(&staticUnits, 0, sizeof(staticUnits));
	drawRamMap(0, 0, 1);
	printLine("Press the any-key.", 20, 18, TEXT_FLAGS_ALIGN_CENTER | TEXT_FLAGS_COLOR_HIGHLIGHT);
	game.gameSubState = 0;
}
/* fixed section functions.
   Note, there are many ways to deal with fixed sections. Each game can do whatever the developer wants.
   In this game we start from an "empty" map, and then we update ony the critical parts.
   There are three kinds of tiles. 
   a) The tiles that reside in ram, and they are always available.
   b) The tiles that reside in rom, so if they need to be used, they have to be copied in ram.
   c) Characters. Since there are few digits/characters to be displayed, these must be overwritten everytime. 
*/
void updateFixedSection(uint8_t updateText, const char *cText, uint8_t textColor, uint8_t forceUpdate)
{
	static uint16_t force = 0;
	static int8_t healths[4] = {0, 0, 0, 0};
	static int8_t windSpeed = 0;			
	uint8_t *text = (uint8_t*) cText;
	if (forceUpdate)
	{
		windSpeed = 0 ;
		force = 0;
		memset(healths, 0, sizeof(healths));
	}
	if (updateText)
	{
		uint8_t i;
		int8_t center = 0;
		center = (SCREEN_SIZE_X/TILE_SIZE_X - strlen(cText)) / 2;		// hopefully the compiler translates this into >>1
		if (center < 0)
			center = 0;
		for (i = 0; i < (SCREEN_SIZE_X/TILE_SIZE_X / 4); i++)
		{
			// clear 4 tiles per iteration.
			((uint32_t*)&fixedSectionMap[0])[i] = 0;			
		}		
		for (i = 0; i < MAX_FIXED_SECTION_TEXT && text[i] != 0; i++)
		{
			// print to tile
			putCharInTile(NULL, text[i], textColor, 0x5, 0 , (uint8_t *) fixedSectionTiles[i + FIXED_SECTION_TILE_TEXT_OFFSET], 1);
			// since offset is 1, we need to put the background in the text.
			fixedSectionTiles[i + FIXED_SECTION_TILE_TEXT_OFFSET][0] = BLACK_BACKGROUND;
			fixedSectionMap[i + center] = i + FIXED_SECTION_TILE_TEXT_OFFSET; 
		}
	}
	static uint16_t weapon = -1;
	uint16_t currentWeapon = game.currentWeapon | (game.configTimeMultiplier << 8 | game.configBounceMultiplier << 12);
	// Update weaopn
	if (weapon != currentWeapon || forceUpdate)
	{
		// copy weapon.
		weapon = currentWeapon;
		if (weaponsData[game.currentWeapon].flags & WEAPON_FLAGS_GRENADE_TYPE)
		{
			putSymbolToFixedTile(FIXED_SECTION_FIXED_TILES, weaponsData[game.currentWeapon].weaponSymbols[0]);
			putSymbolToFixedTile(FIXED_SECTION_FIXED_TILES +1, weaponsData[game.currentWeapon].weaponSymbols[1]);
			putSymbolToFixedTile(FIXED_SECTION_FIXED_TILES +2, weaponsData[game.currentWeapon].weaponSymbols[2] + (game.configBounceMultiplier > 1));
			putSymbolToFixedTile(FIXED_SECTION_FIXED_TILES +3, weaponsData[game.currentWeapon].weaponSymbols[3] + (game.configTimeMultiplier - 1));

		}
		else
		{
			for (int i = 0; i < FIXED_SECTION_WEAPON_TILES; i++)
			{
				putSymbolToFixedTile(FIXED_SECTION_FIXED_TILES + i, weaponsData[game.currentWeapon].weaponSymbols[i]);
			}			
		}
	}
	if (force != game.firingForce || forceUpdate)
	{
		force = game.firingForce;
		// update force.
		// The bar is 10 tiles long. it has therefore a 0 to 80 maximum.
		int pixelsToCover = (game.firingForce + 1) * FIRING_FORCE_BAR_LENGTH * 8 / 256; // hopefully /256 is implemented as >> 8 by the compiler...
		for (int i = FIRST_FIRING_FORCE_BAR_TILE; i < FIRST_FIRING_FORCE_BAR_TILE + FIRING_FORCE_BAR_LENGTH; i++)
		{
			if (pixelsToCover >= 8)
			{
				fixedSectionMap[FIRING_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_FULL;
			}
			else if (pixelsToCover > 0)
			{
				fixedSectionMap[FIRING_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_POWER_TILE;				
				putSymbolToFixedTile(FIXED_SECTION_POWER_TILE , FIXED_SECTION_TILE_ONE_LEFT + pixelsToCover - 1); //
			}
			else			
			{
				fixedSectionMap[FIRING_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_EMPTY;
			}
			pixelsToCover -= 8;
		}
	}
	// compute teams health
	for (int t = 0; t < MAX_TEAMS ; t ++)
	{
		int teamHealth = 0;
		for (int u = 0; u < MAX_TEAM_UNITS; u++)
		{
			teamHealth += teams[t].health[u];
		}
		// Now, calculate the new pixels to cover:
		teamHealth = teamHealth * HEALTH_BAR_LENGTH * 8 / (100 * game.numberOfUnitsPerTeam);
		//		
		if (healths[t] != teamHealth || forceUpdate)
		{
			if (healths[t] > teamHealth)
				healths[t]--;
			else 
				healths[t]++;
			int pixelsToCover = healths[t];
			for (int i = healthBarCoordinates[t][0]; i < healthBarCoordinates[t][0] + HEALTH_BAR_LENGTH; i++)
			{
				if (pixelsToCover >= 8)
				{
					fixedSectionMap[healthBarCoordinates[t][1] * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_FULL;
				}
				else if (pixelsToCover > 0)
				{
					fixedSectionMap[healthBarCoordinates[t][1] * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_HEALTH_RED_TILE + t;
					putSymbolToFixedTile(FIXED_SECTION_HEALTH_RED_TILE + t , FIXED_SECTION_TILE_ONE_LEFT + pixelsToCover - 1); //
				}
				else
				{
					fixedSectionMap[healthBarCoordinates[t][1] * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_EMPTY;
				}
				pixelsToCover -= 8;
			}		
		}
	}
	// let's consider wind speed. Get a gradual variation.
	int16_t delta = game.windSpeed - windSpeed;
	if (delta >= 2)
	{
		windSpeed +=2;
	}
	else if (delta > 0)
	{
		windSpeed++;
	}
	else if (delta <= -2)
	{
		windSpeed -= 2;
	}
	else if (delta < 0)
	{
		windSpeed--;
	}
	if (delta != 0 || forceUpdate)
	{
		if (windSpeed <= 0)
		{  // wind going left. First, clear out the right bar
			for (int i = FIRST_RIGHT_WIND_BAR_TILE; i < FIRST_RIGHT_WIND_BAR_TILE + WIND_BAR_LENGTH; i++)
			{
				fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_EMPTY;
			}
			if (windSpeed != 0)
			{
				int pixelsToCover = -windSpeed * WIND_BAR_LENGTH * 8 / 127; //127
				for (int i = FIRST_LEFT_WIND_BAR_TILE + WIND_BAR_LENGTH - 1; i >= FIRST_LEFT_WIND_BAR_TILE; i--)
				{
					if (pixelsToCover >= 8)
					{
						fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_FULL;
					}
					else if (pixelsToCover > 0)
					{
						fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_WIND_TILE;
						putSymbolToFixedTile(FIXED_SECTION_WIND_TILE , FIXED_SECTION_TILE_ONE_RIGHT + pixelsToCover - 1); //
					}
					else
					{
						fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_EMPTY;
					}
					pixelsToCover -= 8;
				}
			}
		}
		if (windSpeed >= 0)
		{  // wind going left. First, clear out the left bar
			for (int i = FIRST_LEFT_WIND_BAR_TILE; i < FIRST_LEFT_WIND_BAR_TILE + WIND_BAR_LENGTH; i++)
			{
				fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_EMPTY;
			}
			if (windSpeed != 0)
			{
				int pixelsToCover = windSpeed * WIND_BAR_LENGTH * 8 / 127; //127
				for (int i = FIRST_RIGHT_WIND_BAR_TILE; i < FIRST_RIGHT_WIND_BAR_TILE + WIND_BAR_LENGTH; i++)
				{
					if (pixelsToCover >= 8)
					{
						fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_FULL;
					}
					else if (pixelsToCover > 0)
					{
						fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_WIND_TILE;
						putSymbolToFixedTile(FIXED_SECTION_WIND_TILE , FIXED_SECTION_TILE_ONE_LEFT + pixelsToCover - 1); //
					}
					else
					{
						fixedSectionMap[WIND_FORCE_Y * FIXED_SECTION_MAPSIZEX + i] = FIXED_SECTION_TILE_EMPTY;
					}
					pixelsToCover -= 8;
				}
			}
		}	
	}
}
inline void putSymbolToFixedTile(uint8_t fixedTileNumber, uint8_t symbol)
{
	uint32_t *s = (uint32_t*) fixedSectionTileData[symbol];
	uint32_t *d = (uint32_t*) fixedSectionTiles[fixedTileNumber];
	#if GFX_MODE == TILE_MODE2
		for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 8; i++)
		{
			*d++= *s++;
		}	
	#else
		for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 4; i++)
		{
			*d++= *s++;
		}
	#endif
}
void initFixedSection(uint8_t gameMode)
{
	// menu mode or title screen mode. 
	if (gameMode)
	{
		memcpy(fixedSectionTiles, fixedSectionTileData, FIXED_SECTION_FIXED_TILES * TILE_SIZE_X * TILE_SIZE_Y / 2);
		memcpy(fixedSectionMap, fixedSectionRomMap, sizeof(fixedSectionMap));
		updateFixedSection(1, "", 7, 1);
	}
	else
	{ 
		memset(fixedSectionTiles, 0, 32);
		memset(fixedSectionMap, 0, sizeof(fixedSectionMap));
	}
}