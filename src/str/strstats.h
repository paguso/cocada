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

#ifndef STRSTATS_H
#define STRSTATS_H

#include <stddef.h>

#include "alphabet.h"
#include "strstream.h"
#include "xchar.h"
#include "xstring.h"

/**
 * @brief Returns individual letter counts of a given string.
 *
 * @deprecated Too specific. Should move out of here.
 */
size_t *char_count(char *str, size_t slen, alphabet *ab);


size_t *xchar_count(xstring *xstr, alphabet *ab);


/**
 * @brief Compute cumulative char frequencies
 * cumul_count(T, c) = Sum for a<c count(T,a)
 *
 * Example:
 * ```
 *        T =abracadabra   AB={a,b,c,d,r}
 *
 *           i    :   0    1    2    3    4    5
 *     alphabet   :   a    b    c    d    r
 *      chr_cnt   :   5    2    1    1    2
 *  cumul_freqs   :   0    5    7    8    9   11
 * ```
 *
 * Therefore, if the alphabet is A=<a[0], ..., a[s-1]>, then
 * the number of chars in the interval a[i]...a[j-1] is given by
 * cumul_count[j] - cumul_count[i]
 *
 * @deprecated Too specific. Should move out of here.
 */
size_t  *char_cumul_count(char *str, size_t slen, alphabet *ab);


/**
 * @brief Returns individual letter counts of a given stream.
 * @see ab_count
 * @deprecated Too specific. Should move out of here.
 */
size_t *char_count_stream(strstream *sst, alphabet *ab);


/**
 * @brief Compute cumulative char frequencies of a given stream
 * @deprecated Too specific. Should move out of here.
 */
size_t  *char_cumul_count_stream(strstream *sst, alphabet *ab);


#endif // STRSTATS_H