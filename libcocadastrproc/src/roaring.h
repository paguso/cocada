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
typedef struct _RoaringBitVec RoaringBitVec;

/**
 * @brief Constructs a new roaring bitvector with a given @p length,
 * with all bits initially set to 0.
 */
RoaringBitVec *roaringbitvec_new(uint32 length);


/**
 * @brief Constructs a new roaring bitvector from a raw bitarray
 * @p src, with given @p length.
 */
RoaringBitVec *roaringbitvec_new_from_bitarr(byte *src, uint32 length);


/**
 * @brief Destructor
 */
void roaringbitvec_free(RoaringBitVec *self);


/**
 * @brief Returns the length of the roaring bitvector.
 */
usize roaringbitvec_len(RoaringBitVec *self);


/**
 * @brief Returns the cardinality of the set respresented to a
 * roaring bitvector
 */
usize roaringbitvec_card(RoaringBitVec *self);


/**
 * @brief Returns the number of positions with a given @p bit value.
 */
usize roaringbitvec_count(RoaringBitVec *self, bool bit);


/**
 * @brief Returns the physical size of the roaring bitvector in bytes.
 */
usize roaringbitvec_memsize(RoaringBitVec *self);


/**
 * @brief Optimises the internal physical size by discarding unused memory.
 * This is normally done when no further modification is expected,
 * although it continues to be allowed.
 */
void roaringbitvec_fit(RoaringBitVec *self);


/**
 * @brief Sets the bit at position @p pos to a given boolean value @p val.
 */
void roaringbitvec_set(RoaringBitVec *self, usize pos, bool val);


/**
 * @brief Returns the value of the bit at position @p pos.
 */
bool roaringbitvec_get(RoaringBitVec *self, usize pos);


/**
 * @brief Same as roaringbitvec_rank(self, 0, pos)
 * @see roaringbitvec_rank()
 */
usize roaringbitvec_rank0(RoaringBitVec *self, usize pos);


/**
 * @brief Same as roaringbitvec_rank(self, 1, pos)
 * @see roaringbitvec_rank()
 */
usize roaringbitvec_rank1(RoaringBitVec *self, usize pos);


/**
 * @brief Returns the @p bit-rank of position @p pos, that is
 * the number of positions to the left of @p pos with value
 * @p bit.
 */
usize roaringbitvec_rank(RoaringBitVec *self, bool bit, usize pos);


/**
 * @brief Same as roaringbitvec_select(self, 0, rank)
 * @see roaringbitvec_select()
 */
usize roaringbitvec_select0(RoaringBitVec *self, usize rank);


/**
 * @brief Same as roaringbitvec_select(self, 1, rank)
 * @see roaringbitvec_select()
 */
usize roaringbitvec_select1(RoaringBitVec *self, usize rank);


/**
 * @brief Returns the position `j` with a given @p bit value and
 * @p rank if it exists.
 * Such position `j` has value @p bit and there are exactly @p rank
 * positions with that same  @p bit value to the left of `j`.
 * If no such position exists, the length of the bitvector is returned.
 */
usize roaringbitvec_select(RoaringBitVec *self, bool bit, usize rank);


/**
 * @brief Prints a text representation of the bitvector to
 * the output @p stream.
 */
void roaringbitvec_fprint(FILE *stream, RoaringBitVec *self);

#endif
