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

#include "coretype.h"

#define DECL_PAIR(type1, type2, ...) \
	typedef struct pair_##type1##_##type2 { \
		type1 first; \
		type2 second; \
	} pair_##type1##_##type2;

#define DECL_PAIRS(TYPE,...) \
	DECL_PAIR(TYPE, TYPE)

XX_CORETYPES(DECL_PAIRS)

#define PAIR(type1, type2) pair_##type1##_##type2

#define MAKE_PAIR(type1, type2, first, second) \
	(PAIR(type1, type2)) { \
		.first = first, \
		         .second = second \
	}


#endif // PAIR_H