
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

#ifndef SAARRAY_H
#define SAARRAY_H

#include <stddef.h>

#include "coretype.h"

/**
 * @file arrays.h
 * @author Paulo Fonseca
 *
 * @brief Size-annotated arrays
 *
 * Some functions defined here deal with *size-annotated arrays* (`sa_arr`).
 * A size-annotated array is an array with a prepended `size_t` value
 * indicating its useful capacity in bytes. This can be used for
 * bounds-checking in some situations without the need for providing this
 * information explicitly. Notice that the term "size" here refers to the
 * "physical" size of the useful part of array, not its "logical".
 *
 * The physical layout of such an array can be depicted as
 *
 *
 * ```
 *
 *                       |<---------------------- S Bytes ---------------------->|
 *
 *	+--------------------+-------------------------------------------------------+
 *  |     S (size_t)     |                 Useful array area                     |
 *  +--------------------+-------------------------------------------------------+
 *                        ^
 *                        |
 * 	        The handler is a pointer to this location
 *
 *
 * ```
 *
 * When creating such an array of size `S`, one should allocate memory for the whole
 * object at once, that is `S + sizeof(size_t)` bytes. This is necessary to ensure
 * that the size comes immediately before the useful area of the array in memory.
 * However the handler used to manipulate the array, that is to access, read and write
 * elements is actually a pointer to the start of the useful area.
 *
 */


/**
 * @brief Allocates an array of @p nmemb elements, each of @p memb_size
 * bytes, **with prepended size information** (sa_arr).
 *
 * This function allocates an array of `S = nmemb * nmemb_size` bytes **plus**
 * the size of a size_t value **immediately before** the useful area
 * of the array. The size `S` is stored at this location as a size_t value,
 * the useful area is initialised with 0's, and a pointer to the start
 * location of the useful area of the array  is returned, as illustrated in
 * the diagram below. This makes the array size readily available via
 * the ::sa_arr_sizeof function, without having to store this information separately.
 *
 * ```
 *
 *                       |<---------- S = (nmemb * nmemb_size) Bytes ----------->|
 *
 *	+--------------------+-------------------------------------------------------+
 *  |     S (size_t)     |                 Useful array area                     |
 *  +--------------------+-------------------------------------------------------+
 *                        ^
 *                        |
 * 	        returns a pointer to this location
 *
 *
 * ```
 *
 * @warning Although it can be seamlessly accessed through the  returned pointer,
 * this array should be only reallocated or freed via the companion functions
 * ::sa_arr_realloc and ::sa_arr_free.
 *
 * @see sa_arr_realloc
 * @see sa_arr_free
 * @see sa_arr_sizeof
 */
void *sa_arr_calloc(size_t nmemb, size_t memb_size);


/**
 * @brief Reallocs an array with prepended size information.
 * @see sa_arr_calloc
 * @see sa_arr_free
 * @see sa_arr_sizeof
 */
void *sa_arr_realloc(void *arr, size_t nmemb, size_t memb_size);


/**
 * @brief Returns the useful capacity, in bytes, of an array
 * with prepended size info.
 * @see sa_arr_calloc
 * @see sa_arr_free
 * @see sa_arr_sizeof
 */
size_t sa_arr_sizeof(void *arr);


/**
 * @brief Deallocates an array with prepended size info.
 * This will release all allocated memory consisting of the
 * useful area and the prepended capacity.
 */
void sa_arr_free(void *arr);


#define SA_ARR_DECL(TYPE, ...)\
	TYPE *sa_arr_##TYPE##_calloc(size_t nmemb);\
	TYPE *sa_arr_##TYPE##_realloc(TYPE *arr, size_t nmemb);\
	size_t sa_arr_##TYPE##_len(TYPE *arr);

XX_CORETYPES(SA_ARR_DECL)

#endif
