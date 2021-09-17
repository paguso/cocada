
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

#ifndef ARRITER_H
#define ARRITER_H

#include <stdlib.h>
#include <string.h>

#include "bitbyte.h"
#include "new.h"
#include "cstrutil.h"
#include "iter.h"
#include "trait.h"

/**
 * @file arriter.h
 * @author Paulo Fonseca
 *
 * @brief Array iterator.
 */


/**
 * @brief Array iterator type
 */
typedef struct {
	iter _t_iter;
	void *src;
	size_t typesize;
	size_t len;
	size_t index;
} arr_iter;


/**
 * @brief Gets an iterator for an array
 * @param arr (*no transfer*) the source array
 * @param len the source array length
 * @param typesize the size of the elements stored in the source array in bytes
 */
arr_iter *arr_get_iter(void *arr, size_t len, size_t typesize);


DECL_TRAIT(arr_iter, iter);


#endif