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

/**
 * @file arrays.h
 * @author Paulo Fonseca
 *
 * @brief Array utility macros and functions.
 *
 * # Size-annotated arrays
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
 * the useful area is initialised with 0´s, and a pointer to the start
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
	for(size_t _i=(FROM), _to=(TO); _i<_to; (ARR)[_i++]=(EXPR))


/**
 * @brief Copies N elements from an array SRC from position =FROMSRC
 *        into an array DEST from position =FROMDEST.
 */
#define ARR_COPY( DEST, FROMDEST, SRC, FROMSRC, N )\
	for(size_t _i=0, _n=(N), _fs=(FROMSRC), _fd=(FROMDEST); _i<_n; _i++)\
		(DEST)[_fd+_i]=(SRC)[_fs+_i]


/**
 * @brief Prints the array @p ARR from position @p FROM to position @p TO-1
 *        using @p LABEL as label, displaying @p ELTSPERLINE elements per line,
 *        separated by @p SEPARATOR and using the printf format string @p FORMAT.
 *        @p LEFT_MARGIN is a string that is printed at the beginning of each line.
 */
#define ARR_FPRINT(STREAM, ARR, FROM, TO, ELTSPERLINE, LABEL, FORMAT, SEPARATOR, LEFT_MARGIN)\
	{ fprintf(STREAM, "%s"LABEL"[%zu:%zu] =", LEFT_MARGIN, ((size_t)(FROM)), ((size_t)(TO)));\
		for (size_t __i=FROM, __el=(ELTSPERLINE); __i<TO; __i++) {\
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
	for (size_t __i=0; __i < (ROWS); __i++){\
		ID[__i] = __ptr##ID;\
		__ptr##ID += (COLS);\
	}

/**
 * @brief Creates a new matrix of a given TYPE with @p ROWS rows and @p COLS columns 
 * and initializes it to 0.
 */
#define NEW_MATRIX_0(ID, TYPE, ROWS, COLS)\
	size_t __len##ID =  ( (ROWS) * sizeof(TYPE*) ) + ((ROWS) * (COLS) * sizeof(TYPE) );\
	TYPE** ID = (TYPE**) malloc(__len##ID);\
	memset(ID, 0x0, __len##ID);\
	TYPE* __ptr##ID = (TYPE *) (ID + (ROWS));\
	for (size_t __i=0; __i < (ROWS); __i++){\
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
	for (size_t __i=0, __li = (ROWS); __i < __li; __i++) \
		for (size_t __j=0, __lj = (COLS); __j < __lj; __j++) \
			ID[__i][__j] = (EXPR);\


/**
 * @brief Expands into a type name for an array with elements of a 
 * given TYPE called TYPE_array (for example int_array, size_t_array, etc).
 * A TYPE_array encapsulates an ordinary C array of TYPE and its 
 * length in a struct. This is convenient because we can pass and 
 * receive the array and its length to and from functions as a single 
 * argument. Differently from vectors and other generic arrays, the type 
 * of the elements makes its use more convenient, without the need for 
 * casts and other type conversions.
 * 
 * Prior to being used, a TYPE_array must be declared with the macro
 * ::DECL_ARRAY. By importing this file you get the declaration of
 * TYPE_array for all the core types defined in coretype.h.
 * 
 * A TYPE_array object is primarily meant to be created on the stack,
 * although the encapsulated array will typically be allocated on 
 * the heap. Thus we can pass and receive a TYPE_array by value. 
 * 
 * Example:
 * ```
 * void print_int_array(ARRAY(int) a){
 * 	for (size_t i=0; i<a.len; i++){
 * 		printf("%d ", a.arr[i]);
 * 	}
 * }
 * 
 * int main() {
 * 	ARRAY(int) a = ARRAY_NEW(int, 10);
 * 	for (size_t i=0; i<a.len; i++){
 * 		a.arr[i] = i;
 * 	}
 * 	print_int_array(a);
 * 	ARRAY_FREE(a);
 * }
 * ```
 */
#define ARRAY(TYPE) TYPE##_array

/**
 * @brief Declares a type name for an array with elements of a
 * given TYPE called TYPE_array (for example int_array, size_t_array, etc).
 * By importing this file you get the declaration of
 * TYPE_array for all the core types defined in coretype.h.
 * @see ARRAY
 */
#define DECL_ARRAY(TYPE, ...)\
	typedef struct {\
		TYPE *arr;\
		size_t len;\
	} ARRAY(TYPE);

XX_CORETYPES(DECL_ARRAY)

/**
 * @brief Creates a new TYPE_array object with a given length
 * on the stack.
 * The encapsulated array is allocated on the heap and is left 
 * uninitialized.
 * This array can be freed with ::ARRAY_FREE.
 * @see See example in ::ARRAY
 */
#define ARRAY_NEW(TYPE, LEN) ((ARRAY(TYPE)){.len=(LEN), .arr=(TYPE*)malloc((LEN)*sizeof(TYPE))})

/**
 * @brief Encapsulates an existing array of a given TYPE with a given length
 * in a TYPE_array object on the stack.
 * 
 * Example
 * ```
 * int *src = calloc(5, sizeof(int));
 * ARRAY(int) a = ARRAY_NEW_FROM_ARR(int, 5, src);
 * ```
 */
#define ARRAY_NEW_FROM_ARR(TYPE, LEN, SRC) ((ARRAY(TYPE)){.len=(LEN), .arr=((TYPE*)(SRC))})

/**
 * @brief Frees the encapsulated array of a TYPE_array object.
 * @warning This is a shallow free. It does not free the contents of the 
 * encapsulated array if it contains pointers to other objects. For
 * the proper disposal of structured object collections, use vectors 
 * defined in vec.h and the destructor infrastructure defined in new.h.*
 */
#define ARRAY_FREE(A) free((A).arr)

#endif