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

#ifndef STRVIEW_H
#define STRVIEW_H

#include "coretype.h"
#include "iter.h"
#include "trait.h"


/**
 * @file strview.h
 * @brief String View
 * @author Paulo Fonseca
 *
 * A String View is a lightweight object used to represent an
 * *unmutable* substring of an existing string.
 */


/**
 * @brief String View format modifier to be used to print its represented
 * substring with printf-family functions
 *
 * Example:
 * ```C
 * strview sv;
 * // initialise the strview...
 * printf("str = %"SVFMT" len = %zu\n", SVPRT(sv), sv.len);
 */
#define SVFMT ".*s"


/**
 * @brief To be used together with SVFMT to print the (sub)string content
 * of @p S  with printf-family functions
 *
 * @see SVFMT
 */
#define SVPRT(S) (int)S.len, S.str


/**
 * @brief String View type.
 */
typedef struct strview {
	const char *str;  /**< A **non-owned** pointer to a source string */
	size_t len; /**< The length of the view */
} strview;



/**
 * @brief String View initialiser from a null-terminated C string.
 *
 * @param self A pointer to the memory to be initialised
 * @param src a pointer to the source string
 *
 * @warning The #strview has only non-owned references, so no
 * specific destructor is needed for a stack-allocated strview.
 */
void strview_init(strview *self, const char* src);


/**
 * @brief String View builder from a null-terminated C string.
 *
 * @param src a pointer to the source string
 * @return A copy (by value) of a strview representing @p src
 *
 * @see strview_init
 */
strview strview_build(const char* src);


/**
 * @brief String View constructor from a null-terminated C string.
 *
 * @warning #strview is a lightweight object meant to be mostly
 * stack-allocated or stored in flat arrays. This constructor returns
 * a new heap-allocated instance. This should be avoided but this
 * constructor is included for completeness. Since #strview has no
 * owned references, no specific destructor is needed.
 * A heap-allocated #strview can be disposed of with #FREE .
 *
 * @see strview_init
 * @see FREE
 */
strview *strview_new(const char* src);


/**
 * @brief String View initializer from a given pointer and substring range.
 *
 * @param self A pointer to the memory to be initialised
 * @param src a pointer to the source string
 * @param from The initial position of the substring/offset from the start
 * position of @p src
 * @param to The final position of the substring (not included).
 *
 * @warning No bound checks performed. If @p to < @p from, the behaviour is undefined.
 */
void strview_init_from_range(strview *self, const char* src, size_t from, size_t to);


/**
 * @brief String View builder from a given pointer and substring range.
 *
 * @param src a pointer to the source string
 * @param from The initial position of the substring/offset from the start
 * position of @p src
 * @param to The final position of the substring (not included)
 *
 * @return A copy (by value) of a strview representing  `src[from:to]`
 *
 * @warning No bound checks performed. If @p to < @p from, the behaviour is undefined.
 *
 * @see strvire_init_from_range
 */
strview strview_build_from_range(const char* src, size_t from, size_t to);


/**
 * @brief String View constructor from a given pointer and substring range.
 *
 * @warning #strview is a lightweight object meant to be mostly
 * stack-allocated or stored in flat arrays. This constructor returns
 * a new heap-allocated instance. This should be avoided but this
 * constructor is included for completeness. Since #strview has no
 * owned references, no specific destructor is needed.
 * A heap-allocated #strview can be disposed of with #FREE .
 *
 * @warning No bound checks performed. If @p to < @p from, the behaviour is undefined.
 *
 * @see strview_new_from_range
 * @see FREE
 */
strview *strview_new_from_range(const char* src, size_t from, size_t to);


/**
 * @brief Creates a new heap-allocated null-terminated C string from
 * a strview
 */
char *strview_to_new_cstr(const strview *self);


/**
 * @brief Creates a new heap-allocated null-terminated C string from
 * the strview substring `self[from:to]`.
 *
 * @param from The initial position of the substring/offset from the start
 * position of @p self
 * @param to The final position of the substring (not included)
 *
 * @warning No bound checks performed. If @p to < @p from, the behaviour is undefined.
 */
char *strview_substr_to_new_cstr(const strview *self, size_t from, size_t to);


/**
 * @brief Copies the contents of a string view to a char buffer and appends
 * a null '\0' char.
 *
 * @param self The source strview
 * @param dest The destination buffer
 *
 * @warning @p dest must be large enough to hold the length of @self plus
 * the extra '\0'. Bound checks are not ensured.
 */
void strview_cpy_to_cstr(const strview *self, char *dest);


/**
 * @brief Copies the contents of the string view substring `self[from:to]`
 * to a char buffer and appends  a null '\0' char.
 *
 * @param self The source strview
 * @param from The initial position of the substring/offset from the start
 * position of @p self
 * @param to The final position of the substring (not included)
 * @param dest The destination buffer
 *
 * @warning @p dest must be large enough to hold the length of @self plus
 * the extra '\0'. Bound checks are not ensured.
 */
