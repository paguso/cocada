/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include "coretype.h"

#ifndef MATHUTIL_H
#define MATHUTIL_H


#define UINT64_MAX_PRIME ((uint64_t)18446744073709551557)

/**
 * @file mathutil.h
 * @author Paulo Fonseca
 *
 * @brief Math utility functions.
 */

/**
 * @brief Minimum of two values
 */
#define MIN( A, B )   ( ( (A) < (B) ) ? (A) : (B) )


/**
 * @brief Maximum of two values
 */
#define MAX( A, B )   ( ( (A) > (B) ) ? (A) : (B) )


/**
 * @brief Minimum of three values
 */
#define MIN3( A, B, C ) ( ( (A) < (B) ) ? MIN(A, C) : MIN(B, C) )


/**
 * @brief Maximum of three values
 */
#define MAX3( A, B, C ) ( ( (A) > (B) ) ? MAX(A, C) : MAX(B, C) )


/**
 * @brief Is N even?
 */
#define IS_EVEN( N ) ( ((N) % 2) == 0 )


/**
 * @brief Is N odd?
 */
#define IS_ODD( N ) ( ((N) % 2) == 1 )


/**
 * @brief Declares `divfloor_TYPE`.
 * @param TYPE An unsigned integer type.
 */
#define DECL_DIVFLOOR( TYPE , ...)\
	/**\
	 * @brief Computes floor(num/den) for unsigned integer types.\
	 */\
	TYPE divfloor_##TYPE(TYPE num, TYPE den);

XX_UNSIGNED_INT(DECL_DIVFLOOR)


/**
 * @brief Declares `divceil_TYPE`.
 * @param TYPE An unsigned integer type.
 */
#define DECL_DIVCEIL( TYPE , ...)\
	/**\
	 * @brief Computes ceil(num/den) for unsigned integer types.\
	 */\
	TYPE divceil_##TYPE(TYPE num, TYPE den);

XX_UNSIGNED_INT(DECL_DIVCEIL)


/**
 * @brief Tests whether an unsigned int is a power or two
 */
#define IS_POW2(UNS_INT) (UNS_INT && !(UNS_INT & (UNS_INT - 1)))

/**
 * @brief Declares `pow2ceil_TYPE`.
 * @param TYPE An unsigned integer type.
 */
#define DECL_POW2CEIL( TYPE , ...)\
	/**\
	 * @brief Computes the smallest power of 2 greater or equal to @p val\
	 */\
	TYPE pow2ceil_##TYPE( TYPE val );


#define MEAN(A, B) (((A) + (B)) / 2);

XX_UNSIGNED_INT(DECL_POW2CEIL)


/**
 * @brief Computes (a + b) mod m for 64bit unsigned values.
 * Takes proper care of overflow if a + b > UINT64_MAX.
 */
uint64_t mod_sum(uint64_t a, uint64_t b, uint64_t m);


/**
 * @brief Computes (a * b) mod m for 64bit unsigned values.
 * Takes proper care of overflow if a * b > UINT64_MAX.
 */
uint64_t mod_mult(uint64_t a, uint64_t b, uint64_t m);


/**
 * @brief Computes (a^b) mod m for 64bit unsigned values.
 * Takes proper care of overflow if a^b > UINT64_MAX.
 */
uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m);


/**
 * @brief Naive O(sqrt(n))-time primality testing.
 */
bool is_prime_naive(uint64_t n);


/**
 * @brief Deterministic Miller-Rabin primality testing.
 */
bool is_prime_mr(uint64_t n);


/**
 * @brief Returns the prime sucessor of @p n, i.e. the smallest prime >= @p n.
 */
uint64_t prime_succ(uint64_t n);


/**
 * @brief Declares `average_TYPE`.
 * @param TYPE An unsigned integer type.
 */
#define DECL_AVG(TYPE, ...)\
	/**\
	 * @brief Computes the average of an array of an unsigned int type.\
	 * @warning This function is slow because it takes care of overflows.\
	 */\
	double average_##TYPE(TYPE *vals, size_t n);

XX_UNSIGNED_INT(DECL_AVG)


/**
 * @brief Declares `kth_smallest_TYPE`.
 * @param TYPE The type of the values.
 */
#define DECL_KTH_SMALLEST(TYPE, ...)\
	/**\
	 * @brief Returns the @p kth element of the array @p v of length @p len.\
	 * @param dirty If true, the original array may be reordered in the process.\
	 * Else, a copy is first created and @p v is left intact.\
	 */\
	TYPE kth_smallest_##TYPE(TYPE *v, size_t len, size_t k, bool dirty);

XX_PRIMITIVES(DECL_KTH_SMALLEST)


/**
 * @brief Declares `median_TYPE`.
 * @param TYPE The type of the values.
 */
#define DECL_MEDIAN(TYPE, ...)\
	/**\
	 * @brief Returns the median of the array @p v of length @p len.\
	 * @param dirty If true, the original array may be reordered in the process.\
	 * Else, a copy is first created and @p v is left intact.\
	 */\
	TYPE median_##TYPE(TYPE *v, size_t len, bool dirty);

XX_PRIMITIVES(DECL_MEDIAN)




#endif