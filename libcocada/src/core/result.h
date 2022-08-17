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
 * By using the macro `DECL_RESULT(R, T*, E*)` a type named `R_res` which could
 * be used as follows.
 *
 * ```C
 * DECL_RESULT(R, T*, E*);
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
 *  result.res.err = NEW(E);
 *  result.res.err->code = //set error code;
 *  result.res.err->msg  = //set error message;
 * SUCCESS:
 *  // free resources
 *  return result;
 * }
 *
 * ```
 *
 */


#include "coretype.h"


#define DECL_RESULT(N, T) \
	typedef struct {\
		bool ok;\
		T res;\
	} N##_res;\

#define DECL_RESULT_ERR(N, T, E) \
	typedef T N##_ok_t;\
	typedef E N##_err_t;\
	typedef struct {\
		bool ok;\
		union {\
			T ok;\
			E err;\
		} res;\
	} N##_res;\



#endif