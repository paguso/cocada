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

#ifndef STRFILEREADER_H
#define STRFILEREADER_H

/**
 * @file strfilereader.h
 * @brief String file reader. Implements the strread trait.
 * @author Paulo Fonseca
 * @see strread.h
 *
 * This string reader is backed by a text FILE, and therefore is
 * likely to be a *buffered* reader.
 */

#include "trait.h"


/**
 * @brief String file reader
 */
typedef struct _strfilereader strfilereader;


DECL_TRAIT(strfilereader, strread)


/**
 * @brief Opens a character file for reading.
 */
strfilereader *strfilereader_open(char *filename);


/**
 * @brief Closes the source file and destroys the reader.
 */
void strfilereader_close(strfilereader *self);



#endif