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

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
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
 * byte_t               | byte_t         | coretype.h
 * void *               | rawptr         | coretype.h
 * char *               | cstr           | coretype.h
 *
 */


/**
 * @brief A required unsigned byte type.
 *
 * The C11 standard  IEC 9899:2011 defines a "byte" as an addressable unit
 * of data storage large enough to hold any member of the basic character
 * set of the execution environment.
 *
 * It also defines a char as "single-byte" character and so sizeof(char)
 * should always return 1.
 *
 * Moreover, the standard library <limits.h> defines CHAR_BIT to be the
 * number of bits for smallest object that is not a bit-field ("byte")
 * and specifies a minimum size of 8 (eight).
 *
 * Although a byte may be composed of more that eight bits,
 * ISO Norm IEC 80000-13:2008 (item 13.9 c) suggests that the word "byte"
 * be defined as a synonymm of octet, i.e. a sequence of eight bits.
 *
 * A byte_t type is therefore defined as an alias for unsigned char.
 * A BYTESIZE constant is defined as a CHAR_BIT synonym, and
 * accordingly a maximum value constant BYTE_MAX is defined as
 * UCHAR_MAX synonym.
 */
typedef unsigned char byte_t;

#define BYTESIZE CHAR_BIT
/*
 * This library is not strictly dependent on a byte being an octet.
 * However it has only been tested on such conditions, hence this safeguard.
 */
#if BYTESIZE!=8
#error Error: this code requires an 8-bit byte_t type
#endif
#define BYTE_MAX UCHAR_MAX



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
typedef void               *rawptr;
#endif


#define SIGNED_char char
#define SIGNED_uchar char
#define SIGNED_short short
#define SIGNED_ushort short
#define SIGNED_int int
#define SIGNED_uint int
#define SIGNED_long long
#define SIGNED_ulong long
#define SIGNED_llong llong
#define SIGNED_ullong llong
#define SIGNED_int8_t int8_t
#define SIGNED_uint8_t int8_t
#define SIGNED_int16_t int16_t
#define SIGNED_uint16_t int16_t
#define SIGNED_int32_t int32_t
#define SIGNED_uint32_t int32_t
#define SIGNED_int64_t int64_t
#define SIGNED_uint64_t int64_t
#define SIGNED_bool int8_t
#define SIGNED__Bool int8_t
#define SIGNED_byte_t int8_t
#define SIGNED_size_t llong
#define UNSIGNED_char uchar
#define UNSIGNED_uchar uchar
#define UNSIGNED_short ushort
#define UNSIGNED_ushort ushort
#define UNSIGNED_int uint
#define UNSIGNED_uint uint
#define UNSIGNED_long ulong
#define UNSIGNED_ulong ulong
#define UNSIGNED_llong ullong
#define UNSIGNED_ullong ullong
#define UNSIGNED_int8_t uint8_t
#define UNSIGNED_uint8_t uint8_t
#define UNSIGNED_int16_t uint16_t
#define UNSIGNED_uint16_t uint16_t
#define UNSIGNED_int32_t uint32_t
#define UNSIGNED_uint32_t uint32_t
#define UNSIGNED_int64_t uint64_t
#define UNSIGNED_uint64_t uint64_t
#define UNSIGNED_bool uint8_t
#define UNSIGNED__Bool uint8_t
#define UNSIGNED_byte_t uint8_t
#define UNSIGNED_size_t size_t

#define SIGNED(TYPE) SIGNED_##TYPE
#define UNSIGNED(TYPE) UNSIGNED_##TYPE


/**
 * NULL pointer constant.
 */
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef cstr
typedef char               *cstr;
#endif


/* Int types widths */

#if SHRT_MAX==0x7fff
#define SHRT_BYTES 2
#define SHRT_BITS 16
#elif SHRT_MAX==0x7fffffff
#define SHRT_BYTES 4
#define SHRT_BITS 32
#elif SHRT_MAX==0x7fffffffffffffff
#define SHRT_BYTES 8
#define SHRT_BITS 64
#else
#error "Unknown/invalid short size"
#endif
#define USHRT_BYTES SHRT_BYTES
#define USHRT_BITS SHRT_BITS

#if INT_MAX==0x7fff
#define INT_BYTES 2
#define INT_BITS 16
#elif INT_MAX==0x7fffffff
#define INT_BYTES 4
#define INT_BITS 32
#elif INT_MAX==0x7fffffffffffffff
#define INT_BYTES 8
#define INT_BITS 64
#else
#error "Unknown/invalid int size"
#endif
#define UINT_BYTES INT_BYTES
#define UINT_BITS INT_BITS

#if LONG_MAX==0x7fffffff
#define LONG_BYTES 4
#define LONG_BITS 32
#elif LONG_MAX==0x7fffffffffffffff
#define LONG_BYTES 8
#define LONG_BITS 64
#else
#error "Unknown/invalid long size"
#endif
#define ULONG_BYTES LONG_BYTES
#define ULONG_BITS LONG_BITS


#if LLONG_MAX==0x7fffffffffffffff
#define LLONG_BYTES 8
#define LLONG_BITS 64
#else
#error "Unknown/invalid long long size"
#endif
#define ULLONG_BYTES LLONG_BYTES
#define ULLONG_BITS LLONG_BITS


#define XX_UNSIGNED_INT(XX, ...) \
	XX(uchar, __VA_ARGS__) \
	XX(ushort, __VA_ARGS__) \
	XX(uint, __VA_ARGS__) \
	XX(ulong, __VA_ARGS__) \
	XX(ullong, __VA_ARGS__) \
	XX(size_t, __VA_ARGS__) \
	XX(uint8_t, __VA_ARGS__) \
	XX(uint16_t, __VA_ARGS__) \
	XX(uint32_t, __VA_ARGS__) \
	XX(uint64_t, __VA_ARGS__) \
	XX(byte_t, __VA_ARGS__)

#define XX_SIGNED_INT(XX, ...) \
	XX(char, __VA_ARGS__) \
	XX(short, __VA_ARGS__) \
	XX(int, __VA_ARGS__) \
	XX(long, __VA_ARGS__) \
	XX(llong, __VA_ARGS__) \
	XX(bool, __VA_ARGS__) \
	XX(int8_t, __VA_ARGS__) \
	XX(int16_t, __VA_ARGS__) \
	XX(int32_t, __VA_ARGS__) \
	XX(int64_t, __VA_ARGS__)

#define XX_INTS( XX, ...) \
	XX_UNSIGNED_INT(XX, __VA_ARGS__) \
	XX_SIGNED_INT(XX, __VA_ARGS__)

#define XX_FLOATS( XX, ...) \
	XX(float, __VA_ARGS__) \
	XX(double, __VA_ARGS__) \
	XX(ldouble, __VA_ARGS__)

#define XX_PRIMITIVES( XX, ...) \
	XX_INTS(XX, __VA_ARGS__) \
	XX_FLOATS(XX, __VA_ARGS__)

#define XX_PTRS( XX, ...) \
	XX(rawptr, __VA_ARGS__) \
	XX(cstr , __VA_ARGS__)

#define XX_CORETYPES(XX,...) \
	XX_PRIMITIVES(XX, __VA_ARGS__) \
	XX_PTRS(XX, __VA_ARGS__)

#endif //CORETYPE_H