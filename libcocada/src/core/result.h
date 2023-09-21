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
 * For example, suppose that a function reads a serialised object of type `T`
 * from a file whose path is given as input. A possible prototype for
 * this function could be.
 *
 * ```C
 * T* read_T_from_file(char *path);
 * ```
 *
 * However, this process may incur in a IO runtime error. In this case, we
 * could have this error be represented by a custom error type E which contains
 * a code and a message.
 *
 * ```C
 * typedef struct {
 *  int code;
 *  char *msg;
 * } E;
 * ```
 *
 * By using the macro `DECL_RESULT_OK_ERR(R, T*, E*) a type named `R_res` which could
 * be used as follows.
 *
 * ```C
 * DECL_RESULT_OK(R, T*, E*);
 *
 * R_res read_T_from_file(char *path) {
 *  R_res result;
 *  T *read_obj;
 *  // try to read the object from file into read_obj
 *  // if an error occurs at some point, goto FAIL
 *  result.ok = true;
 *  result.res.ok = read_obj;
 *  goto SUCCESS;
 * FAIL:
 *  result.ok = false;
 *  result.val.err.code = //set error code;
 *  result.val.err.msg  = //set error message;
 * SUCCESS:
 *  // free resources
 *  return result;
 * }
 *
 * ```
 *
 */


#include "coretype.h"

/**
 * @brief Declares a @p NAME_res result type with success value only.
 */
#define DECL_RESULT_OK(NAME, OK_RES_TYPE) \
	typedef struct {\
		bool ok;\
		OK_RES_TYPE val;\
	} NAME##_res;


/**
 * @brief Declares a @p NAME_res result type with success and error values.
 */
#define DECL_RESULT_OK_ERR(NAME, OK_RES_TYPE, ERR_RES_TYPE) \
	typedef struct {\
		bool ok;\
		union {\
			OK_RES_TYPE ok;\
			ERR_RES_TYPE err;\
		} val;\
	} NAME##_res;



#endif