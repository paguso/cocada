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
 * strfilereader *sfr = strfilereader_new_from_path(filename);
 * strread *r = strfilereader_as_strread(sfr);
 * for (int c; (c=strread_getc(r)) != EOF;)
 *     printf ("Read c=%c\n", (char)c);
 * strfilereader_free(sfr);
 * ```
 */

#include <stdio.h>

#include "trait.h"
#include "strread.h"


/**
 * @brief String file reader
 */
typedef struct _strfilereader strfilereader;


DECL_TRAIT(strfilereader, strread)


/**
 * @brief Creates a new reader attached to an already open character input stream.
 * The reader assumes no ownership of the stream, which must be closed separately.
 * @warning The stream is supposed to be a proper text FILE open for reading.
 * No checks performed.
 */
strfilereader *strfilereader_new(FILE *stream);


/**
 * @brief Creates a new reader attached to a character input stream from its @p path.
 * A new input stream is created and opened for reading.
 * @returns NULL if the FILE at specified @p path cannot be open in "r" mode.
 */
strfilereader *strfilereader_new_from_path(const char *path);


/**
 * @brief Destructor. If the reader was created with strfilereader_new(),
 * the underlying stream is not closed. If the reader was created with the
 * strfilereader_new_from_path() constructor, the stream is closed.
 */
void strfilereader_free(strfilereader *self);



#endif