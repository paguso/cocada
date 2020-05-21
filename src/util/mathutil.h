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

#include "coretype.h"

#ifndef MATHUTIL_H
#define MATHUTIL_H

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
#define IS_EVEN( N ) ( 1 - ( (N) % 2) )


/**
 * @brief Is N odd?
 */
#define IS_ODD( N ) ( (N) % 2 )


/**
 * @brief Computes floor(value/base) as double values
 */
#define DIVFLOOR(NUM, DEN) (floor(((double)(NUM))/((double)(DEN))))


/**
 * @brief Computes ceil(value/base) as double values
 */
#define DIVCEIL(NUM, DEN) (ceil(((double)(NUM))/((double)(DEN))))


/**
 * @brief Computes the smallest power of 2 greater or equal to @p val
 */
#define POW2CEIL_DECL( TYPE , ...)\
TYPE pow2ceil_##TYPE( TYPE val );


XX_UNSIGNED_INT(POW2CEIL_DECL)


#endif