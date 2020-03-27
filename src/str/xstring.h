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

#ifndef XSTRING_H
#define XSTRING_H

#include "dynstr.h"
#include "new.h"
#include "xchar.h"

/**
 * @file xstring.h
 *
 * @author Paulo Fonseca
 *
 * @brief Dynamic strings over eXtended character sets.
 */

/**
 * @brief eXtended string
 */
typedef struct _xstring xstring;


/**
 * @brief Creates a new empty string.
 * @param sizeof_char the size of each extended character in bytes.
 */
xstring *xstring_new(size_t sizeof_char);


/**
 * @brief Creates a new xstring with a specified length.
 *        Positions 0 through @p len-1 are set to 0.
 * @param sizeof_char the size of each extended character in bytes.
 */
xstring *xstring_new_with_len(size_t sizeof_char, size_t len);


/**
 * @brief Creates a new empty xstring with a specified initial capacity.
 * @param sizeof_char the size of each extended character in bytes.
 * @param cap The initial capacity.
 */
xstring *xstring_new_with_capacity(size_t sizeof_char, size_t cap);


/**
 * @brief Converts a raw byte array into an xstring. The source array is
 * moved into the xstring, meaning that, after the conversion,
 * the @p src array **should be no longer used from outside the xstring**.
 * @param src (**move**) The raw source array.
 * @param len The length of the created xstring.
 * @param sizeof_char The size of the extended char in bytes.
 * @warning
 * - The size of the source buffer @p src should be at least
 *   @p len * @p sizeof_char bytes. Positions after this number will be ignored.
 * - @p src must be a heap allocated array. In particular, no constant array
 *   or string literal should be used.
 * @see xstring_new_from_arr_cpy
 */
xstring *xstring_new_from_arr(void *src, size_t len, size_t sizeof_char);


/**
 * @brief Creates a new xstring from a *copy* of a raw byte array.
 * The @p src array should be dealt with by the caller after this function call.
 * This should be used to create an xstring form a string literal, for example
 * ```C
 *     char *p = "Some string literal";
 *     xstring *s = xstring_new_from_arr_cpy(p, strlen(p), sizeof(char));
 * ```
 * @param src  The raw source array.
 * @param len The length of the created xstring.
 * @param sizeof_char The size of the extended char in bytes.
 * @warning
 * - The size of the source buffer @p src should be at least
 *   @p len * @p sizeof_char bytes. Positions after this number will be ignored.
 * @see xstring_new_from_arr
 */
xstring *xstring_new_from_arr_cpy(const void *src, size_t len, size_t sizeof_char);


/**
 * @brief Destructor.
 */
void xstring_free(xstring *xs);


/**
 * @brief Destructor.
 */
void xstring_dispose(void *ptr, const dtor *dt);


/**
 * @brief Trims the internal representation of the xstring, i.e. deallocates
 *        unused internal memory.
 */
void xstr_trim(xstring *xs);


/**
 * @brief Clips the xstring
 */
void xstr_clip(xstring *xs, size_t from, size_t to);


/**
 * @brief Clears the xstring
 */
void xstr_clear(xstring *xs);


/**
 * @brief Rotates the xstring @npos positions to the left.
 * Example: `xstr_rotate_left(s="abcdefg",3) -> s="defgabc"`
 */
void xstr_rot_left(xstring *xs, size_t npos);


/**
 * @brief Prints a xstring to the standard output.
 */
void xstr_print(const xstring *xs);


/**
 * @brief Appends a string representation of the xstring to a string buffer
          @p dest
 */
void xstr_to_string (const xstring *xs, dynstr *dest);


/**
 * @brief Returns the lenght of the xstring.
 */
size_t xstr_len(const xstring *xs);


/**
 * @brief Returns the size of each character position in bytes.
 */
size_t xstr_sizeof_char(const xstring *xs);


/**
 * @brief Returns the char at a specified position.
 * @warn  No out-of-bounds verification is assumed.
 */
xchar_t xstr_get(const xstring *xs, size_t pos);


/**
 * @brief Sets the char at a specified position.
 * @warn  No out-of-bounds verification is assumed.
 * @warn  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_set(xstring *xs, size_t pos, xchar_t val);


/**
 * @brief Sets the first n chars to a specified value
 * @warn  No out-of-bounds verification is assumed.
 * @warn  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_nset(xstring *xs, size_t n, xchar_t val);


/**
 * @brief Appends a new char to a xstring.
 * @warn  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_push(xstring *xs, xchar_t c);


/**
 * @brief Appends @p n copies of char @p to the string.
 * @warn  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_push_n(xstring *xs, xchar_t c, size_t n);


/**
 * @brief Concatenates (copies) the contents of @p src at the end of @p dest.
 * @warn  Requires that both xstrings have the same character byte size.
 */
void xstr_cat(xstring *dest, const xstring *src);


/**
 * @brief Same as <code>xstr_ncpy(dest, 0, src, 0, xstr_len(dest))</code>.
 * @see xstr_ncpy
 * @see xstr_len
 */
void xstr_cpy(xstring *dest, const xstring *src);


/**
 * @brief Copies @p n characters from position @p from_src of the source
 *        xstring @p src into the destination xstring @p dest, starting at
 *        position @p from_dest. That is, copies and pastes
 *        @p src[@p from_src : @p from_src + @p nxchars] over
 *        @p dest[@p from_dest : @p from_dest + @p nxchars].
 *        Source and destination may not overlap.
 *
 * @warn Assumes without verification that both xstrings have the same internal
 *       character bytesize.
 * @warn The destination must be large enough, or a buffer overrun will occur.
 *       No out-of-bounds verification is performed.
 */
void xstr_ncpy( xstring *dest, size_t from_dest, const xstring *src, size_t from_src, size_t n );


/**
 * @brief Returns the representation of the xstring as a byte array and
          destroys the xstring.
 */
void *xstr_detach(xstring *xs);


/**
 * @brief Lexicographically compares the first @p n xchars  of two strings.
 * @return -1 if this < other, 0 if this==other, +1 if this > other
 */
int xstr_ncmp(const xstring *this, const xstring *other, size_t n);


/**
 * @brief Lexicographically compares two strings.
 * @return -1 if this < other, 0 if this==other, +1 if this > other
 */
int xstr_cmp(const xstring *this, const xstring *other);



#endif // !XSTRING_H