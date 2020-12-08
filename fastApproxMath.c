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
 * fastApproxMath.h/c: this files allows a quick evaluation of atan2 and sqrt
 * with a decent level of precision.
 */
#include "fastApproxMath.h" 
int32_t fastApproxAtan2(int32_t x, int32_t y)
{
	/* we subdivide the plane in 4 quadrants.
	*
	*          \ Q1/
	*           \|/
	*       --Q2-+--Q0--
	*           /|\
	*          /Q3 \
	*
	* Q0: x > |y|
	* Q1: y > |x|
	* Q2: -x > |y|
	* Q3: -y > |x|
	*/
	// let's avoid singularities.
	if (x == 0)
	{
		if (y > 0)
			return (FIXED_PI / 2);
		else
			return (FIXED_PI/2 * 3);
	}
	if (y == 0)
	{
		if (x > 0)
			return 0;
		else
			return FIXED_PI;
	}
	int32_t yabs = y > 0 ? y : -y;
	if (x >= yabs )	// note: x > 0 is checked for sure, as yabs >= 0.
	{  // Q0
	    if (x == yabs)
            return (y > 0 ? FIXED_PI/4 : - FIXED_PI/4);
		return approxAtanTable[  (NUMBER_OF_ATAN_ENTRIES * y / x + NUMBER_OF_ATAN_ENTRIES)/2 ] << 14;
	}
	else
	{
		int32_t xabs = x > 0 ? x : -x;
		if (y > xabs)  // Q1
		{
			return FIXED_PI/2 - (approxAtanTable[ (NUMBER_OF_ATAN_ENTRIES * x / y + NUMBER_OF_ATAN_ENTRIES)/2 ] << 14);
		}
		else if (xabs >= yabs)	// now, if we are here we are just checking -x > yabs.
		{
            if (xabs == yabs)
                return y > 0 ? FIXED_PI / 4 * 3: FIXED_PI / 4 * 5;

			return FIXED_PI + (approxAtanTable[ ( NUMBER_OF_ATAN_ENTRIES * y / x + NUMBER_OF_ATAN_ENTRIES)/2 ] << 14);
		}
		else
		{	// then it must be yabs >= xabs. Last quadrant
			return FIXED_PI/2 * 3 - (approxAtanTable[(NUMBER_OF_ATAN_ENTRIES * x / y + NUMBER_OF_ATAN_ENTRIES)/2] << 14);
		}
		// now just the 45 ° are left.
	}

}

uint32_t fastApproximateSqrt(uint32_t x)
{
	// x represents a 16.16 fixed point decimal.
	if (x == 0)
		return 0;
	int n = (33 - 8  - __builtin_clz(x)) & (~1);		// let's get the smallest even number of positions we need to shift our x, so that it is strictly smaller than 256
	// now we get the sqrt
	return (sqrtTable[( x >> n)] << ((n >> 1))) >> 4;
}