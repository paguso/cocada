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

#ifndef ROARING_H
#define ROARING_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "coretype.h"

/**
 * @file roaring.h
 * @brief Roaring bitvector
 * @author Paulo Fonseca
 */

typedef struct _roaringbitvec roaringbitvec;

roaringbitvec *roaringbitvec_new(size_t n);

roaringbitvec *roaringbitvec_new_from_bitarr(byte_t *b, size_t n);

void roaringbitvec_free(roaringbitvec *self);

size_t roaringbitvec_card(roaringbitvec *self);

size_t roaringbitvec_count(roaringbitvec *self, bool bit);

size_t roaringbitvec_memsize(roaringbitvec *self);

void roaringbitvec_fit(roaringbitvec *self);

void roaringbitvec_set(roaringbitvec *self, size_t pos, bool val);

bool roaringbitvec_get(roaringbitvec *self, size_t pos);

size_t roaringbitvec_rank0(roaringbitvec *self, size_t pos);

size_t roaringbitvec_rank1(roaringbitvec *self, size_t pos);

size_t roaringbitvec_rank(roaringbitvec *self, bool bit, size_t pos);

size_t roaringbitvec_select0(roaringbitvec *self, size_t rank);

size_t roaringbitvec_select1(roaringbitvec *self, size_t rank);

size_t roaringbitvec_select(roaringbitvec *self, bool bit, size_t rank);

void roaringbitvec_fprint(FILE *stream, roaringbitvec *self);

#endif