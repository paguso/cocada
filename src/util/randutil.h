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

#include "coretype.h"

/**
 * @file randutil.h
 * @author Paulo Fonseca
 * @brief Random number generation utility functions
 */


/**
 * Returns a randomly generated 64-bit unsigned integer
 */
uint64_t rand_next ();


/*
 * Generic rand_range declaration
 */
#define RAND_RANGE_DECL(TYPE)\
/** Returns a uniformly picked TYPE random number in the range @p min to @p max-1*/\
TYPE rand_range_##TYPE(TYPE min, TYPE max);


/**
 * Shuffle in-place the elements of array @p arr containing @p n elements
 * of size @p typesize
 */
void shuffle_arr(void *arr, size_t n, size_t typesize);


/*
 * Generic shuffle_arr declaration
 */
#define SHUFFLE_ARR_DECL(TYPE)\
/** Suffles an array of @p n TYPE elements in place */\
void shuffle_arr_##TYPE(TYPE *arr, size_t n);


#define RAND_ALL_DECL(TYPE, ...)\
SHUFFLE_ARR_DECL(TYPE)\
RAND_RANGE_DECL(TYPE)

XX_INTS(RAND_ALL_DECL)


double rand_norm();
double rand_unif();


#endif