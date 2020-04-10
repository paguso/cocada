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

#include <iter.h>

typedef struct _strbuf strbuf;

/**
 * @file strbuf.h
 * @author Paulo Fonseca
 *
 * @brief String buffer.
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
 * @brief Destructor.
 */
void strbuf_free(strbuf *sb);


/**
 * @brief Creates a new string buffer from a source static string @src.
 * @warning The source string contents are simply copied onto the dynamic
 *          string and the former is left untouched.
 */
strbuf *strbuf_new_from_str(const char *src);


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
void strbuf_append(strbuf *sb, const char *suff);


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
void strbuf_join(strbuf *sb, size_t n, const char**arr, const char *sep);



/**
 * @brief Appends copies of strings in an iterator to @p sb,
 *        separating each of these strings by @p sep
 * # Example
 * ```C
 * vec *v = new vec(sizeof(char *));
 * vec_push_rawptr(cstr_clone("Spring"));
 * vec_push_rawptr(cstr_clone("Summer"));
 * vec_push_rawptr(cstr_clone("Autumn"));
 * vec_push_rawptr(cstr_clone("Winter"));
 * vec_iter *it = vec_get_iter(v);
 * strbuf *sb = str_buf_new_from_str("Four seasons: ");
 * strbuf_join(sb, it, " and ");
 * printf("%s", strbuf_as_str(sb)); // prints "Four seaons: Spring and Summer and Autumn and Winter"
 * ```
 */
void strbuf_join_iter(strbuf *sb, iter *it, const char *sep);


/**
 * @brief Appends a character @p c.
 */
void strbuf_append_char(strbuf *sb, char c);



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