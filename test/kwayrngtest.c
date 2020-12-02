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
#include <inttypes.h>

#include "CuTest.h"

#include "arrays.h"
#include "errlog.h"
#include "kwayrng.h"


void test_kwayrng(CuTest *tc)
{
	size_t nbits = 4;
	for (size_t k=0; k<10; k++) {
		kwayrng *rng = kwayrng_new(k, nbits);
		const uint64_t *coef = kwayrng_coefs(rng);
		FPRINT_ARR(stderr, coef, coef, %zu, 0, k, 10);
		DEBUG("%zu-way independent sequence:\n", k);
		for (size_t i=0; i<kwayrng_maxval(rng); i++) {
			DEBUG("X[%zu] = %"PRIu64"\n", i,  kwayrng_next(rng));
		}
	}
}

void test_uniform(CuTest *tc)
{
	size_t *counts = NEW_ARR_0(size_t, 32);
	uint64_t *coefs = NEW_ARR(uint64_t, 4);
	for (size_t k0=0; k0<32; k0++) {
		coefs[0] = k0;
		for (size_t k1=0; k1<32; k1++) {
			coefs[1] = k1;
			for (size_t k2=0; k2<32; k2++) {
				coefs[2] = k2;
				for (size_t k3=0; k3<32; k3++) {
					coefs[3] = k3;
					kwayrng *rng = kwayrng_new_with_coefs(4, coefs,  4);
					for (size_t x=0; x<32; x++) {
						uint64_t val = kwayrng_next(rng);
						counts[(size_t)val]++;
					}
				}
			}
		}
	}
	PRINT_ARR(counts, counts, %zu, 0, 32, 32);
}


CuSuite *kwayrng_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_kwayrng);
	SUITE_ADD_TEST(suite, test_uniform);
	return suite;
}


