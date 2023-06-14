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

#include <stdlib.h>

#include "coretype.h"
#include "errlog.h"
#include "memdbg.h"
#include "range.h"



#define RANGE_ARR_IMPL(TYPE, ...)\
	size_t range_arr_len_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step)\
	{\
		if (from == to) {\
			return 0;\
		} else if (from < to) {\
			ERROR_ASSERT(step!=0, "Range with step=0 has infinite size.\n");\
			if (step < 0) {\
				return 0;\
			}\
			size_t n = (to - from);\
			n = (n / step) + ((n % step) ? 1 : 0);\
			return n;\
		} else {\
			ERROR_ASSERT(step!=0, "Range with step=0 has infinite size.\n");\
			if (step > 0) {\
				return 0;\
			}\
			size_t n = (from - to);\
			n = (n / (-step)) + ((n % (-step)) ? 1 : 0);\
			return n;\
		}\
	}\
	\
	size_t range_arr_fill_##TYPE(TYPE *dest, TYPE from, TYPE to, SIGNED(TYPE) step)\
	{\
		size_t n = range_arr_len_##TYPE(from, to, step);\
		for (size_t i = 0; i < n; i++, from += step) {\
			dest[i] = from;\
		}\
		return n;\
	}\
	\
	range_##TYPE range_arr_new_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step) \
	{\
		size_t n = range_arr_len_##TYPE(from, to, step);\
		TYPE *arr = (TYPE *)malloc(n * sizeof(TYPE));\
		range_arr_fill_##TYPE(arr, from, to, step);\
		return (range_##TYPE){.n = n, .arr = arr};\
	}\

XX_INTS(RANGE_ARR_IMPL)
