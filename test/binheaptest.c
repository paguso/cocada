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
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"

#include "arrays.h"
#include "binheap.h"
#include "randutil.h"
#include "order.h"



int cmp_ptr_dbl(const void *p1, const void *p2)
{
	double d1 = (*((double **)p1))[0];
	double d2 = (*((double **)p2))[0];
	if ( d1 < d2 )
		return -1;
	else if ( d1 == d2 )
		return 0;
	else
		return +1;
}


/*
 * Tests storing pointers to double
 */
void test_binheap_push_pop(CuTest *tc)
{
	size_t len = 10;
	binheap *maxheap = binheap_new(&cmp_ptr_dbl, sizeof(double *));
	CuAssertSizeTEquals(tc, 0, binheap_size(maxheap));

	double rv[len];
	for (size_t i=0; i<len; ++i) rv[i]=(double)i;
	shuffle_arr(rv, len, sizeof(double));


	for (size_t i =0; i<len; i++) {
		double *d = &rv[i];
		binheap_push(maxheap, &d);
		CuAssertSizeTEquals(tc, i+1, binheap_size(maxheap));
	}


	for (size_t i =0; i<len; i++) {
		double *d;
		binheap_pop(maxheap, &d);
		//printf("maxheap #%zu = %f\n",i,d);
		CuAssertSizeTEquals(tc, len-i-1, binheap_size(maxheap));
		CuAssertDblEquals(tc, (double)(len-i-1), *d, 0.1);
	}

	DESTROY(maxheap, DTOR(binheap));
}


void test_binheap_push_pop_int(CuTest *tc)
{
	size_t len = 10;
	binheap *maxheap = binheap_new(cmp_int, sizeof(int));
	CuAssertSizeTEquals(tc, 0, binheap_size(maxheap));

	int rv[len];
	for (size_t i=0; i<len; i++) rv[i]=i;
	shuffle_arr(rv, len, sizeof(int));

	for (size_t i =0; i<len; i++) {
		binheap_push_int(maxheap, rv[i]);
		CuAssertSizeTEquals(tc, i+1, binheap_size(maxheap));
	}


	for (size_t i =0; i<len; i++) {
		int d;
		d = binheap_pop_int(maxheap);
		//printf("maxheap #%zu = %d\n",i,d);
		CuAssertSizeTEquals(tc, len-i-1, binheap_size(maxheap));
		CuAssertIntEquals(tc, (int)(len-i-1), d);
	}

	DESTROY(maxheap, DTOR(binheap));
}



CuSuite *binheap_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_binheap_push_pop);
	SUITE_ADD_TEST(suite, test_binheap_push_pop_int);
	return suite;
}