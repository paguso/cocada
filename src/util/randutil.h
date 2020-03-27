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

#ifndef RANDUTIL_H
#define RANDUTIL_H

#include <stddef.h>
#include <stdint.h>

/**
 * Shuffle in-place the elements of array @p arr containing @p n elements
 * of size @p typesize
 */
void shuffle_arr(void *arr, size_t n, size_t typesize);


#define RAND_RANGE_DECL( TYPE )\
TYPE rand_range_##TYPE(TYPE l, TYPE r);

RAND_RANGE_DECL(short);
RAND_RANGE_DECL(int);
RAND_RANGE_DECL(long);
RAND_RANGE_DECL(size_t);
RAND_RANGE_DECL(int8_t);
RAND_RANGE_DECL(int16_t);
RAND_RANGE_DECL(int32_t);
RAND_RANGE_DECL(int64_t);
RAND_RANGE_DECL(uint8_t);
RAND_RANGE_DECL(uint16_t);
RAND_RANGE_DECL(uint32_t);
RAND_RANGE_DECL(uint64_t);

#endif