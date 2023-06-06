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
#include "range.h"


#define RANGE_ARR_NEW_IMPL(TYPE, ...)\
	range_##TYPE range_arr_new_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step) \
	{\
		if (from == to) {\
			return (range_##TYPE) {.n = 0, .arr = (TYPE*)malloc(0)};\
		} else if (from < to) {\
			ERROR_ASSERT(step!=0, "Range with step=0 has infinite size.\n");\
			if (step < 0) {\
				return (range_##TYPE) {.n = 0, .arr = (TYPE*)malloc(0)};\
			}\
			size_t n = (to - from);\
			n = (n / step) + ((n % step) ? 1 : 0);\
			TYPE *arr = (TYPE *)malloc(n * sizeof(TYPE));\
			TYPE value = from;\
			for (size_t i = 0; i < n; i++, value += step) {\
				arr[i] = value;\
			}\
			return (range_##TYPE){.n = n, .arr = arr};\
		} else {\
			ERROR_ASSERT(step!=0, "Range with step=0 has infinite size.\n");\
			if (step > 0) {\
				return (range_##TYPE) {.n = 0, .arr = (TYPE*)malloc(0)};\
			}\
			size_t n = (from - to);\
			n = (n / (-step)) + ((n % (-step)) ? 1 : 0);\
			TYPE *arr = (TYPE *)malloc(n * sizeof(TYPE));\
			TYPE value = from;\
			for (size_t i = 0; i < n; i++, value += step) {\
				arr[i] = value;\
			}\
			return (range_##TYPE){.n = n, .arr = arr};\
		}\
	}



range_int xrange_arr_new_int(int from, int to, SIGNED(int) step)
{
	if (from == to) {
		return (range_int) {
			.n = 0, .arr = (int *)malloc(0)
		};
	}
	else if (from < to) {
		ERROR_ASSERT(step!=0, "Range with step=0 has infinite size.\n");
		if (step < 0) {
			return (range_int) {
				.n = 0, .arr = (int *)malloc(0)
			};
		}
		size_t n = (to - from);
		n = (n / step) + ((n % step) ? 1 : 0);
		int *arr = (int *)malloc(n * sizeof(int));
		int value = from;
		for (size_t i = 0; i < n; i++, value += step) {
			arr[i] = value;
		}
		return (range_int) {
			.n = n, .arr = arr
		};
	}
	else {   // from > to
		ERROR_ASSERT(step!=0, "Range with step=0 has infinite size.\n");
		if (step > 0) {
			return (range_int) {
				.n = 0, .arr = (int *)malloc(0)
			};
		}
		size_t n = (from - to);
		n = (n / (-step)) + ((n%(-step)) ? 1 : 0);
		int *arr = (int *)malloc(n * sizeof(int));
		int value = from;
		for (size_t i = 0; i < n; i++, value += step) {
			arr[i] = value;
		}
		return (range_int) {
			.n = n, .arr = arr
		};
	}
}



XX_INTS(RANGE_ARR_NEW_IMPL)
