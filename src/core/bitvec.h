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

#ifndef bitvec_H
#define bitvec_H

#include <stdbool.h>
#include <stddef.h>

#include "bitsandbytes.h"
#include "dynstr.h"
#include "new.h"

/**
 * @file bitvector.h
 * @author Paulo Fonseca
 *
 * @brief Dynamic bit vector.
 */

typedef struct _bitvec bitvec;


/**
 * @brief Constructor
 */
bitvec *bitvec_new();


/**
 * @brief Constructor
 * @param capacity The initial bit capacity.
 */
bitvec *bitvec_new_with_capacity(size_t capacity);


/**
 * @brief Constructs a new bitvector from a source bitarray.
 *        Source bits are copied internally
 * @param src (no transfer) The source bit array
 * @param len Number of bits to be copied
 */
bitvec *bitvec_new_from_bitarray(const byte_t *src, size_t len);


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
 * @brief Destructor
 */
void bitvec_free(bitvec *bv);


/**
 * @brief Destructor
 */
void bitvec_dispose(void *ptr, const dtor *dt);


/**
 * @brief Returns the (bit) length of the bitvector.
 */
size_t bitvec_len(const bitvec *bv);


/**
 * @brief Returns the bit value stored at a given position @p pos.
 */
bool bitvec_get_bit (const bitvec *bv, size_t pos);


/**
 * @brief Sets the bit at a given position.
 */
void bitvec_set_bit (bitvec *bv, size_t pos, bool bit);


/**
 * @brief Returns the number of positions set to a given @p bit value.
 */
size_t bitvec_count(const bitvec *bv, bool bit);


/**
 * @brief Appends a new @p bit.
 */
void bitvec_append (bitvec *bv, bool bit);


/**
 * @brief Appends @p n new positions with the same @p bit value.
 */
void bitvec_append_n (bitvec *bv, size_t n, bool bit);


/**
 * @brief Concatenates the contents of @p src to @p bv.
 * @param bv (no transfer) The target bitvector
 * @param src (no transfer) The source bitvector
 */
void bitvec_cat (bitvec *bv, const bitvec *src);


/**
 * @brief Frees unused space
 */
void bitvec_trim(bitvec *bv);


/**
 * @brief Detaches and returns the internal raw byte array
 * and destroys the bitvector object
 * @param bv (full transfer) The bitvector to be dismantled.
 */
byte_t *bitvec_detach (bitvec *bv);


/**
 * @brief Writes a string representations of the bitvector to a string buffer
 * @param bytes_per_line Number of bytes per row. Use SIZE_MAX to write
 *        as a single line.
 */
void bitvec_to_string ( const bitvec *bv, dynstr *dest, size_t bytes_per_row);


/**
 * @brief Prints the bitvector do std output.
 * @param bytes_per_row Number of bytes per row
 */
void bitvec_print(const bitvec *bv, size_t bytes_per_row);


#endif