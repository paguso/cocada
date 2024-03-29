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

#ifndef XSTR_H
#define XSTR_H

#include "new.h"
#include "xchar.h"

/**
 * @file xstr.h
 *
 * @author Paulo Fonseca
 *
 * @brief Dynamic string over an eXtended character set.
 *
 * An xstr is an indexable dynamic sequence of 'characters' of
 * fixed length. The exact number of bytes needed to represent
 * individual characters is provided by the user to optmise space.
 * Characters are read from/written to the string via methods that
 * return/receive xchar_t values. Thus xstr characters must fit
 * into the xchar_t value, that is, they must require at most
 * XCHAR_BYTES bytes.
 *
 * @see xchar.h
 */

/**
 * @brief eXtended string
 */
typedef struct _xstr xstr;


/**
 * @brief Creates a new empty string.
 * @param sizeof_char the size of each extended character in bytes.
 */
xstr *xstr_new(size_t sizeof_char);


/**
 * @brief Creates a new empty xstr with a specified initial capacity.
 * @param sizeof_char the size of each extended character in bytes.
 * @param cap The initial capacity.
 */
xstr *xstr_new_with_capacity(size_t sizeof_char, size_t cap);


/**
 * @brief Converts a raw byte array into an xstr. The source array is
 * moved into the xstr, meaning that, after the conversion,
 * the @p src array **should be no longer used from outside the xstr**.
 * @param src (**move**) The raw source array.
 * @param len The length of the created xstr.
 * @param sizeof_char The size of the extended char in bytes.
 * @warning
 * - The size of the source buffer @p src should be at least
 *   @p len * @p sizeof_char bytes. Positions after this number will be ignored.
 * - @p src must be a heap allocated array. In particular, no constant array
 *   or string literal should be used.
 * @see xstr_new_from_arr_cpy
 */
xstr *xstr_new_from_arr(void *src, size_t len, size_t sizeof_char);


/**
 * @brief Creates a new xstr from a *copy* of a raw byte array.
 * The @p src array should be dealt with by the caller after this function call.
 * This should be used to create an xstr form a string literal, for example
 * ```C
 *     char *p = "Some string literal";
 *     xstr *s = xstr_new_from_arr_cpy(p, strlen(p), sizeof(char));
 * ```
 * @param src  The raw source array.
 * @param len The length of the created xstr.
 * @param sizeof_char The size of the extended char in bytes.
 * @warning
 * - The size of the source buffer @p src should be at least
 *   @p len * @p sizeof_char bytes. Positions after this number will be ignored.
 * @see xstr_new_from_arr
 */
xstr *xstr_new_from_arr_cpy(const void *src, size_t len, size_t sizeof_char);


/**
 * @brief Destructor.
 */
void xstr_free(xstr *self);


/**
 * @brief Fits the xstr to its actual size, i.e. deallocates
 *        unused internal memory.
 */
void xstr_fit(xstr *self);


/**
 * @brief Clips the xstr
 */
void xstr_clip(xstr *self, size_t from, size_t to);


/**
 * @brief Clears the xstr
 */
void xstr_clear(xstr *self);


/**
 * @brief Rotates the xstr @p npos positions to the left.
 * Example: `xstr_rotate_left(s="abcdefg",3) -> s="defgabc"`
 */
void xstr_rot_left(xstr *self, size_t npos);


/**
 * @brief Returns the lenght of the xstr.
 */
size_t xstr_len(const xstr *self);


/**
 * @brief Returns the size of each character position in bytes.
 */
size_t xstr_sizeof_char(const xstr *self);


/**
 * @brief Returns the internal raw byte contents of the xstr.
 * @warning The returned information is not meant to be directly modified.
 */
const byte_t *xstr_as_bytes(const xstr *self);


/**
 * @brief Returns the physical size (in bytes) of the internal raw byte representation
 * xstr_len(self) * xstr_sizeof_char(self)
 */
size_t xstr_nbytes(const xstr *self);


/**
 * @brief Returns the char at a specified position.
 * @warning  No out-of-bounds verification is assumed.
 */
xchar_t xstr_get(const xstr *self, size_t pos);


/**
 * @brief Sets the char at a specified position.
 * @warn  No out-of-bounds verification is assumed.
 * @warn  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_set(xstr *self, size_t pos, xchar_t val);


/**
 * @brief Sets the first n chars to a specified value
 * @warning  No out-of-bounds verification is assumed.
 * @warning  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_nset(xstr *self, size_t n, xchar_t val);


/**
 * @brief Appends a new char to a xstr.
 * @warning  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_push(xstr *self, xchar_t c);


/**
 * @brief Appends @p n copies of char @p to the string.
 * @warning  May result in information loss if the internal representation uses a
 *        smaller number of bytes for each position than sizeof(xchar_t).
 */
void xstr_push_n(xstr *self, xchar_t c, size_t n);


/**
 * @brief Concatenates (copies) the contents of @p src at the end of @p self.
 * @warning  Requires that both xstrs have the same character byte size.
 */
void xstr_cat(xstr *self, const xstr *src);


/**
 * @brief Same as <code>xstr_ncpy(self, 0, src, 0, xstr_len(self))</code>.
 * @see xstr_ncpy
 * @see xstr_len
 */
void xstr_cpy(xstr *self, const xstr *src);


/**
 * @brief Copies @p n characters from position @p from_other of the source
 *        xstr @p other into the destination xstr @p self, starting at
 *        position @p from_self. That is, copies and pastes
 *        @p other[@p from_other : @p from_other + @p n] over
 *        @p self[@p from_self : @p from_self + @p n].
 *        Source and destination may not overlap.
 *
 * @warning Assumes without verification that both xstrs have the same internal
 *       character bytesize.
 * @warning The destination must be large enough, or a buffer overrun will occur.
 *       No out-of-bounds verification is performed.
 */
void xstr_ncpy( xstr *self, size_t from_self, const xstr *other,
                size_t from_other,
                size_t n );


/**
 * @brief Returns the representation of the xstr as a byte array and
          destroys the xstr.
 */
void *xstr_detach(xstr *self);


/**
 * @brief Lexicographically compares the first @p n xchars  of two strings.
 * @return -1 if self < other, 0 if self==other, +1 if this > other
 */
int xstr_ncmp(const xstr *self, const xstr *other, size_t n);


/**
 * @brief Lexicographically compares two strings.
 * @return -1 if self < other, 0 if self==other, +1 if self > other
 */
int xstr_cmp(const xstr *self, const xstr *other);


#define FOREACH_IN_XSTR(CHR, STR) \
	for (xstr *__s = (xstr *)(STR); __s; __s = NULL) \
		for (xchar_t CHR = 1; CHR ; CHR = 0) \
			for (size_t __i = 0, __l = xstr_len(__s); __i < __l; __i = __l) \
				for (CHR = xstr_get(__s, __i); __i < __l; CHR = ((++__i) < __l) ? xstr_get(__s, __i) : CHR )



#endif // !XSTRING_H