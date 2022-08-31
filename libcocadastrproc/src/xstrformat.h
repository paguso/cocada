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

#ifndef XSTRFORMAT_H
#define XSTRFORMAT_H

#include <stdio.h>

#include "xchar.h"
#include "xstr.h"
#include "format.h"

/**
 * @file xstrformat.h
 * @author Paulo Fonseca
 * @brief Extended string (xstr) formatter
 * @see format.h
 * 
 * This formatter implements the format trait to provide a interface
 * for printing an xstr.
 */

/**
 * @brief Formatter type.
 * 
 */
typedef struct _xstrformat xstrformat;


/**
 * @brief Creates a formatter for @p src that formats individual
 * xchars according to the default format specification XCHAR_FMT 
 * @see xchar.h
 */
xstrformat *xstrformat_new(const xstr *src);


/**
 * @brief Creates a formatter for @p src that formats individual xchars 
 * as the corresponding ASCII chars.
 * @warning Assumes xstr_sizeof_char(src) == sizeof(char)
 */
xstrformat *xstrformat_new_ascii(const xstr *src);


/**
 * @brief Destructor
 */
void xstrformat_free(xstrformat *self);


DECL_TRAIT(xstrformat, format)

#endif