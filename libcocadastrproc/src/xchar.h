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

#ifndef XCHAR_H
#define XCHAR_H

#include <limits.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>

/**
 * @file xchar.h
 * @author Paulo Fonseca
 * @brief eXtended char
 *
 * According to the C standard, the <b>char</b> type is a standard signed
 * integer type, "large enough to store any member of the basic execution
 * character set". Its is often implemented as a single byte (octet) and
 * thus capable of representing 255 distinct symbols, frequently associated
 * with the ASCII encoding, although no such connexion is implied by the
 * standard.
 *
 * The eXtended char type (xchar_t) defined herein is simply a renaming
 * of a larger standard integer type, to be used whenever larger alphabets
 * are required. **This type is not intended for I/O. No connexion with a
 * particular character set or encoding is required or implied, neither
 * the size is related to the current locale.**
 *
 * The XCHAR_BYTES constant macro defines the size of the xchar_t type
 * in bytes. Allowed values are 1, 2, 4, and 8. When not defined, the
 * default value of 4 (32 bits) is assumed. When XCHAR_BYTES is set
 * to $N$, the  xchar_t is a typedef renaming of the standard int`N`_t
 * type. In addition to that, two more constants are defined. XCHAR_MAX
 * defines the maximum numerical value of xchar_t, and XCHAR_FMT is a
 * format string used for printing the *numerical value* of a xchar_t
 * with printf family functions.
 */


#ifndef XCHAR_BYTES

#warning "Undefined XCHAR_BYTES. Setting to default = 4 (32 bits)"
#define XCHAR_BYTES 4
typedef int32_t   xchar_t;
#define XCHAR_MAX INT32_MAX
#define XCHAR_FMT PRId32
typedef int32_t   xchar_wt; // xchar wrapper type

#elif XCHAR_BYTES == 1

typedef int8_t   xchar_t;
#define XCHAR_MAX INT8_MAX
#define XCHAR_FMT PRId8
typedef int32_t   xchar_wt; // xchar wrapper type

#elif XCHAR_BYTES == 2

typedef int16_t  xchar_t;
#define XCHAR_MAX INT16_MAX
#define XCHAR_FMT PRId16
typedef int32_t   xchar_wt; // xchar wrapper type

#elif XCHAR_BYTES == 4

typedef int32_t  xchar_t;
#define XCHAR_MAX INT32_MAX
#define XCHAR_FMT PRId32
typedef int32_t   xchar_wt; // xchar wrapper type

#elif XCHAR_BYTES == 8

typedef int64_t  xchar_t;
#define XCHAR_MAX INT64_MAX
#define XCHAR_FMT PRId64
typedef int64_t   xchar_wt; // xchar wrapper type

#else

#warning "Invalid XCHAR_BYTES. Allowed values are 1,2,4, and 8. Setting to default = 4 (32 bits)"
#undef XCHAR_BYTES
#define XCHAR_BYTES 4

typedef int32_t  xchar_t;
#define XCHAR_MAX INT32_MAX
#define XCHAR_FMT PRId32

#endif


/**
 * @deprecated Should be removed. No direct connexion with I/O assumed.
 */
#define XEOF ((xchar_wt)EOF)


#endif // XCHAR_BYTES
