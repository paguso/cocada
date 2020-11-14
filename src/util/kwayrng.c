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

#include <stddef.h>
#include <stdint.h>

#include "arrutil.h"
#include "coretype.h"
#include "errlog.h"
#include "kwayrng.h"
#include "mathutil.h"
#include "new.h"
#include "randutil.h"


struct  _kwayrng {
	size_t k;
	uint64_t maxval;
	uint64_t *coefs;
	uint64_t count;
};


kwayrng *kwayrng_new(size_t k, size_t nbits)
{
	ERROR_ASSERT(nbits<=64, "Maximum number of random bits is 64");
	kwayrng *ret = NEW(kwayrng);
	ret->k = k;
	ret->maxval = (uint64_t)1 << nbits;
	//ret->maxval = prime_succ((1 << nbits) - 1 );
	ret->coefs = NEW_ARR(uint64_t, k);
	for (size_t i=0; i<k; i++) {
		ret->coefs[i] = rand_range_uint64_t(1, ret->maxval);
	}
	ret->count = 0;
	return ret;
}


kwayrng *kwayrng_new_with_coefs(size_t k, uint64_t *coefs, size_t nbits)
{
	ERROR_ASSERT(nbits<64, "Maximum number of random bits is 63");
	kwayrng *ret = NEW(kwayrng);
	ret->k = k;
	//ret->maxval = prime_succ((1 << nbits) - 1 );
	ret->maxval = (uint64_t)1 << nbits;
	ret->coefs = NEW_ARR(uint64_t, k);
	for (size_t i=0; i<k; i++) {
		ret->coefs[i] = coefs[i] % ret->maxval;
	}
	ret->count = 0;
	return ret;
}


void kwayrng_free(kwayrng *rng)
{
	FREE(rng->coefs);
	FREE(rng);
}


size_t kwayrng_k(kwayrng *rng)
{
	return rng->k;
}


const uint64_t *kwayrng_coefs(kwayrng *rng)
{
	return rng->coefs;
}


void kwayrng_reset(kwayrng *rng)
{
	rng->count = 0;
}


uint64_t kwayrng_maxval(kwayrng *rng)
{
	return rng->maxval;
}


uint64_t kwayrng_next(kwayrng *rng)
{
	uint64_t ret = 0;
	for (uint64_t i=0, pow=1; i < rng->k; i++) {
		ret = mod_sum(ret, mod_mult(rng->coefs[i], pow, rng->maxval), rng->maxval);
		pow = mod_mult(pow, rng->count, rng->maxval);
	}
	rng->count = (rng->count + 1) % rng->maxval;
	return ret;
}


uint64_t kwayrng_val(kwayrng *rng, uint64_t ith)
{
	uint64_t ret = 0;
	ith =  ith % rng->maxval;
	for (uint64_t i=0, pow=1; i < rng->k; i++) {
		ret = mod_sum(ret, mod_mult(rng->coefs[i], pow, rng->maxval), rng->maxval);
		pow = mod_mult(pow, ith, rng->maxval);
	}
	return ret;
}