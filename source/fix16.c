/*
 * fix16.c
 * This file is part of libfixmath
 *
 * Copyright (C) 2015 - Félix Arreola Rodríguez
 *
 * libfixmath is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libfixmath is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libfixmath. If not, see <http://www.gnu.org/licenses/>.
 */

#include <3ds.h>

#include "fix16.h"

fix16_t fix16_add(fix16_t a, fix16_t b) {
	// Use unsigned integers because overflow with signed integers is
	// an undefined operation (http://www.airs.com/blog/archives/120).
	u32 _a = a, _b = b;
	u32 sum = _a + _b;

	// Overflow can only happen if sign of a == sign of b, and then
	// it causes sign of sum != sign of a.
	if (!((_a ^ _b) & 0x80000000) && ((_a ^ sum) & 0x80000000))
		return fix16_overflow;
	
	return sum;
}

fix16_t fix16_sub(fix16_t a, fix16_t b) {
	u32 _a = a, _b = b;
	u32 diff = _a - _b;

	// Overflow can only happen if sign of a != sign of b, and then
	// it causes sign of diff != sign of a.
	if (((_a ^ _b) & 0x80000000) && ((_a ^ diff) & 0x80000000))
		return fix16_overflow;
	
	return diff;
}

fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1) {
	// Each argument is divided to 16-bit parts.
	//					AB
	//			*	 CD
	// -----------
	//					BD	16 * 16 -> 32 bit products
	//				 CB
	//				 AD
	//				AC
	//			 |----| 64 bit product
	s32 A = (inArg0 >> 16), C = (inArg1 >> 16);
	u32 B = (inArg0 & 0xFFFF), D = (inArg1 & 0xFFFF);
	
	s32 AC = A*C;
	s32 AD_CB = A*D + C*B;
	u32 BD = B*D;
	
	s32 product_hi = AC + (AD_CB >> 16);
	
	// Handle carry from lower 32 bits to upper part of result.
	u32 ad_cb_temp = AD_CB << 16;
	u32 product_lo = BD + ad_cb_temp;
	if (product_lo < BD)
		product_hi++;
	
	// The upper 17 bits should all be the same (the sign).
	if (product_hi >> 31 != product_hi >> 15)
		return fix16_overflow;
	
	// Subtracting 0x8000 (= 0.5) and then using signed right shift
	// achieves proper rounding to result-1, except in the corner
	// case of negative numbers and lowest word = 0x8000.
	// To handle that, we also have to subtract 1 for negative numbers.
	u32 product_lo_tmp = product_lo;
	product_lo -= 0x8000;
	product_lo -= (u32)product_hi >> 31;
	if (product_lo > product_lo_tmp)
		product_hi--;
	
	// Discard the lowest 16 bits. Note that this is not exactly the same
	// as dividing by 0x10000. For example if product = -1, result will
	// also be -1 and not 0. This is compensated by adding +1 to the result
	// and compensating this in turn in the rounding above.
	fix16_t result = (product_hi << 16) | (product_lo >> 16);
	result += 1;
	return result;
}

fix16_t fix16_div(fix16_t a, fix16_t b) {
	// This uses the basic binary restoring division algorithm.
	// It appears to be faster to do the whole division manually than
	// trying to compose a 64-bit divide out of 32-bit divisions on
	// platforms without hardware divide.
	
	if (b == 0)
		return fix16_minimum;
	
	uint32_t remainder = (a >= 0) ? a : (-a);
	uint32_t divider = (b >= 0) ? b : (-b);

	uint32_t quotient = 0;
	uint32_t bit = 0x10000;
	
	/* The algorithm requires D >= R */
	while (divider < remainder)
	{
		divider <<= 1;
		bit <<= 1;
	}
	
	if (!bit)
		return fix16_overflow;
	
	if (divider & 0x80000000)
	{
		// Perform one step manually to avoid overflows later.
		// We know that divider's bottom bit is 0 here.
		if (remainder >= divider)
		{
				quotient |= bit;
				remainder -= divider;
		}
		divider >>= 1;
		bit >>= 1;
	}
	
	/* Main division loop */
	while (bit && remainder)
	{
		if (remainder >= divider)
		{
				quotient |= bit;
				remainder -= divider;
		}
		
		remainder <<= 1;
		bit >>= 1;
	}	 
			
	if (remainder >= divider)
	{
		quotient++;
	}
	
	fix16_t result = quotient;
	
	/* Figure out the sign of result */
	if ((a ^ b) & 0x80000000)
	{
		if (result == fix16_minimum)
				return fix16_overflow;
		
		result = -result;
	}
	
	return result;
}

