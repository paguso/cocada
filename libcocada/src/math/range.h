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

#ifndef RANGE_H
#define RANGE_H

#include "coretype.h"

#define RANGE_ARR_DECL(TYPE,...)\
	typedef struct {\
		size_t n;\
		TYPE *arr;\
	} range_##TYPE;\
	\
	size_t range_arr_len_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step);\
	size_t range_arr_fill_##TYPE(TYPE *dest, TYPE from, TYPE to, SIGNED(TYPE) step);\
	range_##TYPE range_arr_new_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step);


XX_INTS(RANGE_ARR_DECL)

#endif
