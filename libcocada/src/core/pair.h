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

#ifndef PAIR_H
#define PAIR_H

/**
 * @file pair.h
 * @author Paulo Fonseca
 * @brief Pair type
 */

#include "coretype.h"

/**
 * @brief Declares a pair of given types.
 * By importing this file "pair.h", homogenous pairs are declared for all core types,
 * ex. pair_int_int, pair_double_double, etc.
 * @param TYPE1 The type of the first element.
 * @param TYPE2 The type of the second element.
 * @see coretype.h
 */
#define DECL_PAIR(TYPE1, TYPE2, ...) \
	/**\
	 * @brief A pair of a `TYPE1` and a `TYPE2`.\
	 */\
	typedef struct pair_##TYPE1##_##TYPE2 { \
		TYPE1 first; /**< The first element */ \
		TYPE2 second; /**< The second element */ \
	} pair_##TYPE1##_##TYPE2;


/**
 * @brief Declares an homogenous pair. Same as `DECL_PAIR(TYPE, TYPE)`.
 * @param TYPE The type of both elements.
 * @see DECL_PAIR
 */
#define DECL_PAIR2(TYPE, ...) DECL_PAIR(TYPE, TYPE)

/**
 * @brief To be used as type name for a pair of given types.
 * For example
 *
 * ```c
 * DECL_PAIR(int, char);
 *
 * int main() {
 * 		PAIR(int, char) p = {1, 'a'};
 * 		PAIR(int, char) q = {2, 'b'};
 * 		...
 * }
 *
 * ```
 */
#define PAIR(TYPE1, TYPE2) pair_##TYPE1##_##TYPE2

/**
 * @brief Homogenous pair type name. Same as `PAIR(TYPE, TYPE)`.
 * @param TYPE The type of both elements.
 * @see PAIR
 */
#define PAIR2(TYPE) pair_##TYPE##_##TYPE

/**
 * @hide
 */
#define DECL_PAIRS2(TYPE,...) \
	DECL_PAIR2(TYPE)

XX_CORETYPES(DECL_PAIRS2)

#endif // PAIR_H
