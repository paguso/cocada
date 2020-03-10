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

#ifndef CSARRAY_H
#define CSARRAY_H

#include <stddef.h>

#include "alphabet.h"
#include "strstream.h"

/**
 * @file csarray.h
 * @author Paulo Fonseca
 *
 * @brief Compressed Suffix Array.
 */

typedef struct _csarray csarray;


/**
 * @brief Creates a CSA for the string @p str of length @p len over
 *        the alphabet @p ab
 */
csarray *csarray_new(char *str, size_t len, alphabet *ab);


/**
 * @brief Creates a CSA from the stream @p sst over the alphabet @p ab
 */
csarray *csarray_new_from_stream(strstream *sst, alphabet *ab);


/**
 * @brief Destructor
 * @param csarr
 */
void csarray_free(csarray *csa);


/**
 * @brief Print the CSA to the standard output.
 */
void csarray_print(csarray *csa);


/**
 * @brief Returns the length of the CSA.
 *        A sentinel character is virtually added at the end of the source
 *        string s.t. the length of the CSA will be the length of
 *        the source string plus one
 */
size_t csarray_len(csarray *csa);


/**
 * @brief Returns csa.phi(i) defined as SA'[pos'] where
 *        SA' is the inverse of the suffix array and
 *        pos' = SA[i] + 1, if SA[i] < SA.len, or 0 otherwise.
 */
size_t csarray_phi(csarray *csa, size_t i);


/**
 * @brief Returns the suffix array value at position @p i.
 *        This corresponds to the text start position of the @p i-th suffix
 *        in lexicographic order.
 *        Because of the virtually added sentinel we have
 *        csarray_get(csarr, 0)==csarray_len(csarr)-1
 */
size_t csarray_get(csarray *csa, size_t i);


/**
 * @brief Returns the inverse of @p i, that is, the array index j s.t.
 *        csarray_get(@p csa, j) == @p i.
 *        This corresponds to the lexicographic rank of the
 *        source string suffix starting at position @p i.
 */
size_t csarray_get_inv(csarray *csa, size_t i);


/**
 * @brief Returns the char at position @p i of the source string inferred
 *        from the CSA.
 */
xchar_t csarray_get_char(csarray *csa, size_t pos);


#endif