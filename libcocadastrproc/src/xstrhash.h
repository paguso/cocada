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

#ifndef __XSTRHASH_H__
#define __XSTRHASH_H__

#include <stddef.h>
#include <stdint.h>

#include "alphabet.h"
#include "new.h"
#include "xstr.h"


/**
 * @file xstrhash.h
 * @author Paulo Fonseca
 * @brief Lexicographic hash values of xstrings.
 *
 * The hash of an xstring over an alphabet of size s is its value as a number
 * in base s, each character standing for its rank in the alphabet
 * (#xstrhash_lex). Hash values wrap around modulo 2^64. The rolling versions
 * (#xstrhash_roll_lex) compute the hash of the next window of a string from
 * the previous one in constant time.
 */

typedef struct _xstrhash xstrhash;


/**
 * @param @move ab alphabet
 */
xstrhash *xstrhash_new(alphabet *ab);

void xstrhash_finalise(void *ptr, const finaliser *fnr);

uint64_t xstrhash_lex(const xstrhash *self, const xstr *s);

uint64_t xstrhash_lex_sub(const xstrhash *self, const xstr *s, size_t from,
                          size_t to);

uint64_t xstrhash_roll_lex(const xstrhash *self, const xstr *s, uint64_t hash,
                           xchar_t c);

uint64_t xstrhash_roll_lex_sub(const xstrhash *self, const xstr *s, size_t from,
                               size_t to, uint64_t hash, xchar_t c);


#endif