void strview_substr_cpy_to_cstr(const strview *self, size_t from, size_t to, char *dest);


/**
 * @brief Returns the length of th strview.
 */
size_t strview_len(const strview *self);


/**
 * @brief String view lexicographic comparison, compatible with #cmp_func (order.h)
 *
 * @param left a pointer to a strview
 * @param right a pointer to a strview
 *
 * @return -1 if @p left is lexicographically smaller than @p right,
 * 		    0 if @p left is equal to @p right, or
 * 		   +1 if @p left is lexicographically greater than @p right,
 *
 * @see cmp_func
 */
int strview_cmp(const void* left, const void *right);


/**
 * @brief Compares the contents of the string view to a raw string
 * for equality.
 */
bool strview_eq_cstr(const strview *self, const char *other);


/**
 * @brief Compares the contents of the string view to a raw string
 * for equality up to the first @p n positions.
 *
 * @return true iff the length of the strview is at least @p n and
 * its first @p n positions are equal to those of @p other.
 */
bool strview_eqn_cstr(const strview *self, const char *other, size_t n);


/**
 * @brief Returns a #strview coresponding to `self[from:to]`
 */
strview strview_substr(const strview *self, size_t from, size_t to);


/**
 * @brief Finds the first occurrence of a substring in a string view.
 *
 * @param substr The substring searched for
 * @param len The length of the substring searched for
 *
 * @return The position of the first occurrence of @p substr in @self,
 * if any, or the length of the string view otherwise.
 */
size_t strview_find(const strview *self, const char *substr, size_t len);


/**
 * @brief Finds the last occurrence of a substring in a string view.
 *
 * @param substr The substring searched for
 * @param len The length of the substring searched for
 *
 * @return The position of the last occurrence of @p substr in @self,
 * if any, or the length of the string view otherwise.
 * The length of the string view is also returned if @p len == 0.
 */
size_t strview_find_last(const strview *self, const char *substr, size_t len);


/**
 * @brief Find the first @p n occurrences of the pattern @p pat in @p self.
 * The positions of the matches are stored in @p dest.
 *
 * @return The number of matches of @p pat in @p self from left to right,
 * starting at position @p from_pos up to a maximum number @p n.

 * @warning The array @p dest should have enough space to store at least
 * @p n size_t values.
 *
 * @note This algorithm runs in O(m+l) time, where m=strlen(pat) and l=strbuf_len(self)-from_pos.
 */
size_t strview_find_n(const strview *self, const char *pat, size_t len, size_t n, size_t *dest);


/**
 * @brief String View Iterator type.
 *
 * A strview iterator is used to iterate over substrings of a source
 * strview, each represented by another strview.
 *
 * @see strview_iter_has_next
 * @see strview_iter_next
 */
typedef struct strview_iter strview_iter;


/**
 * @brief String View Iterator destructor.
 */
void strview_iter_free(strview_iter *self);


/**
 * @brief Does the iterator have any remaining element?
 */
bool strview_iter_has_next(strview_iter *self);


/**
 * @brief Gets the next substring view of the iterator.
 * @return If  `strview_iter_has_next(it) == true` returns the next
 * substring view. Otherwise, the behaviour is undefined.
 */
strview strview_iter_next(strview_iter *self);


/**
 * @brief Splits a string view.
 *
 * Let `X` be a string of lenght `n >= 0` and `S` be a separator string
 * of length `m > 0` such that `X = T_0 S_1 T_1 S_2 ... T_k-1 S_k T_k`
 * where `S_1 .. S_k` are all the non-overlapping left-to-right occurrences
 * of `S` in `X` and `T_0 .. T_k` are the possibly empty strings in-between
 * these occurrences. This operation returns an iterator over `T_0 .. T_k`.
 *
 * Example
 *
 * ```C
 * strview sview = strview_build("strview_iter_has_next_");
 * strview_iter *iter = strview_split(&sview, "_", 1);
 * for (int i = 0; strview_iter_has_next(iter); i++) {
 *     strview tok = strview_iter_next(iter);
 *     printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
 * }
 * ```
 * prints:
 * ```
 * tok = strview (len = 7)
 * tok = iter (len = 4)
 * tok = has (len = 3)
 * tok = next (len = 4)
 * tok =  (len = 0)
 * ```
 *
 * @param self The string view to be split
 * @param sep The separator string
 * @param sep_len The length of the separator
 *
 * @returns An iterator over the split tokens
 *
 * @warning The separator must be a non-empty string.
 */
strview_iter *strview_split(const strview *self, const char *sep, size_t sep_len);


#endif
