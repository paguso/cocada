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
#include "xstr.h"

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
typedef struct _StrStream StrStream;


/**
 * @brief Opens a stream for a in-memory source string.
 * @param str The source string.
 * @param slen The source string length.
 */
StrStream *strstream_open_str(char *str, usize slen);


/**
 * @brief Opens a stream for a in-memory source xstr.
 * @param str The source xstr.
 */
StrStream *strstream_open_xstr(xstr *xstr);


/**
 * @brief Opens a stream for a source text file.
 */
StrStream *strstream_open_file(char *filename);


/**
 * @brief Opens a stream for a source xstr text file.
 */
StrStream *strstream_open_xfile(char *filename, usize bytes_per_char);


/**
 * @brief Resets the stream, i.e. moves cursor to initial position.
 */
void strstream_reset(StrStream *sst);


/**
 * @brief Tests whether a stream has reached its end.
 */
bool strstream_end(StrStream *sst);


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
xchar strstream_getc(StrStream *sst);


/**
 * @brief Attempts to read the next @p n chars into the string *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
usize strstream_reads(StrStream *sst, char *dest, usize n);


/**
 * @brief Attempts to read the next @p n chars into the string *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
usize strstream_reads_until(StrStream *sst, char *dest, char delim);


/**
 * @brief Attempts to read the next @p n xchars into the xstr *dest.
 *        Less than @p n xchars can be read if the stream reaches its end.
 * @returns The number of xchars actually read.
 */
usize strstream_readxs(StrStream *sst, xstr *xstr, usize n);


/**
 * @brief Attempts to read the next @p n xchars into the xstr *dest.
 *        Less than @p n xchars can be read if the stream reaches its end.
 * @returns The number of xchars actually read.
 */
usize strstream_readxs_until(StrStream *sst, xstr *xstr, xchar delim);


/**
 * @brief Closes the stream and disposes the stream object.
 */
void strstream_close(StrStream *sst);


/**
 * @brief Returns the size of the xchar used in the stream.
 */
usize strstream_sizeof_char(StrStream *sst);

#endif
