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

#ifndef XSTRREAD_H
#define XSTRREAD_H

#include <stddef.h>

#include "xchar.h"
#include "xstr.h"


typedef struct _xstrread xstrread;


typedef struct _xstrread_vt {
	void 	 (*reset) (xstrread *self);
	xchar_wt (*getch) (xstrread *self);
	size_t   (*read) (xstrread *self, xstr *dest, size_t n);
	size_t   (*read_until) (xstrread *self, xstr *dest, xchar_t delim);
} xstrread_vt;


struct _xstrread {
	xstrread_vt *vt;
	void *impltor;
};


/**
 * @brief Resets the stream, i.e. moves cursor to initial position.
 */
void xstrread_reset(xstrread *trait);


/**
 * @brief Reads the next xchar_t from a stream.
 * @returns The next character as an xchar_wt, or XEOF if the stream has
 *          reached its end.
 */
xchar_wt xstrread_getc(xstrread *trait);


/**
 * @brief Attempts to read the next @p n chars into the xstring *dest.
 *        Less than @p n characters can be read if the stream reaches its end.
 * @returns The number of chars actually read.
 */
size_t xstrread_read(xstrread *trait, xstr *dest, size_t n);


/**
 * @brief Reads characters from the reader into the xstring @p dest until a
 *        delimiter xchar_t @p delim is found. This delimiter xchar is not
 *        consumed.
 * @returns The number of chars actually read (excluding the delimiter).
 */
size_t xstrread_read_until(xstrread *trait, xstr *dest, xchar_t delim);



#endif