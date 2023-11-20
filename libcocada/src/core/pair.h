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
 * @see coretype.h
 */
#define DECL_PAIR(type1, type2, ...) \
	typedef struct pair_##type1##_##type2 { \
		type1 first; \
		type2 second; \
	} pair_##type1##_##type2;


#define DECL_PAIR2(TYPE, ...) DECL_PAIR(TYPE, TYPE)

/**
 * @brief To be used as type name for a pair of given types.
 * For example
 * ```
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

#define PAIR2(TYPE) pair_##TYPE##_##TYPE

#define DECL_PAIRS2(TYPE,...) \
	DECL_PAIR2(TYPE)

XX_CORETYPES(DECL_PAIRS2)

#endif // PAIR_H
