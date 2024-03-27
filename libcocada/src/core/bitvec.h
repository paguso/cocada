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

#ifndef BITVEC_H
#define BITVEC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "bitbyte.h"
#include "strbuf.h"
#include "new.h"
#include "format.h"

/**
 * @file bitvec.h
 * @author Paulo Fonseca
 *
 * @brief Dynamic bit vector
 */

typedef struct _bitvec bitvec;


/**
 * @brief Constructor
 */
bitvec *bitvec_new();


/**
 * @brief Creates a new empty (length = 0) bitvector with a given @p capacity.
 */
bitvec *bitvec_new_with_capacity(size_t capacity);


/**
 * @brief Creates a new array with a given @p length, with all
 * positions set to zero.
 */
bitvec *bitvec_new_with_len(size_t length);


/**
 * @brief Constructs a new bitvector from a source bitarray.
 *        Source bits are copied internally
 * @param src (no transfer) The source bit array
 * @param len Number of bits to be copied
 */
bitvec *bitvec_new_from_bitarr(const byte_t *src, size_t len);


/**
 * @brief Finaliser
 * @see new.h
 */
void bitvec_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Destructor
 */
void bitvec_free(bitvec *bv);


/**
 * @brief Clones a bitvector
 * @param src (no transfer) The source bitvector
 */
bitvec *bitvec_clone(const bitvec *src);


/**
 * @brief Clones the first @p nbits of @p src to a new bitvector
 * @param src (no transfer) The source bitvector
 * @param nbits Number of bits to be copied
 */
bitvec *bitvec_cropped_clone(const bitvec *src, size_t nbits);



/**
 * @brief Fits the bitvector internal buffer to its actual size, i.e.
 * deallocates unused memory.
 */
void bitvec_fit(bitvec *bv);


/**
 * @brief Returns the physical memory used by the bitvector in bytes
 */
size_t bitvec_memsize(bitvec *bv);


/**
 * @brief Returns a pointer to the internar representation of the
 * bitvector as a raw byte array.
 * @warning The returned representation is not meant to be
 * modified directly.
 */
const byte_t *bitvec_as_bytes(const bitvec *bv);


/**
 * @brief Detaches and returns the internal raw byte array
 * and destroys the bitvector object
 * @param bv (full transfer) The bitvector to be dismantled.
 */
byte_t *bitvec_detach (bitvec *bv);


/**
 * @brief Returns the (bit) length of the bitvector.
 */
size_t bitvec_len(const bitvec *bv);


/**
 * @brief Returns the bit value stored at a given position @p pos.
 */
bool bitvec_get_bit (const bitvec *bv, size_t pos);


/**
 * @brief Returns the number of positions set to a given @p bit value.
 */
size_t bitvec_count(const bitvec *bv, bool bit);


/**
 * @brief Returns the number of positions set to a given @p bit value
 * in the subvector @p bv[@p from: @p to].
 */
size_t bitvec_count_range(const bitvec *bv, bool bit, size_t from, size_t to);



size_t bitvec_select(const bitvec *bv, bool bit, size_t rank);


/**
 * @brief Sets the bit at a given position.
 */
void bitvec_set_bit (bitvec *bv, size_t pos, bool bit);


/**
 * @brief Appends a new @p bit.
 */
void bitvec_push (bitvec *bv, bool bit);


/**
 * @brief Appends @p n new positions with the same @p bit value.
 */
void bitvec_push_n (bitvec *bv, size_t n, bool bit);


/**
 * @brief Concatenates the contents of @p src to @p bv.
 * @param bv (no transfer) The target bitvector
 * @param src (no transfer) The source bitvector
 */
void bitvec_cat (bitvec *bv, const bitvec *src);



/**
 * @brief Writes a string representations of the bitvector to a string buffer
 * @param bytes_per_line Number of bytes per row. Use SIZE_MAX to write
 *        as a single line.
 * @deprecated Use the bitvec_format interface
 * @see bitvec_get_format()
 */
void bitvec_to_string ( const bitvec *bv, strbuf *dest, size_t bytes_per_row);


/**
 * @brief Prints the bitvector do std output.
 * @param bytes_per_row Number of bytes per row
 * @deprecated Use the bitvec_format interface
 * @see bitvec_get_format()
 */
void bitvec_print(FILE *stream, const bitvec *bv, size_t bytes_per_row);


typedef struct _bitvec_format bitvec_format;


DECL_TRAIT(bitvec_format, format);


/**
 * @brief Returns a default formatter for printing the bitvector content.
 * @see format.h
 */
bitvec_format *bitvec_get_format(bitvec *self, uint bytes_per_row);


/**
 * @brief Bitvector formatter destructor
 */
void bitvec_format_free(bitvec_format *self);


#endif