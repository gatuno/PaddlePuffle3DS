/*
 * fix16.h
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

#ifndef __LIBFIXMATH_FIX16_H__
#define __LIBFIXMATH_FIX16_H__

#ifndef FIXMATH_FUNC_ATTRS
# ifdef __GNUC__
#   if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#     define FIXMATH_FUNC_ATTRS __attribute__((leaf, nothrow, const))
#   else
#     define FIXMATH_FUNC_ATTRS __attribute__((nothrow, const))
#   endif
# else
#   define FIXMATH_FUNC_ATTRS
# endif
#endif

#include <3ds.h>

typedef s32 fix16_t;

static const fix16_t fix16_maximum  = 0x7FFFFFFF; /*!< the maximum value of fix16_t */
static const fix16_t fix16_minimum  = 0x80000000; /*!< the minimum value of fix16_t */
static const fix16_t fix16_overflow = 0x80000000; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */

static const fix16_t fix16_one = 0x00010000; /*!< fix16_t value of 1 */

static inline fix16_t fix16_from_int (int a) {return a * fix16_one;}

static inline int fix16_to_int (fix16_t a) {
	if (a >= 0)
		return (a + (fix16_one >> 1)) / fix16_one;
	return (a - (fix16_one >> 1)) / fix16_one;
}

static inline fix16_t fix16_abs(fix16_t x) { return (x < 0 ? -x : x); }
static inline fix16_t fix16_floor(fix16_t x) { return (x & 0xFFFF0000UL); }
static inline fix16_t fix16_ceil(fix16_t x) { return (x & 0xFFFF0000UL) + (x & 0x0000FFFFUL ? fix16_one : 0); }
static inline fix16_t fix16_min(fix16_t x, fix16_t y) { return (x < y ? x : y); }
static inline fix16_t fix16_max(fix16_t x, fix16_t y) { return (x > y ? x : y); }
static inline fix16_t fix16_clamp(fix16_t x, fix16_t lo, fix16_t hi) { return fix16_min(fix16_max(x, lo), hi); }

extern fix16_t fix16_add(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;
extern fix16_t fix16_sub(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;

extern fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1) FIXMATH_FUNC_ATTRS;
extern fix16_t fix16_div(fix16_t inArg0, fix16_t inArg1) FIXMATH_FUNC_ATTRS;

#endif /* __LIBFIXMATH_FIX16_H__ */

