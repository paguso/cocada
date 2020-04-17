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

#ifndef STRREAD_H
#define STRREAD_H

/**
 * @file strread.h
 * @author Paulo Fonseca
 * @brief String Reader Trait
 */

#include <stdbool.h>
#include <stddef.h>


typedef struct _strread strread;

typedef struct {
	void    (*reset)(void *self);
	char	(*getc)(void *self);
	size_t  (*read_str)(void *self, char *dest, size_t n);
	size_t  (*read_str_until)(void *self, char *dest, char delim);
	//size_t  (*read_xstr)(void *self, xstring *xstr, size_t n);
	//size_t  (*read_xstr_until)(void *self, xstring *xstr, xchar_t delim);
	//void    (*close)(void *self);
	size_t  (*sizeof_char)(void *self);
}  strread_vt;


struct _strread {
	void *impltor;
	strread_vt *vtbl;
};


strread_vt strread_vt_new();


/**
 * @brief Resets the stream, i.e. moves cursor to initial position.
 */
void strread_reset(strread *trait);


/**
 * @brief Reads the next char from a stream.
 * @returns The next character as an int, or EOF if the stream has
 *          reached its end.
 *
 * Example of usage:
 * @code
 * strread *ftrait = strread_open_file(filename);
 * for (int c; (c=strread_getc(ftrait)) != EOF;)
 *     printf ("Read c=%c\n", (char)c);
 * strread_close(ftrait);
 * @endcode
 */
char strread_getc(strread *trait);


/**
 * @brief Attempts to read the next @p n chars into the string *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
size_t strread_read_str(strread *trait, char *dest, size_t n);


/**
 * @brief Attempts to read the next @p n chars into the string *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
size_t strread_read_str_until(strread *trait, char *dest, char delim);


/**
 * @brief Attempts to read the next @p n xchars into the xstring *dest.
 *        Less than @p n xchars can be read if the stream reaches its end.
 * @returns The number of xchars actually read.
 */
//size_t strread_read_xstr(strread *trait, xstring *dest, size_t n);


/**
 * @brief Attempts to read the next @p n xchars into the xstring *dest.
 *        Less than @p n xchars can be read if the stream reaches its end.
 * @returns The number of xchars actually read.
 */
//size_t strread_read_xstr_until(strread *trait, xstring *dest, xchar_t delim);


/**
 * @brief Closes the stream and disposes the reader object.
 */
//void strread_close(strread *trait);


/**
 * @brief Returns the size of the xchar used in the stream.
 */
size_t strread_sizeof_char(strread *trait);


#endif