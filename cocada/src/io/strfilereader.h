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
 * @author Paulo Fonseca
 * @brief String file reader. Implements the strread trait for reading char
 * strings from a FILE stream.
 * @see strread.h
 *
 * # Example
 *
 * ```C
 * strfilereader *sfr = strfilereader_open(filename);
 * strread *ftrait = strfilereader_as_strread(sfr);
 * for (int c; (c=strread_getc(ftrait)) != EOF;)
 *     printf ("Read c=%c\n", (char)c);
 * strfilereader_close(sfr);
 * ```
 */

#include <stdio.h>

#include "trait.h"


/**
 * @brief String file reader
 */
typedef struct _strfilereader strfilereader;


DECL_TRAIT(strfilereader, strread)


/**
 * @brief Opens a character file for reading from its @p path.
 * @returns NULL if the FILE at specified @p path cannot be open in "r" mode.
 */
strfilereader *strfilereader_open_path(const char *path);


/**
 * @brief Opens a character FILE for reading.
 * @warning stream is supposed to be a proper text FILE open for reading.
 * No checks performed.
 */
strfilereader *strfilereader_open(FILE *stream);


/**
 * @brief Closes the source file and destroys the reader.
 */
void strfilereader_close(strfilereader *self);



#endif