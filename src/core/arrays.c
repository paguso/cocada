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

#include "arrays.h"
#include "errlog.h"

void *arr_new (size_t typesize, size_t len)
{
	size_t size = len * typesize;
	void *ret = malloc(size + sizeof(size_t));
	*((size_t *)ret) = size;
	return ret + sizeof(size_t);
}


void *arr_realloc(void *arr, size_t size)
{
	arr -= sizeof(size_t);
	arr = realloc(arr, size + sizeof(size_t));
	*((size_t *)arr) = size;
	return arr + sizeof(size_t);
}

size_t arr_size(void *arr)
{
	return *((size_t *)(arr - sizeof(size_t)));
}

void arr_free(void *arr)
{
	free (arr - sizeof(size_t));
}


#define ARR_NEW_IMPL(TYPE, ...)\
	TYPE *arr_##TYPE##_new(size_t len)\
	{\
		return (TYPE *)arr_new(sizeof(TYPE), len);\
	}
XX_CORETYPES(ARR_NEW_IMPL)


#define ARR_REALLOC_IMPL(TYPE, ...)\
	TYPE *arr_##TYPE##_realloc(TYPE *arr, size_t len)\
	{\
		return arr_realloc(arr, sizeof(TYPE) * len);\
	}
XX_CORETYPES(ARR_REALLOC_IMPL)


#define ARR_LEN_IMPL(TYPE, ...)\
	size_t arr_##TYPE##_len(TYPE *arr)\
	{\
		WARN_IF(arr_size(arr) % sizeof(TYPE),\
		        "Physical array size is not a multiple of TYPE size");\
		return arr_size(arr) / sizeof(TYPE);\
	}
XX_CORETYPES(ARR_LEN_IMPL)