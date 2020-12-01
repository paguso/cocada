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

#ifndef TWUHASH_H
#define TWUHASH_H

#include "coretype.h"

/**
 * @file twuhash.h
 * @brief Two-way universal hash family
 * @author Paulo Fonseca
 *
 * Defines a family of two-way universal hash functions mapping 
 * n-bit to m-bit unsigned int values.
 * The hash is computed as
 * ```
 * h(x) = Ax + B
 * ```
 * where the input `x` is regarded as a column vector of size `n`,
 * `A` is a `m x n` random bit matrix, and `B` is a random column vector
 * of length `m`, with all multiplications and additions modulo-2.
 * Two-way universal means that, for any given input x,
 * and any pair or ouput positions `(i,j)`, with `0<i<j<n`,
 * the probability that the output bits at those positions are
 * either (0,0), (0,1), (1,0), or (1,1), are all equally 1/4, over
 * all possible choices of `A` and `B`.
 */


typedef struct _twuhash twuhash;

/**
 * @brief Creates a new random hash function from the two-way
 * universal family mapping @p in_bits to @p out_bits bits.
 * @warning We must have 0 < @p in_bits, @p out_bits <= 64
 */
twuhash *twuhash_new(byte_t in_bits, byte_t out_bits);

/**
 * @brief Hashes a value @p x with @p h:m->n bits.
 * Only the lowest m bits of @p x are considered.
 */
uint64_t twuhash_hash(twuhash *h, uint64_t x);


#endif