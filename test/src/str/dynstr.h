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

#ifndef DYNSTR_H
#define DYNSTR_H

typedef struct _dynstr dynstr;

/**
 * @file dynstr.h
 * @author Paulo Fonseca
 * 
 * @brief Dynamic string (a.k.a. String buffer) ADT.
 */


/**
 * @brief Creates an empty dynamic string with default capaciy.
 */
dynstr *dynstr_new();


/**
 * @brief Creates an empty dynamic string with a given initial capacity.
 */
dynstr *dynstr_new_with_capacity(size_t init_capacity);


/**
 * @brief Destructor.
 */
void dynstr_free(dynstr *dstr);


/**
 * @brief Creates a new dynamic string from a source static string @src.
 * @warning The source string contents are simply copied onto the dynamic
 *          string and the former is left untouched.
 */
dynstr *dynstr_new_from_str(char *src);


/**
 * @brief Returns the "logical" length of a given dynamic string.
 */
size_t dstr_len(dynstr *dstr);


/**
 * @brief Returns the capacity (i.e. the "physical" length)
 *        of a given dynamic string.
 */
size_t dstr_capacity(dynstr *dstr);


/**
 * @brief Returns the character at a given position.
 */
char dstr_get(dynstr *dstr, size_t pos);


/**
 * @brief Sets all positions to '\0'.
 */
void dstr_clear(dynstr *dstr);


/**
 * @brief Sets (overwrites) the character of a given position @p pos to @p c.
 */
void dstr_set(dynstr *dtsr, size_t pos, char c);


/**
 * @brief Appends a copy of the contents of a static string @p suff. 
 * @warning The source string @p suff is left untouched.
 */
void dstr_append(dynstr *dstr, char *suff);


/**
 * @brief Appends a character @p c.
 */
void dstr_append_char(dynstr *dstr, char c);


/**
 * @brief Returns a read-only reference to the current internal static string.
 *
 * @warning The internal string can change between calls and the returned 
 *          reference can become NULL or invalid.
 */
const char *dstr_as_str(dynstr *dstr);


/**
 * @brief Detaches and returns the current internal static string after 
 *        trimming (removal of trailing unused positions).
 * 
 * @see cstr_trim
 * @warning After this operation, the dynamic string @p dstr is destroyed.
 */
char *dstr_detach(dynstr *dstr);

#endif