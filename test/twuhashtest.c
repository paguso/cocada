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

#include "CuTest.h"

#include "arrays.h"
#include "errlog.h"
#include "new.h"
#include "randutil.h"
#include "twuhash.h"



void twuhash_test_hash(CuTest *tc)
{
	byte_t in_bits = 32;
	byte_t out_bits = 16;
	twuhash *h = twuhash_new(in_bits, out_bits);
	uint64_t *sample;
	size_t samplesize = 1 << 20;
	sample = NEW_ARR(uint64_t, samplesize);
	for (size_t i = 0; i < samplesize; i++) {
		uint64_t x = rand_next();
		x >>= (64 - in_bits);
		sample[i] = twuhash_hash(h, x);
		DEBUG("hash(%"PRIu64")=%"PRIu64"\n", x, sample[i] );
	}
	uint64_t mask_i = 1;
	for (size_t i = 0; i < out_bits; i++) {
		uint64_t mask_j = (1 << (i+1));
		for (size_t j = i+1; j < out_bits; j++) {
			size_t counts[4] = {0,0,0,0};
			for (size_t k=0; k<samplesize; k++) {
				unsigned char pair = ((mask_i & sample[k]) > 0);
				pair <<= 1;
				pair |= ((mask_j & sample[k]) > 0);
				counts[pair]++;
			}

			DEBUG("i=%zu j=%zu #(0,0)=%zu #(0,1)=%zu #(1,0)=%zu #(1,1)=%zu\n",
			      i, j, counts[0], counts[1], counts[2], counts[3]);

			mask_j <<= 1;
		}
		mask_i <<= 1;
	}
	FREE(sample);
}



CuSuite *twuhash_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, twuhash_test_hash);
	return suite;
}