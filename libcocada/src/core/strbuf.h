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
void strbuf_free(strbuf *sb);


/**
 * @brief Creates a new string buffer from a source static string @src.
 * @warning The source string contents are simply copied onto the dynamic
 *          string and the former is left untouched.
 */
strbuf *strbuf_new_from_str(const char *src, size_t len);


/**
 * @brief Returns the "logical" length of a given dynamic string.
 */
size_t strbuf_len(strbuf *sb);


/**
 * @brief Returns the capacity (i.e. the "physical" length)
 *        of a given dynamic string.
 */
size_t strbuf_capacity(strbuf *sb);


/**
 * @brief Returns the character at a given position.
 */
char strbuf_get(strbuf *sb, size_t pos);


/**
 * @brief Sets all positions to '\0'.
 */
void strbuf_clear(strbuf *sb);


/**
 * @brief Sets (overwrites) the character of a given position @p pos to @p c.
 */
void strbuf_set(strbuf *sb, size_t pos, char c);



/**
 * @brief Appends a copy of the contents of a static string @p suff.
 * @warning The source string @p suff is left untouched.
 */
void strbuf_append(strbuf *sb, const char *suff, size_t len);


/**
 * @brief Appends a character @p c.
 */
void strbuf_append_char(strbuf *sb, char c);


/**
 * @brief Inserts a string @p str of length @p len at position @p pos.
 * @warning No bound checks performed.
 */
void strbuf_ins(strbuf *sb, size_t pos, const char *str, size_t len);


/**
 * @brief Cuts the substring @p sb[@p from:@pfrom + @p len] from the buffer.
 * If @p dest is not NULL, the removed substring is copied there.
 * @warning No bound checks performed.
 * @warning If @p dest is non-null, it should have space for at least @p len + 1
 * chars, since a '\0' is added right after the copied substring such that
 * @p dest is a prorperly null-terminated C string.
 */
void strbuf_cut(strbuf *sb, size_t from, size_t len, char *dest);


/**
 * @brief Pastes the string @p src of length @p len over the contents
 * of the string buffer @p sb, starting from position @p from.
 * The string buffer will be extended if the pasted over string goes
 * past the end of the buffer.
 *
 * @warning No bound checks performed.
 */
void strbuf_paste(strbuf *sb, size_t from, const char *src, size_t len);


/**
 * @brief Appends copies of @p n strings in an array @p arr to @p sb,
 *        separating each of these strings by @p sep
 * # Example
 * ```C
 * strbuf *sb = str_buf_new_from_str("Four seasons: ");
 * char *seasons[4] = {"Spring", "Summer", "Autumn", "Winter"};
 * strbuf_join(sb, 4, seasons, " and ");
 * printf("%s", strbuf_as_str(sb)); // prints "Four seaons: Spring and Summer and Autumn and Winter"
 * ```
 */
void strbuf_join(strbuf *sb, size_t n, const char **arr, const char *sep);


/**
 * @brief Concatenates two stringbuffers.
 */
void strbuf_cat(strbuf *dest, const strbuf *src);


/**
 * @brief Concatenates the @p n-prefix of @p src (or all @p src if its
 * length is <= @p n ) to @p dest.
 */
void strbuf_ncat(strbuf *dest, const strbuf *src, size_t n);


/**
 * @brief Replaces the first @p n  left-to-right non-overlapping occurrences
 * of the substring @p old in the string buffer @p sb, with the substring @p new.
 *
 * # Example
 * ```
 * strbuf *sb = strbuf_new_from_str("macaca", 6);
 * strbuf_replace_n(sb, "ca", "na", 2);
 * // yelds "manana"
 * strbuf_replace_n(sb, "ma", "ba", 2);
 * // yelds "banana" . only 1 <= 2 occurrence found
 * strbuf_replace_n(sb, "ana", "aca", 2);
 * // yelds "bacana" . only 1 <= 2 "non-overlapping" occurrence found
 * ```
 */
void strbuf_replace_n(strbuf *sb, const char *old, const char *new, size_t n);


/**
 * @brief Same as `strbuf_replace_n(sb, old, new, 1)`
 * @see strbuf_replace_n
 */
void strbuf_replace(strbuf *sb, const char *old, const char *new);


/**
 * @brief Same as `strbuf_replace_n(sb, old, new, strbuf_len(sb) + 1)`
 * @see strbuf_replace_n
 */
void strbuf_replace_all(strbuf *sb, const char *old, const char *new);


/**
 * @brief Returns a read-only reference to the current internal static string.
 *
 * @warning The internal string can change between calls and the returned
 *          reference can become NULL or invalid.
 */
const char *strbuf_as_str(strbuf *sb);


/**
 * @brief Detaches and returns the current internal static string after
 *        trimming (removal of trailing unused positions).
 *
 * @see cstr_trim
 * @warning After this operation, the dynamic string @p sb is destroyed.
 */
char *strbuf_detach(strbuf *sb);

#endif