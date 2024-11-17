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

#ifndef ARRAYUTIL_H
#define ARRAYUTIL_H

#include <stddef.h>
#include <string.h>

#include "coretype.h"
#include "memdbg.h"

/**
 * @file arrays.h
 * @author Paulo Fonseca
 *
 * @brief Array utility macros and functions.
 */


/**
 * @brief Allocates a new array of N elements of a given TYPE.
 */
#define ARR_NEW( TYPE, N ) (((N) > 0) ? ((TYPE*)(malloc((N)*sizeof(TYPE)))) : NULL)


/**
 * @brief Allocates a new array of N elements of a given TYPE all set to 0.
 */
#define ARR_OF_0_NEW( TYPE, N ) (((N) > 0) ? (TYPE *) calloc((N), sizeof(TYPE)) : NULL)


/**
 * @brief Sets the positions FROM to TO-1 of the array ARR with the expression EXPR.
 * @warning The expression EXPR is evaluated for every element to be set.
 *          If EXPR is an object creator call `type_new()` then every position
 *          will be set to a different instance.
 */
#define ARR_FILL( ARR, FROM, TO, EXPR ) \
	for(usize _i=(FROM), _to=(TO); _i<_to; (ARR)[_i++]=(EXPR))


/**
 * @brief Copies N elements from an array SRC from position =FROMSRC
 *        into an array DEST from position =FROMDEST.
 */
#define ARR_COPY( DEST, FROMDEST, SRC, FROMSRC, N )\
	for(usize _i=0, _n=(N), _fs=(FROMSRC), _fd=(FROMDEST); _i<_n; _i++)\
		(DEST)[_fd+_i]=(SRC)[_fs+_i]


/**
 * @brief Prints the array @p ARR from position @p FROM to position @p TO-1
 *        using @p LABEL as label, displaying @p ELTSPERLINE elements per line,
 *        separated by @p SEPARATOR and using the printf format string @p FORMAT.
 *        @p LEFT_MARGIN is a string that is printed at the beginning of each line.
 */
#define ARR_FPRINT(STREAM, ARR, FROM, TO, ELTSPERLINE, LABEL, FORMAT, SEPARATOR, LEFT_MARGIN)\
	{ fprintf(STREAM, "%s"LABEL"[%zu:%zu] =", LEFT_MARGIN, ((usize)(FROM)), ((usize)(TO)));\
		for (usize __i=FROM, __el=(ELTSPERLINE); __i<TO; __i++) {\
			if(!((__i-FROM)%__el)) fprintf(STREAM, "\n%s%4zu: ",LEFT_MARGIN, __i);\
			fprintf(STREAM, FORMAT"%s" , ARR[__i], (__i<(TO-1))?SEPARATOR:"");}\
		fprintf(STREAM, "\n");}


/**
 * @brief Same as `ARR_FPRINT(stdout, ARR, FROM, TO, ELTSPERLINE, LABEL, FORMAT, SEPARATOR, LEFT_MARGIN)`
 */
#define ARR_PRINT(ARR, FROM, TO, ELTSPERLINE, LABEL, FORMAT, SEPARATOR, LEFT_MARGIN)\
	ARR_FPRINT(stdout, ARR, FROM, TO, ELTSPERLINE, LABEL, FORMAT, SEPARATOR, LEFT_MARGIN)


/**
 * @brief Creates a new matrix of a given TYPE with @p ROWS rows and @p COLS columns.
 */
#define NEW_MATRIX(ID, TYPE, ROWS, COLS)\
	TYPE** ID = (TYPE**) malloc( ( (ROWS) * sizeof(TYPE*) ) + ((ROWS) * (COLS) * sizeof(TYPE)));\
	TYPE* __ptr##ID =(TYPE *) (ID + (ROWS));\
	for (usize __i=0; __i < (ROWS); __i++){\
		ID[__i] = __ptr##ID;\
		__ptr##ID += (COLS);\
	}

/**
 * @brief Creates a new matrix of a given TYPE with @p ROWS rows and @p COLS columns
 * and initializes it to 0.
 */
