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

#ifndef KWAYRNG_H
#define KWAYRNG_H

#include <stddef.h>

/**
 * @file kwayrng.h
 * @brief k-way independent random number generator.
 * @author Paulo Fonseca
 * 
 * This generator generates k-way independent random numbers in the field
 * Z/pZ={0,...,p-1} where p is a prime power.
 * The values are generated by evaluating a polynomial with degree in Z/pZ
 * with randomly chosen coefficients.
 * The description of this r.n.g. requires O(k*lg(p)) bits of space to
 * store the polynomial coefficients. In practice, these values are stored
 * as 64-bit unsigned values.
 * @author Paulo Fonseca, Nivan Ferreira Jr
 */

typedef struct _kwayrng kwayrng;

/**
 * @brief Creates a new k-way independent r.n.g. for generating values
 * of @p nbits bits, i.e. the r.n.g will generate a pseudorandom
 * k-way independent sequence of values in Z/pZ where p=2^@p nbits.
 * Each call to kwayrng_next() yields the next value of the sequence.
 * A call to kwayrng_reset() restarts the sequence.
 *
 * @param k The order of independence.
 * @param nbits The size of the generated values in bits. This value
 * should range from 1 to 64.
 */
kwayrng *kwayrng_new(size_t k, size_t nbits);


/**
 * @brief Creates a k-way independent r.n.g. with given polynomial
 * coefficients.
 * @param (**NO TRANSFER**) coefs The polynomial coefficients.
 * Values are considered modulo the appropriate max value
 * p=2^@p nbits.
 */
kwayrng *kwayrng_new_with_coefs(size_t k, uint64_t *coefs, size_t nbits);


/**
 * @brief Destructor
 */
void kwayrng_free(kwayrng *rng);


/**
 * @brief Returns the k-value of the k-way independent r.n.g.
 */
size_t kwayrng_k(kwayrng *rng);


/**
 * @brief Returns the k polynomial coefficients of the r.n.g.
 */
const uint64_t *kwayrng_coefs(kwayrng *rng);


/**
 * @brief Returns the number `p` of distinct values generated by
 * @p rng. This number `p` is always a prime power.
 * @see kwayrng_new
 * @see kwayrng.h
 */
uint64_t kwayrng_maxval(kwayrng *rng);


/**
 * @brief Restarts the sequence of values generated by @p rng.
 */
void kwayrng_reset(kwayrng *rng);


/**
 * @brief Yelds the next random value in the sequence.
 */
uint64_t kwayrng_next(kwayrng *rng);


/**
 * @brief Yelds the @p ith value of the pseudorandom sequence
 * directly without modifying the internal counter.
 */
uint64_t kwayrng_val(kwayrng *rng, uint64_t ith);

#endif