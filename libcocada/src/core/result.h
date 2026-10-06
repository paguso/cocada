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

#ifndef RESULT_H
#define RESULT_H

/**
 * @file result.h
 * @brief Generic Result type
 * @author Paulo Fonseca
 *
 * This header contain macros for declaring generic Result types.
 * A Result type is used in functions that may result in runtime errors.
 * The type encapsulates both possible results of such operations,
 * the successful result and the error result.
 *
 * There are thwo kinds or results:
 * 1. OK-Result
 * 2. OK-Error-Result
 *
 * An *OK-Result* encapsulates a boolean indicating if the operation was
 * successful and a result value of a given type `T` if the operation succeeds.
 *
 * ```C
 * T_result res = do_something(...);
 * if (res.ok) {
 * 		T val = res.val;
 * 		consume res.val ...
 * } else {
 * 		handle failure...
 * }
 * ```
 *
 * An *OK-Error-Result* encapsulates a boolean indicating if the operation was
 * successful and one of two result values for when the operation succees of fails.
 *
 * ```C
 * T_E_result res = do_something(...);
 * if (res.ok) {
 * 		T ok_val = res.val.ok;
 * 		consume res.val.ok ...
 * } else {
 * 		E err_val = res.value.err;
 * 		consume res.val.err ...
 * 		handle failure...
 * }
 * ```
 */


#include "coretype.h"

/**
 * @brief Declares an OK-Result `NAME_res`.
 * @param NAME The type name prefix.
 * @param OK_TYPE The type of the successful operation result value.
 */
#define DECL_RESULT_OK(NAME, OK_TYPE) \
	/**\
	 * @brief OK-Result type.\
	 */\
	typedef struct {\
		bool ok;        	/**< Success/fail indicator */  \
		OK_TYPE val;	/**< Successful result value. */\
	} NAME##_res;

/**
 * @brief Declares an OK-Error-Result `NAME_res`.
 * @param NAME The type name prefix.
 * @param OK_TYPE The type of the successful operation result value.
 * @param ERR_TYPE The type of the unsuccessful operation result value.
 */
#define DECL_RESULT_OK_ERR(NAME, OK_TYPE, ERR_TYPE) \
	/**\
	 * @brief OK-Error-Result type.\
	 */\
	typedef struct {\
		bool ok;        	/**< Success/fail indicator */  \
		union {\
			OK_TYPE ok;     /**< Successful result value */ \
			ERR_TYPE err;   /**< Unsucessful result value */\
		} val;\
	} NAME##_res;


#endif
