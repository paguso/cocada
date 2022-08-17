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

/**
 * @file strreader.h
 * @author Paulo Fonseca
 * @brief String reader. Implements the strread trait over an in-memory char string.
 */


#ifndef STRREADER_H
#define STRREADER_H

#include <stddef.h>

#include "strread.h"
#include "trait.h"


typedef struct _strreader strreader;

/**
 * @brief "Opens" the in-memory source string @p src of length @p len as a stream reader.
 */
strreader *strreader_new(char *src, size_t len);


/**
 * @brief Destructor.
 * @warning This *does not* deallocate de source string.
 */
void strreader_free(strreader *rdr);


DECL_TRAIT(strreader, strread)


#endif