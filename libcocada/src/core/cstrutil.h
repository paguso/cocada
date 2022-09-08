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

#ifndef CSTRUTIL_H
#define CSTRUTIL_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "new.h"

/**
 * @file cstrutil.h
 * @author Paulo Fonseca
 *
 * @brief Plain C strings (char *) manipulation functions
 */


/**
 * @brief Creates a new C string of a given length.
 *        The physical length of the string will be @p len+1 and the
 *        string will be initially filled with '\0'.
 *        The returned array will thus be capable of storing strings whose
 *        strlen varies from 0 to @p len.
 */
char *cstr_new(size_t len);


/**
 * @brief Creates a clone of a source string.
 */
char *cstr_clone(const char *src);


/**
 * @brief Creates a clone of a source string up to a given length @p len.
 */
char *cstr_clone_len(const char *src, size_t len);


/**
 * @brief Changes the contents of @p dest to @p src. Use as `dest = cstr_reassign(dest, src);`
 * @return A pointer to the reassigned string.
 * @warning The string @p dest may be relocated if the previous location
 * is not large enough to hold the new value @p src. Make sure to use it
 * as "`dest = cstr_reassign(dest, src)`" to avoid pointer inconsistence.
 *
 * # Example
 * ```C
 * char *s = "foo";
 * printf("%s\n", s); // prints "foo"
 * s = reassign(s, "bar");
 * printf("%s\n", s); // prints "bar"
 * ```
 */
char *cstr_reassign(char *dest, const char *src);


/**
 * @brief Sets @p str[j]=@p c for @p from <= j < @p to.
 */
void cstr_fill(char *str, size_t from, size_t to, char c);


/**
 * @brief Sets the first @p len positons of @p str to '\0'.
 */
void cstr_clear(char *str, size_t len);


/**
 * @brief Returns a new string equals to @p str[@p from..@p to-1].
 */
char *cstr_substr(char *str, size_t from,  size_t to);


/**
 * @brief 'Crops' the string @p str to @p str[@p from: @p to].
 *        The cropped char array will end with a '\0'.
 *        The memory used by the parts of the string out of this interval
 *        will be freed. As part of the operation, the remaining 'cropped'
 *        portion may be relocated.
 * @return The address of the trimmed string.
 */
char *cstr_crop(char *str, size_t from, size_t to);



/**
 * @brief Same as cstr_crop(@p str, 0, @p len)
 */
char *cstr_crop_len(char *str, size_t len);


/**
 * @brief Removes unwanted chars from both ends of the string
 *
 * # Example
 *
 * ```C
 * char *s = "<!-- some nice comment -->";
 * cstr_trim(s, strlen(s), " -!<>", 5);
 * printf("%s", s); // prints "some nice comment" (w/o the quotes)
 * ```
 */
void cstr_trim(char *str, size_t len, char *unwanted, size_t unw_len);


/**
 * @brief resizes the string to a given length @p len.
 * If @p len is less than the current string length, this is equivalent
 * to `cstr_crop(str, 0, len)`. Else, the string is enlarged to
 * have physical length @p len+1, i.e. with trailing positions
 * set to '\0'.
 * @warning The string may be relocated.
 */
char *cstr_resize(char *str, size_t len);


/**
 * @brief Cuts away the substring @p str[@p from: @p to] from @p str.
 * Example: 
 * ```
 * char *str = "banana";
 * str = cstr_cut(str, 1, 3);
 * printf("%s", str); // prints "bana"
 * ```
 * If @p from >= @p to This operation has no effect.
 * The operation does not reallocate the string. To adjust the physiscal
 * array size to the new string size, use cstr_resize()
 * new size
 * @warning No bound checks are performed.
 */
char *cstr_cut(char *str, size_t from, size_t to);


/**
 * @brief Reverts a string in place.
 */
void cstr_revert(char *str, size_t len);


/**
 * @brief Converts an unsigned int to a string.
 * @param dest The target string with enough space
 *        ( ceil(log(val, b) + 1), where b is the base of the
 *        conversion
 * @param val the value to be converted
 * @param base 'b' (binary), 'o' (octal), 'd' (decimal=default),
 *        'h' (hexadecimal)
 * @return Upon success, returns a @p dest
 */
char *uint_to_cstr(char *dest, uintmax_t val, char base);


/**
 * @brief Compares two ASCII strings for equality
 */
bool cstr_equals(const char *left, const char *right);


/**
 * @brief Compares two ASCII strings for equality ignoring case.
 */
bool cstr_equals_ignore_case(const char *left, const char *right);


/**
 * @brief Joins @p n strings into a single string,
 * intercalating them with a given separator @p sep.
 *
 * #Example
 * ```C
 * char *lifecicle = cstr_join("-", 3, "code", "test", "repeat");
 * printf("%s\n",lifecicle);// prints "code-test-repeat"
 * ```
 */
char *cstr_join(const char *sep, size_t n, ...);


#define FOREACH_IN_SUBSTR(CHR, STR, FROM, TO) \
	for (char *__s = (char *)(STR), CHR; __s; __s = NULL) \
		for (size_t __i = FROM, __l = TO; __i < __l; __i = __l) \
			for (CHR = __s[__i]; __i < __l; CHR = ((++__i) < __l) ? __s[__i] : CHR )


#define FOREACH_IN_CSTR(CHR, STR) \
	for (char *__s = (char *)(STR), CHR; (CHR = (*__s)) != '\0'; __s += 1) \


#endif

