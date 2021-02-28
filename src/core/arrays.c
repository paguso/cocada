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
#include "memdbg.h"

void *sa_arr_calloc(size_t nmemb, size_t memb_size)
{
	size_t size = nmemb * memb_size;
	void *ret = malloc(size + sizeof(size_t));
	*((size_t *)ret) = size;
	return ret + sizeof(size_t);
}


void *sa_arr_realloc(void *arr, size_t nmemb, size_t memb_size)
{
	size_t size = nmemb * memb_size;
	arr -= sizeof(size_t);
	arr = realloc(arr, size + sizeof(size_t));
	*((size_t *)arr) = size;
	return arr + sizeof(size_t);
}


size_t sa_arr_sizeof(void *arr)
{
	return *((size_t *)(arr - sizeof(size_t)));
}


void sa_arr_free(void *arr)
{
	free (arr - sizeof(size_t));
}


#define SA_ARR_IMPL(TYPE, ...)\
TYPE *sa_arr_##TYPE##_calloc(size_t nmemb)\
{\
	return (TYPE *)sa_arr_calloc(nmemb, sizeof(TYPE));\
}\
\
TYPE *sa_arr_##TYPE##_realloc(TYPE *arr, size_t nmemb)\
{\
	return (TYPE *)sa_arr_realloc(arr, nmemb, sizeof(TYPE));\
}\
\
size_t sa_arr_##TYPE##_len(TYPE *arr)\
{\
	WARN_IF(sa_arr_sizeof(arr) % sizeof(TYPE),\
			"Physical array size is not a multiple of TYPE size");\
	return sa_arr_sizeof(arr) / sizeof(TYPE);\
}

XX_CORETYPES(SA_ARR_IMPL)
