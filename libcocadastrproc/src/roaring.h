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

/**
 * @brief Roaring bitvector opaque type
 */
typedef struct _roaringbitvec roaringbitvec;

/**
 * @brief Constructs a new roaring bitvector with a given @p length, 
 * with all bits initially set to 0.
 */
roaringbitvec *roaringbitvec_new(uint32_t length);


/**
 * @brief Constructs a new roaring bitvector from a raw bitarray 
 * @p src, with given @p length.
 */
roaringbitvec *roaringbitvec_new_from_bitarr(byte_t *src, uint32_t length);


/**
 * @brief Destructor
 */
void roaringbitvec_free(roaringbitvec *self);


/**
 * @brief Returns the length of the roaring bitvector.
 */
size_t roaringbitvec_len(roaringbitvec *self);


/**
 * @brief Returns the number of positions with a given @p bit value.
 */
size_t roaringbitvec_count(roaringbitvec *self, bool bit);


/**
 * @brief Returns the physical size of the roaring bitvector in bytes.
 */
size_t roaringbitvec_memsize(roaringbitvec *self);


/**
 * @brief Optimises the internal physical size by discarding unused memory.
 * This is normally done when no further modification is expected,
 * although it continues to be allowed.
 */
void roaringbitvec_fit(roaringbitvec *self);


/**
 * @brief Sets the bit at position @p pos to a given boolean value @p val.
 */
void roaringbitvec_set(roaringbitvec *self, size_t pos, bool val);


/**
 * @brief Returns the value of the bit at position @p pos.
 */
bool roaringbitvec_get(roaringbitvec *self, size_t pos);


/**
 * @brief Same as roaringbitvec_rank(self, 0, pos)
 * @see roaringbitvec_rank()
 */
size_t roaringbitvec_rank0(roaringbitvec *self, size_t pos);


/**
 * @brief Same as roaringbitvec_rank(self, 1, pos)
 * @see roaringbitvec_rank()
 */
size_t roaringbitvec_rank1(roaringbitvec *self, size_t pos);


/**
 * @brief Returns the @p bit-rank of position @p pos, that is
 * the number of positions to the left of @p pos with value
 * @p bit.
 */
size_t roaringbitvec_rank(roaringbitvec *self, bool bit, size_t pos);


/**
 * @brief Same as roaringbitvec_select(self, 0, rank)
 * @see roaringbitvec_select()
 */
size_t roaringbitvec_select0(roaringbitvec *self, size_t rank);


/**
 * @brief Same as roaringbitvec_select(self, 1, rank)
 * @see roaringbitvec_select()
 */
size_t roaringbitvec_select1(roaringbitvec *self, size_t rank);


/**
 * @brief Returns the position `j` with a given @p bit value and 
 * @p rank if it exists.
 * Such position `j` has value @p bit and there are exactly @p rank 
 * positions with that same  @p bit value to the left of `j`. 
 * If no such position exists, the length of the bitvector is returned.
 */
size_t roaringbitvec_select(roaringbitvec *self, bool bit, size_t rank);


/**
 * @brief Prints a text representation of the bitvector to
 * the output @p stream. 
 */
void roaringbitvec_fprint(FILE *stream, roaringbitvec *self);

#endif