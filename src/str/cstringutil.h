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

#ifndef CSTRINGUTIL_H
#define CSTRINGUTIL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "new.h"

/**
 * @file cstringutil.h
 * @author Paulo Fonseca
 *
 * @brief Plain C strings (char *) manipulation functions.
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
 * @brief 'Trims' the string @p str to @p str[@p from..@p to-1].
 *        The trimmed char array will end with a '\0'.
 *        The memory used by the parts of the string out of this interval
 *        will be freed. As part of the operation, the remaining 'trimmed'
 *        portion may be relocated.
 * @return The address of the trimmed string.
 */
char *cstr_trim(char *str, size_t from,  size_t to);


/**
 * @brief Same as cstr_trim(@p str, 0, @plen)
 */
char *cstr_trim_to_len(char *str, size_t len);


/**
 * @brief resizes the string to a given length @p len.
 * If @len is less than the current string length, this is equivalent
 * to `cstr_trim(str, 0, len)`. Else, the string is enlarged to 
 * have physical length @p len+1, i.e. with trailing positions
 * set to '\0'.
 * @warning The string may be relocated.
 */
char *cstr_resize(char *str, size_t len);


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
 */
void uint_to_cstr(char *dest, uintmax_t val, char base);



#endif

