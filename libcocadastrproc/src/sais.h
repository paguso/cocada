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

#ifndef SAIS_H
#define SAIS_H

#include <stddef.h>

#include "alphabet.h"

/**
 * @file sais.h
 * @author Paulo Fonseca
 *
 * @brief G.Nong, S.Zhang and W.H.Chan Suffix Array Induced Sorting (SA-IS)
 *        linear time construction algorithm.
 *        http://ieeexplore.ieee.org/document/5582081/
 */


/**
 * @brief Builds the plain suffix array for the string @p str over the
 *        alphabet @p ab.
 *
 * @warning  A sentinel character, lexicographically smaller than any char in
 *        @p ab is virtually added to @str s.t. the returned suffix array SA
 *        will have size @p len+1 and SA[0]==len.
 */
size_t *sais(char *str, size_t len, alphabet *ab);


#endif