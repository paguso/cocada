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

#ifndef CORETYPE_H
#define CORETYPE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>


/**
 * @file coretype.h
 * @brief One-word renamings (typedefs) of commonly-used types
 * @author Paulo Fonseca
 *
 * The aim of this header is to have a simple one-word name
 * for the commonly used types, including the C-standard types,
 * fixed-size integers, and pointers.
 *
 * Standard type        | One-word name  | Defined in
 * ---------------------|----------------|--------------------
 * char                 | char           | C-standard
 * unsigned char        | uchar          | coretype.h
 * short                | short          | C-standard
 * unsigned short       | ushort         | coretype.h
 * int                  | int            | C-standard
 * unsigned int         | uint           | coretype.h
 * long                 | long           | C-standard
 * unsigned long        | ulong          | coretype.h
 * long long            | llong          | coretype.h
 * unsigned long long   | ullong         | coretype.h
 * float                | float          | C-standard
 * double               | double         | C-standard
 * long double          | ldouble        | coretype.h
 *
 * Library type         | One-word name  | Defined in
 * ---------------------|----------------|--------------------
 * _Bool                | bool           | stdbool.h
 * size_t               | size_t         | stddef.h
 * int8_t               | int8_t         | stdint.h
 * uint8_t              | uint8_t        | stdint.h
 * int16_t              | int16_t        | stdint.h
 * uint16_t             | uint16_t       | stdint.h
 * int32_t              | int32_t        | stdint.h
 * uint32_t             | uint32_t       | stdint.h
 * int64_t              | int64_t        | stdint.h
 * uint64_t             | uint64_t       | stdint.h
 *
 * Custom types         | One-word name  | Defined in
 * ---------------------|----------------|--------------------
 * byte_t               | byte_t         | bitbyte.h
 * void *               | rawptr         | coretype.h
 * char *               | cstr           | coretype.h
 *
 */


#ifndef BYTE_T
#define BYTE_T

typedef unsigned char       byte_t;

#define BYTESIZE CHAR_BIT

#if BYTESIZE!=8
#error Error: this code requires an 8-bit byte_t type
#endif

#define BYTE_MAX UCHAR_MAX
#endif //BYTE_T 


#ifndef uchar
typedef unsigned char       uchar;
#endif

#ifndef ushort
typedef unsigned short      ushort;
#endif

#ifndef uint
typedef unsigned int        uint;
#endif

#ifndef ulong
typedef unsigned long       ulong;
#endif

#ifndef llong
typedef long long           llong;
#endif

#ifndef ullong
typedef unsigned long long  ullong;
#endif

#ifndef ldouble
typedef long double         ldouble;
#endif

#ifndef rawptr
typedef void *              rawptr;
#endif

/**
 * NULL pointer constant.
 */
#ifndef NULL
#define NULL ((void *)0)
#endif


#ifndef cstr
typedef char *              cstr;
#endif


#endif //CORETYPE_H