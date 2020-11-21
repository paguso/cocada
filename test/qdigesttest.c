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

#include <stdio.h>


#include "CuTest.h"

#include "qdigest.h"
#include "randutil.h"


void test_qdigest_upd(CuTest *tc)
{
	size_t range = 1 << 10;
	size_t nupd = 4 * range;
	double err = 0.2;
	qdigest *sketch = qdigest_new(range, err);
	for (size_t i=0; i<nupd; i++) {
		size_t val = rand_range_size_t(0, range);
		size_t qty = rand_range_size_t(1, 10);
		qdigest_upd(sketch, val, qty);
		//printf("\n\nInsert #%zu val=%zu qty=%zu:\n\n", i, val, qty);
		//qdigest_print(sketch, stdout);

	}
}



CuSuite *qdigest_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_qdigest_upd);
	return suite;
}