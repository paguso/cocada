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
#include <stdint.h>

/**
 * @file xchar
 * @author Paulo Fonseca
 * @brief eXtended char
 * According to the C standard, the <b>char</b> type is a standard signed
 * integer type, "large enough to store any member of the basic execution
 * character set". Its is often implemented as a single byte (octet) and
 * thus capable of representing 255 distinct symbols, frequently associated
 * with the ASCII encoding, although no such connexion is implied by the
 * standard.
 * The eXtended char type (xchar_t) defined herein is simply a renaming
 * of a larger standard integer type, to be used whenever larger alphabets are
 * required. No connexion with a particular character set or encoding is
 * required or implied, neither the size is related to the current locale.
 */

/**
 *@brief eXtended char type.
 */
typedef int xchar_t;


/**
 *@brief The maximum value represented by a xchar_t object
 */
#define XCHAR_MAX INT_MAX


#if XCHAR_MAX==INT8_MAX
#define XCHAR_BYTESIZE 1
#elif XCHAR_MAX==INT16_MAX
#define XCHAR_BYTESIZE 2
#elif XCHAR_MAX==INT32_MAX
#define XCHAR_BYTESIZE 4
#elif XCHAR_MAX==INT64_MAX
#define XCHAR_BYTESIZE 8
#else
#error "Unable to identify extended char (xchar) bytesize."
#endif

/**
 *@brief EOF symbol to be used in I/O operations
 */
#define XEOF EOF

/**
 *@brief xchat_t format specifier used for I/O operations like printf or scanf
 */
#define XCHAR_FMT "%d"


/**
 * @brief Reverts the byte order of a xchar_t object in place.
 */
void xchar_flip_bytes(xchar_t *c);


#endif
