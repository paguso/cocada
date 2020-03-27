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

#ifndef STRSTREAM_H
#define STRSTREAM_H

#include <stddef.h>

#include "new.h"
#include "xchar.h"
#include "xstring.h"

/**
 * @file strstream.h
 * @author Paulo Fonseca
 *
 * @brief String stream.
 *
 * @deprecated Use implementations of the strread interface.
 */


/**
 * String stream type
 */
typedef struct _strstream strstream;


/**
 * @brief Opens a stream for a in-memory source string.
 * @param str The source string.
 * @param slen The source string length.
 */
strstream *strstream_open_str(char *str, size_t slen);


/**
 * @brief Opens a stream for a in-memory source xstring.
 * @param str The source xstring.
 */
strstream *strstream_open_xstr(xstring *xstr);


/**
 * @brief Opens a stream for a source text file.
 */
strstream *strstream_open_file(char *filename);


/**
 * @brief Opens a stream for a source xstring text file.
 */
strstream *strstream_open_xfile(char *filename, size_t bytes_per_char);


/**
 * @brief Resets the stream, i.e. moves cursor to initial position.
 */
void strstream_reset(strstream *sst);


/**
 * @brief Tests whether a stream has reached its end.
 */
bool strstream_end(strstream *sst);


/**
 * @brief Reads the next char from a stream.
 * @returns The next character as an int, or EOF if the stream has
 *          reached its end.
 *
 * Example of usage:
 * @code
 * strstream *fsst = strstream_open_file(filename);
 * for (int c; (c=strstream_getc(fsst)) != EOF;)
 *     printf ("Read c=%c\n", (char)c);
 * strstream_close(fsst);
 * @endcode
 */
xchar_t strstream_getc(strstream *sst);


/**
 * @brief Attempts to read the next @p n chars into the string *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
size_t strstream_reads(strstream *sst, char *dest, size_t n);


/**
 * @brief Attempts to read the next @p n chars into the string *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
size_t strstream_reads_until(strstream *sst, char *dest, char delim);


/**
 * @brief Attempts to read the next @p n xchars into the xstring *dest.
 *        Less than @p n xchars can be read if the stream reaches its end.
 * @returns The number of xchars actually read.
 */
size_t strstream_readxs(strstream *sst, xstring *xstr, size_t n);


/**
 * @brief Attempts to read the next @p n xchars into the xstring *dest.
 *        Less than @p n xchars can be read if the stream reaches its end.
 * @returns The number of xchars actually read.
 */
size_t strstream_readxs_until(strstream *sst, xstring *xstr, xchar_t delim);


/**
 * @brief Closes the stream and disposes the stream object.
 */
void strstream_close(strstream *sst);


/**
 * @brief Returns the size of the xchar used in the stream.
 */
size_t strstream_sizeof_char(strstream *sst);

#endif