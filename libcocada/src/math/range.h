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

#ifndef RANGE_H
#define RANGE_H

#include "coretype.h"


/**
 * @file range.h
 * @author Paulo Fonseca
 * @brief Arrays of evenly spaced integers, like Python's `range`.
 */

/**
 * @brief A range of a given @p TYPE.
 */
#define DECL_RANGE_ARR(TYPE,...)\
	typedef struct {\
		size_t n;		/**< Number of values */\
		TYPE *arr;		/**< Array of values */\
	} range_##TYPE;

/**
 * @brief Creates a new range array of a given TYPE.
 * @param from Initial value (Included)
 * @param to Last value (excluded)
 * @param step Increment step (a negative value means a decreasing range.
 */
#define DECL_RANGE_ARR_NEW(TYPE,...)\
	range_##TYPE range_arr_new_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step);

/**
 * @brief Returns the number of values in a range without creating one.
 * @param from Initial value (Included)
 * @param to Last value (excluded)
 * @param step Increment step (a negative value means a decreasing range.
 */
#define DECL_RANGE_ARR_LEN(TYPE,...)\
	size_t range_arr_len_##TYPE(TYPE from, TYPE to, SIGNED(TYPE) step);\

/**
 * @brief Fills an existing array with the values in a range.
 * @param dest The destination array
 * @param from Initial value (Included)
 * @param to Last value (excluded)
 * @param step Increment step (a negative value means a decreasing range.
 * @warning No bound checks performed.
 */
#define DECL_RANGE_ARR_FILL(TYPE,...)\
	size_t range_arr_fill_##TYPE(TYPE *dest, TYPE from, TYPE to, SIGNED(TYPE) step);\


XX_INTS(DECL_RANGE_ARR)
XX_INTS(DECL_RANGE_ARR_NEW)
XX_INTS(DECL_RANGE_ARR_LEN)
XX_INTS(DECL_RANGE_ARR_FILL)

#endif
