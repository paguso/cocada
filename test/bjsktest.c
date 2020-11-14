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

#include <inttypes.h>
#include <stdio.h>

#include "CuTest.h"

#include "bjkst.h"
#include "randutil.h"


void bjkst_test(CuTest *tc)
{
	size_t nbits = 16;
	uint64_t maxval = 1 << nbits;
	size_t ndistinct = maxval >> 3;
	uint64_t step = maxval / ndistinct;
	size_t n = 1 << 20;
	
	double eps = 0.1;
	double delta = 0.1;
	bjkst *counter = bjkst_init(nbits, eps, delta);

	for (int i=0; i<n; i++) {
		uint64_t val = rand_range_uint64_t(0, ndistinct) * step;
		bjkst_process(counter, val);
	}

	uint64_t count = bjkst_qry(counter);
	printf("Counter after %zu values = %"PRIu64"\n", n, count);
}


CuSuite *bjkst_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, bjkst_test);
	return suite;
}