#define NEW_MATRIX_0(ID, TYPE, ROWS, COLS)\
	usize __len##ID =  ( (ROWS) * sizeof(TYPE*) ) + ((ROWS) * (COLS) * sizeof(TYPE) );\
	TYPE** ID = (TYPE**) malloc(__len##ID);\
	memset(ID, 0x0, __len##ID);\
	TYPE* __ptr##ID = (TYPE *) (ID + (ROWS));\
	for (usize __i=0; __i < (ROWS); __i++){\
		ID[__i] = __ptr##ID;\
		__ptr##ID += (COLS);\
	}

/**
 * @brief Frees a matrix created with ::NEW_MATRIX or ::NEW_MATRIX_0.
 */
#define FREE_MATRIX(ID) free(ID)

/**
 * @brief Fills a matrix with a given expression.
 * @warning The expression @p EXPR is evaluated for every element to be set.
 */
#define FILL_MATRIX(ID, ROWS, COLS, EXPR)\
	for (usize __i=0, __li = (ROWS); __i < __li; __i++) \
		for (usize __j=0, __lj = (COLS); __j < __lj; __j++) \
			ID[__i][__j] = (EXPR);\


/**
 * @brief Expands into a type name for an array with elements of a
 * given TYPE called Array_TYPE (for example Array_int, Array_usize, etc).
 * A TYPEArray encapsulates an ordinary C array of TYPE and its
 * length in a struct. This is convenient because we can pass and
 * receive the array and its length to and from functions as a single
 * argument. Differently from vectors and other generic arrays, the type
 * of the elements makes its use more convenient, without the need for
 * casts and other type conversions.
 *
 * Prior to being used, a TYPEArray must be declared with the macro
 * ::DECL_ARRAY. By importing this file you get the declaration of
 * TYPEArray for all the core types defined in coretype.h.
 *
 * An Array object is primarily meant to be created on the stack,
 * although the encapsulated array will typically be allocated on
 * the heap. Thus we can pass and receive a TYPEArray by value.
 *
 * Example:
 * ```
 * void print_int_array(ARRAY(int) a){
 * 	for (usize i=0; i<a.len; i++){
 * 		printf("%d ", a.arr[i]);
 * 	}
 * }
 *
 * int main() {
 * 	ARRAY(int) a = ARRAY_NEW(int, 10);
 * 	for (usize i=0; i<a.len; i++){
 * 		a.arr[i] = i;
 * 	}
 * 	print_int_array(a);
 * 	ARRAY_FREE(a);
 * }
 * ```
 */
#define ARRAY(TYPE) Array_##TYPE

/**
 * @brief Declares a type name for an array with elements of a
 * given TYPE called TYPEArray (for example int_array, usize_array, etc).
 * By importing this file you get the declaration of
 * TYPEArray for all the core types defined in coretype.h.
 * @see ARRAY
 */
#define DECL_ARRAY(TYPE, ...)\
	typedef struct {\
		TYPE *arr;\
		usize len;\
	} ARRAY(TYPE);

XX_CORETYPES(DECL_ARRAY)

/**
 * @brief Creates a new TYPEArray object with a given length
 * on the stack.
 * The encapsulated array is allocated on the heap and is left
 * uninitialized.
 * This array can be freed with ::ARRAY_FREE.
 * @see See example in ::ARRAY
 */
#define ARRAY_NEW(TYPE, LEN) ((ARRAY(TYPE)){.len=(LEN), .arr=(TYPE*)malloc((LEN)*sizeof(TYPE))})

/**
 * @brief Encapsulates an existing array of a given TYPE with a given length
 * in a TYPEArray object on the stack.
 *
 * Example
 * ```
 * int *src = calloc(5, sizeof(int));
 * ARRAY(int) a = ARRAY_NEW_FROM_ARR(int, 5, src);
 * ```
 */
#define ARRAY_NEW_FROM_ARR(TYPE, LEN, SRC) ((ARRAY(TYPE)){.len=(LEN), .arr=((TYPE*)(SRC))})

/**
 * @brief Frees the encapsulated array of a TYPEArray object.
 * @warning This is a shallow free. It does not free the contents of the
 * encapsulated array if it contains pointers to other objects. For
 * the proper disposal of structured object collections, use vectors
 * defined in vec.h and the destructor infrastructure defined in new.h.*
 */
#define ARRAY_FREE(A) free((A).arr)

#endif

