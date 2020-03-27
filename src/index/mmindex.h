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

#ifndef MMINDEX_H
#define MMINDEX_H


#include "alphabet.h"
#include "strread.h"
#include "xstring.h"
#include "vec.h"

typedef struct _mmindex mmindex;

/**
 * @param ab (move)
 * @param n
 * @param w (move)
 * @param k (move)
 */
mmindex *mmindex_new(alphabet *ab, size_t n, size_t *w, size_t *k);

void mmindex_index(mmindex *mmidx, strread *str);

const vec *mmindex_get(mmindex *mmidx, xstring *kmer);


#endif