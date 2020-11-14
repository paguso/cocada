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

#include "arrutil.h"
#include "errlog.h"
#include "new.h"
#include "randutil.h"
#include "twuhash.h"


struct _twuhash {
	byte_t in_bits, out_bits;
	uint64_t *A;
	uint64_t B;
};


twuhash *twuhash_new(byte_t in_bits, byte_t out_bits)
{
	ERROR_ASSERT( 0 < in_bits && in_bits <= 64 && 0 < out_bits && out_bits <= 64,
	              "twuhash must hash m to n bits where 0 < m,n <= 64");
	twuhash *ret = NEW(twuhash);
	ret->in_bits = in_bits;
	ret->out_bits = out_bits;
	ret->A = NEW_ARR_0(uint64_t, out_bits);
	for (size_t i = 0; i < out_bits; i++) {
		ret->A[i] = rand_next();
		ret->A[i] >>= (64 - out_bits);
	}
	ret->B = rand_next();
	ret->B >>= (64 - in_bits);
	return ret;
}



uint64_t twuhash_hash(twuhash *h, uint64_t x)
{
	uint64_t ret = 0;
	for (size_t k = 0; k < h->out_bits; k++) {
		ret <<= 1;
		ret |= (uint64_bitcount1(h->A[k] & x) & 1);
	}
	ret ^= (h->B);
	return ret;
}