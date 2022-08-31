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

#ifndef STRBUF_H
#define STRBUF_H


#include <stddef.h>

#include "new.h"


typedef struct _strbuf strbuf;

/**
 * @file strbuf.h
 * @brief String buffer (dynamic char string)
 * @author Paulo Fonseca
 */


/**
 * @brief Creates a new empty string buffer.
 */
strbuf *strbuf_new();


/**
 * @brief Creates an empty string buffer with a given initial capacity.
 */
strbuf *strbuf_new_with_capacity(size_t init_capacity);


/**
 * @brief Finaliser
 * @see new.h
 */
void strbuf_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Destructor.
 */
void strbuf_free(strbuf *self);


/**
 * @brief Creates a new string buffer from a source static string @src.
 * @warning The source string contents are simply copied onto the dynamic
 *          string and the former is left untouched.
 */
strbuf *strbuf_new_from_str(const char *src, size_t len);


/**
 * @brief Returns the "logical" length of a given dynamic string.
 */
size_t strbuf_len(strbuf *self);


/**
 * @brief Returns the capacity (i.e. the "physical" length)
 *        of a given dynamic string.
 */
size_t strbuf_capacity(strbuf *self);


/**
 * @brief Returns the character at a given position.
 */
char strbuf_get(strbuf *self, size_t pos);


/**
 * @brief Sets all positions to '\0'.
 */
void strbuf_clear(strbuf *self);


/**
 * @brief Sets (overwrites) the character of a given position @p pos to @p c.
 */
void strbuf_set(strbuf *self, size_t pos, char c);


/**
 * @brief Appends a character @p c.
 */
void strbuf_append_char(strbuf *self, char c);


/**
 * @brief Appends a copy of the first @p len chars of a static string @p str.
 * @warning No bounds checks performed.
 */
void strbuf_nappend(strbuf *self, const char *src, size_t len);


/**
 * @brief Same as `strbuf_nappend(self, src, strlen(src))`.
 */
void strbuf_append(strbuf *self, const char *src);


/**
 * @brief Inserts a string @p str of length @p len at position @p pos.
 * @warning No bound checks performed.
 */
void strbuf_ins(strbuf *self, size_t pos, const char *str, size_t len);


/**
 * @brief Cuts the substring @p self[@p from:@pfrom + @p len] from the buffer.
 * If @p dest is not NULL, the removed substring is copied there.
 * @warning No bound checks performed.
 * @warning If @p dest is non-null, it should have space for at least @p len + 1
 * chars, since a '\0' is added right after the copied substring such that
 * @p dest is a prorperly null-terminated C string.
 */
void strbuf_cut(strbuf *self, size_t from, size_t len, char *dest);


/**
 * @brief Pastes the string @p src of length @p len over the contents
 * of the string buffer @p self, starting from position @p from.
 * The string buffer will be extended if the pasted over string goes
 * past the end of the buffer.
 *
 * @warning No bound checks performed.
 */
void strbuf_paste(strbuf *self, size_t from, const char *src, size_t len);


/**
 * @brief Appends copies of @p n strings in an array @p arr to @p self,
 *        separating each of these strings by @p sep
 * # Example
 * ```C
 * strbuf *self = str_buf_new_from_str("Four seasons: ");
 * char *seasons[4] = {"Spring", "Summer", "Autumn", "Winter"};
 * strbuf_join(self, 4, seasons, " and ");
 * printf("%s", strbuf_as_str(self)); // prints "Four seaons: Spring and Summer and Autumn and Winter"
 * ```
 */
void strbuf_join(strbuf *self, size_t n, const char **arr, const char *sep);


/**
 * @brief Appends a copy of the contents of @p other to  @p self.
 */
void strbuf_cat(strbuf *self, const strbuf *other);


/**
 * @brief Appends the @p n-prefix of @p other (or all @p other if its
 * length is <= @p n ) to @p self.
 */
void strbuf_ncat(strbuf *self, const strbuf *other, size_t n);


/**
 * @brief Replaces the first @p n  left-to-right non-overlapping occurrences
 * of the substring @p old in the string buffer @p self, with the substring @p new.
 *
 * # Example
 * ```
 * strbuf *self = strbuf_new_from_str("macaca", 6);
 * strbuf_replace_n(self, "ca", "na", 2);
 * // yelds "manana"
 * strbuf_replace_n(self, "ma", "ba", 2);
 * // yelds "banana" . only 1 <= 2 occurrence found
 * strbuf_replace_n(self, "ana", "aca", 2);
 * // yelds "bacana" . only 1 <= 2 "non-overlapping" occurrence found
 * ```
 */
void strbuf_replace_n(strbuf *self, const char *old, const char *new, size_t n);


/**
 * @brief Same as `strbuf_replace_n(self, old, new, 1)`
 * @see strbuf_replace_n
 */
void strbuf_replace(strbuf *self, const char *old, const char *new);


/**
 * @brief Same as `strbuf_replace_n(self, old, new, strbuf_len(self) + 1)`
 * @see strbuf_replace_n
 */
void strbuf_replace_all(strbuf *self, const char *old, const char *new);


/**
 * @brief Writes formatted output to a string buffer. This function is analogous
 * to sprintf(), except that the output is *appended* to @p self.
 * The format string @p fmt and variable list of arguments follow the printf()
 * family rules.
 *
 * @return Upon success, returns the number of written chars (excluding the
 * null-terminating char '\0').
 *
 */
int strbuf_printf(strbuf *self, const char *fmt, ...);



/**
 * @brief Returns a read-only reference to the current internal static string.
 *
 * @warning The internal string can change between calls and the returned
 *          reference can become NULL or invalid.
 */
const char *strbuf_as_str(strbuf *self);


/**
 * @brief Detaches and returns the current internal static string after
 *        trimming (removal of trailing unused positions).
 *
 * @see cstr_trim
 * @warning After this operation, the dynamic string @p self is destroyed.
 */
char *strbuf_detach(strbuf *self);



#endif