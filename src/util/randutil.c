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

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "SFMT.h"
#include "coretype.h"
#include "bitbyte.h"
#include "randutil.h"


static sfmt_t _rng;
static bool __randinit = false;

static inline void _randinit()
{
	if (!__randinit) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		sfmt_init_gen_rand(&_rng, tv.tv_usec);
		__randinit = true;
	}
}


static inline uint64_t _rand_next_unchecked()
{
	return sfmt_genrand_uint64(&_rng);
}


uint64_t rand_next ()
{
	_randinit();
	return _rand_next_unchecked();
}



#define RAND_RANGE_IMPL( TYPE )\
TYPE rand_range_##TYPE(TYPE l, TYPE r) {\
    assert(r >= l);\
    return l + (rand_next() % (r-l));\
}


void shuffle_arr(void *arr, size_t n, size_t typesize)
{
	_randinit();
	byte_t tmp[typesize];
	if (n > 1) {
		for (size_t j, i = n-1; i > 0; i--) {
			j = (size_t)_rand_next_unchecked() % (i+1);
			memcpy(tmp, arr + (j * typesize), typesize);
			memcpy(arr + (j * typesize), arr + (i * typesize), typesize);
			memcpy(arr + (i * typesize), tmp, typesize);
		}
	}
}

#define SHUFFLE_ARR_IMPL(TYPE) \
void shuffle_arr_##TYPE(TYPE *arr, size_t n) {\
	shuffle_arr(arr, n, sizeof(TYPE));\
}


#define RAND_ALL_IMPL(TYPE, ...) \
RAND_RANGE_IMPL(TYPE) \
SHUFFLE_ARR_IMPL(TYPE)

XX_INTS(RAND_ALL_IMPL)
