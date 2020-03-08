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

#include "bitsandbytes.h"
#include "new.h"
#include "cstringutil.h"

/**
 * @file arrayutil.h
 * @author Paulo Fonseca
 *
 * @brief Assorted array utility macros and functions.
 */

/**
 * @brief Allocates a new array of N elements of a given TYPE.
 */
#define NEW_ARR( TYPE, N ) ((N>0)?((TYPE*)(malloc((N)*sizeof(TYPE)))):NULL)


/**
 * @brief Allocates a new array of N elements of a given TYPE.
 */
#define NEW_ARR_SZOF( TYPE, N , SZOF) ((N>0)?((TYPE*)(malloc((N)*(SZOF)))):NULL)


/**
 * @brief Fills an array ARR from position =FROM to position <TO with value VAL.
 */
#define FILL_ARR( ARR, FROM, TO, VAL ) \
    for(size_t _i=(FROM), _to=(TO); _i<_to; (ARR)[_i++]=(VAL))

        
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


#endif