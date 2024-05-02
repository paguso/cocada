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

#ifndef CSRSBITARRAY_H
#define CSRSBITARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "coretype.h"

/**
 * @file csrsbitarr.h
 * @author Paulo Fonseca
 *
 * @brief Combined sampling rank&select static bitarray.
 * (https://www.dcc.uchile.cl/~gnavarro/ps/sea12.1.pdf)
 */

typedef struct _CSRSBitArr CSRSBitArr;


/**
 * @brief Creates a new r&s bitarray with an attached raw bitarray.
 * @param ba The raw bitarray
 * @param len The array length in bits.
 * @warn The source bitarray is attached to the internal representation
 *       of the created r&s bitarray, i.e. a pointer to @p ba is kept
 *       within the r&s bitarray. The proper functioning of the
 *       r&s bitarray will depend on @p ba being left untouched thereafter.
 *       The raw bitarray may be disposed in the r&s bitarray destructor
 *       by setting the free_data parameter to true.
 */
CSRSBitArr *csrsbitarr_new(byte *ba, usize len);


/**
 * @brief Destructor.
 * @param free_data Indicates whether the source raw bitarray should be freed.
 */
void csrsbitarr_free(CSRSBitArr *ba, bool free_data);


/**
 * @brief Returns a reference to the internal bit array.
 */
const byte *csrsbitarr_data(CSRSBitArr *ba);


/**
 * @brief Prints a representations of the bitarray to standard output.
 */
void csrsbitarr_fprint(FILE *stream, CSRSBitArr *ba, usize bytes_per_row);


/**
 * @brief Returns the length of the bitarray.
 */
usize csrsbitarr_len(CSRSBitArr *ba);


/**
 * @brief Returns the bit at a certain position @p pos.
 */
bool csrsbitarr_get(CSRSBitArr *ba, usize pos);


/**
 * @brief Same as csrsbitarr_rank(@p ba, @p pos, 0).
 * @see csrsbitarr_rank
 */
usize csrsbitarr_rank0(CSRSBitArr *ba, usize pos);


/**
 * @brief Same as csrsbitarr_rank(@p ba, @p pos, 1).
 * @see csrsbitarr_rank
 */
usize csrsbitarr_rank1(CSRSBitArr *ba, usize pos);


/**
 * @brief Computes rank_@p bit(@p ba, @p pos) = # positions j<@p pos
 * s.t. @p ba[j]==@p bit, for 0 <= @p pos < @p ba.len. If @pos>= @ba.len
 * returns the total number of positions with value == @p bit.
 */
usize csrsbitarr_rank(CSRSBitArr *ba, usize pos, bool bit);


/**
 * @brief Same as csrsbitarr_select(@p ba, @p rank, 0).
 * @see csrsbitarr_select
 */
usize csrsbitarr_select0(CSRSBitArr *ba, usize rank);


/**
 * @brief Same as csrsbitarr_select(@p ba, @p rank, 1).
 * @see csrsbitarr_select
 */
usize csrsbitarr_select1(CSRSBitArr *ba, usize rank);


/**
 * @brief Computes select_@p bit(@p ba, @p rank) = j s.t.
 * @p ba[j]==@p bit and rank_@p bit(@p ba, j)=@p rank.
 * If no such position exists, return @p ba.len.
 */
usize csrsbitarr_select(CSRSBitArr *ba, usize rank, bool bit);


/**
 * @brief Same as csrsbitarr_pred(@p ba, @p pos, 0).
 * @see csrsbitarr_pred
 */
usize csrsbitarr_pred0(CSRSBitArr *ba, usize pos);


/**
 * @brief Same as csrsbitarr_pred(@p ba, @p pos, 1).
 * @see csrsbitarr_pred
 */
usize csrsbitarr_pred1(CSRSBitArr *ba, usize pos);


/**
 * @brief Returns the rightmost position whose value is @p bit,
 * strictly to the left of @p pos, i.e max{j<pos | @p ba[j]==@p bit}.
 * If no such position exists, returns @p ba.len.
 */
usize csrsbitarr_pred(CSRSBitArr *ba, usize pos, bool bit);


/**
 * @brief Same as csrsbitarr_succ(@p ba, @p pos, 0).
 * @see csrsbitarr_succ
 */
usize csrsbitarr_succ0(CSRSBitArr *ba, usize pos);


/**
 * @brief Same as csrsbitarr_succ(@p ba, @p pos, 1).
 * @see csrsbitarr_succ
 */
usize csrsbitarr_succ1(CSRSBitArr *ba, usize pos);


/**
 * @brief Returns the leftmost position whose value is @p bit,
 * strictly to the right of @p pos, i.e min{j>pos | @p ba[j]==@p bit}.
 * If no such position exists, returns @p ba.len.
 */
usize csrsbitarr_succ(CSRSBitArr *ba, usize pos, bool bit);


#endif
