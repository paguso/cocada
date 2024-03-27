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

#include "CuTest.h"

#include "bitvec.h"
#include "errlog.h"
#include "fmalg.h"
#include "float.h"
#include "randutil.h"


void test_fmalg(CuTest *tc)
{
	uint64_t maxval = (uint64_t)1 << 32;
	bitvec *ticks = bitvec_new_with_capacity(maxval);
	fmalg *fm  = fmalg_init(maxval, 5, 7);
	uint64_t val = 0, true_count = 0;
	double error;
	for (size_t i = 0; i < 100000; i++) {
		val = rand_range_uint64_t(0, maxval);
		if ( bitvec_get_bit(ticks, val) == 0 ) {
			true_count++;
			bitvec_set_bit(ticks, val, 1);
		}
		fmalg_process(fm, val);
		if (i % 10 == 0) {
			uint64_t f0 = fmalg_query(fm);
			error = abs((double)f0 - (double)true_count) / (double)true_count;
			DEBUG("FM estimate = %"PRIu64" true count = %"PRIu64" error = %f\n", f0,
			      true_count, error);
		}
	}
	fmalg_free(fm);
}


CuSuite *fmalg_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_fmalg);
	return suite;
}