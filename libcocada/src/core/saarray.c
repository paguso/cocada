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

#include <stddef.h>
#include <stdlib.h>

#include "saarray.h"
#include "errlog.h"

void *sa_arr_calloc(usize nmemb, usize memb_size)
{
	usize size = nmemb * memb_size;
	void *ret = malloc(size + sizeof(usize));
	*((usize *)ret) = size;
	return ret + sizeof(usize);
}


void *sa_arr_realloc(void *arr, usize nmemb, usize memb_size)
{
	usize size = nmemb * memb_size;
	arr -= sizeof(usize);
	arr = realloc(arr, size + sizeof(usize));
	*((usize *)arr) = size;
	return arr + sizeof(usize);
}


usize sa_arr_sizeof(void *arr)
{
	return *((usize *)(arr - sizeof(usize)));
}


void sa_arr_free(void *arr)
{
	free (arr - sizeof(usize));
}


#define SA_ARR_IMPL(TYPE, ...)\
	TYPE *sa_arr_##TYPE##_calloc(usize nmemb)\
	{\
		return (TYPE *)sa_arr_calloc(nmemb, sizeof(TYPE));\
	}\
	\
	TYPE *sa_arr_##TYPE##_realloc(TYPE *arr, usize nmemb)\
	{\
		return (TYPE *)sa_arr_realloc(arr, nmemb, sizeof(TYPE));\
	}\
	\
	usize sa_arr_##TYPE##_len(TYPE *arr)\
	{\
		WARN_IF(sa_arr_sizeof(arr) % sizeof(TYPE),\
		        "Physical array size is not a multiple of TYPE size");\
		return sa_arr_sizeof(arr) / sizeof(TYPE);\
	}

XX_CORETYPES(SA_ARR_IMPL)
