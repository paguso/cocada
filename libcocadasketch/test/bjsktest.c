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
	usize nbits = 16;
	uint64 maxval = 1 << nbits;
	usize ndistinct = maxval >> 3;
	uint64 step = maxval / ndistinct;
	usize n = 1 << 20;

	double eps = 0.1;
	double delta = 0.1;
	BJKST *counter = bjkst_init(nbits, eps, delta);

	for (int i = 0; i < n; i++) {
		uint64 val = rand_range_uint64(0, ndistinct) * step;
		bjkst_process(counter, val);
	}

	uint64 count = bjkst_qry(counter);
	printf("Counter after %zu values = %"PRIu64"\n", n, count);
}


CuSuite *bjkst_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, bjkst_test);
	return suite;
}
