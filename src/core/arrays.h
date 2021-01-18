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

#include <stdlib.h>
#include <string.h>

#include "coretype.h"

/**
 * @file arrays.h
 * @author Paulo Fonseca
 *
 * @brief Assorted array utility macros and functions.
 */


void *arr_new(size_t typesize, size_t len);

void *arr_realloc(void *arr, size_t size);

size_t arr_size(void *arr);

void arr_free(void *arr);


#define ARR_NEW_DECL(TYPE, ...)\
	TYPE *arr_##TYPE##_new(size_t len);
XX_CORETYPES(ARR_NEW_DECL)

#define ARR_REALLOC_DECL(TYPE, ...)\
	TYPE *arr_##TYPE##_realloc(TYPE *arr, size_t len);
XX_CORETYPES(ARR_REALLOC_DECL)

#define ARR_LEN_DECL(TYPE, ...)\
	size_t arr_##TYPE##_len(TYPE *arr);
XX_CORETYPES(ARR_LEN_DECL)




/**
 * @brief Allocates a new array of N elements of a given TYPE.
 */
#define NEW_ARR( TYPE, N ) ((N>0)?((TYPE*)(malloc((N)*sizeof(TYPE)))):NULL)


/**
 * @brief Allocates a new array of N elements of a given TYPE all set to 0.
 */
#define NEW_ARR_0( TYPE, N ) ((N>0)?((TYPE*)(calloc((N),sizeof(TYPE)))):NULL)


/**
 * @brief Sets the positions FROM to TO-1 of the array ARR with the expression EXPR.
 * @warning The expression EXPR is evaluated for every element to be set.
 *          If EXPR is an object creator call `type_new()` then every position
 *          will be set to a different instance.
 */
#define FILL_ARR( ARR, FROM, TO, EXPR ) \
	for(size_t _i=(FROM), _to=(TO); _i<_to; (ARR)[_i++]=(EXPR))


/**
 * @brief Copies N elements from an array SRC from position =FROMSRC
 *        into an array DEST from position =FROMDEST.
 */
#define COPY_ARR( DEST, FROMDEST, SRC, FROMSRC, N )\
	for(size_t _i=0, _n=(N), _fs=(FROMSRC), _fd=(FROMDEST); _i<_n; _i++)\
		(DEST)[_fd+_i]=(SRC)[_fs+_i]


/**
 * @brief Prints the array ARR from positio FROM to position TO-1
 *        using NAME as label, displaying ELTSPERLINE elements per line,
 *        and using the printf format string FORMAT.
 */
#define PRINT_ARR( ARR, NAME, FORMAT, FROM, TO , ELTSPERLINE )\
	{ printf(#NAME"[%zu:%zu] =",((size_t)(FROM)), ((size_t)(TO)));\
		for (size_t __i=FROM, __el=(ELTSPERLINE); __i<TO; __i++) {\
			if(!((__i-FROM)%__el)) printf("\n%4zu: ",__i);\
			printf(#FORMAT" " , ARR[__i]);}\
		printf("\n");}


/**
 * @brief Prints the array ARR from positio FROM to position TO-1
 *        using NAME as label, displaying ELTSPERLINE elements per line,
 *        and using the printf format string FORMAT.
 */
#define FPRINT_ARR(STREAM, ARR, NAME, FORMAT, FROM, TO , ELTSPERLINE )\
	{ fprintf(STREAM, #NAME"[%zu:%zu] =",((size_t)(FROM)), ((size_t)(TO)));\
		for (size_t __i=FROM, __el=(ELTSPERLINE); __i<TO; __i++) {\
			if(!((__i-FROM)%__el)) fprintf(STREAM, "\n%4zu: ",__i);\
			fprintf(STREAM, #FORMAT" " , ARR[__i]);}\
		fprintf(STREAM, "\n");}


#define FOREACH_IN_ARR( ELT, ELT_TYPE, ARR, ARR_LEN ) \
	for (ELT_TYPE* __arr = (ELT_TYPE *)(ARR); __arr; __arr = NULL) \
		for (size_t __i = 0, __l = (ARR_LEN); __i < __l; __i = __l) \
			for (ELT_TYPE ELT = __arr[__i]; __i < __l; ELT = ((++__i) < __l) ? __arr[__i] : ELT )


#define NEW_MATRIX(ID, TYPE, ROWS, COLS)\
	TYPE** ID = (TYPE**) malloc( ( (ROWS) * sizeof(TYPE*) ) + ((ROWS) * (COLS) * sizeof(TYPE)));\
	TYPE* __ptr##ID = (TYPE*) (ID + ROWS);\
	for (size_t __i=0; __i < (ROWS); __i++)\
		ID[__i] = __ptr##ID + (__i * COLS);


#define NEW_MATRIX_0(ID, TYPE, ROWS, COLS)\
	size_t __len##ID =  ( (ROWS) * sizeof(TYPE*) ) + ((ROWS) * (COLS) * sizeof(TYPE) );\
	TYPE** ID = (TYPE**) malloc(__len##ID);\
	memset(ID, 0x0, __len##ID);\
	TYPE* __ptr##ID = (TYPE*) (ID + ROWS);\
	for (size_t __i=0; __i < (ROWS); __i++)\
		ID[__i] = __ptr##ID + (__i * COLS);


#define FILL_MATRIX(ID, ROWS, COLS, EXPR)\
	for (size_t __i=0, __li = (ROWS); __i < __li; __i++) \
		for (size_t __j=0, __lj = (COLS); __j < __lj; __j++) \
			ID[__i][__j] = (EXPR);\



#endif