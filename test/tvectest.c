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
#include "cstrutil.h"

#include "tvec.h"
#include "randutil.h"

typedef struct {
	int a;
	float b;
} MyType;


static int MyType_cmp(const void *pl, const void *pr) {
	MyType l = *((MyType *)pl);
	MyType r = *((MyType *)pr);
	if (l.a < r.a) return -1;
	else if (l.a > r.a) return +1;
	else if (l.b < r.b) return -1;
	else if (l.b > r.b) return +1;
	else return 0;
}

TVEC_DECL(MyType)
TVEC_IMPL(MyType)



void tvec_test_push(CuTest *tc)
{
	tvec_int *v = tvec_int_new();

	int n = 100;
	for (int i=0; i<n; i++) {
		tvec_int_push(v, i);
	}

	for (int i=0; i<n; i++) {
		int j = tvec_int_get(v, i);
		CuAssertIntEquals(tc, i, j);
	}

	tvec_cstr *vc = tvec_cstr_new();
	cstr nb = cstr_new(10);

	n = 100;
	for (int i=0; i<n; i++) {
		tvec_cstr_push(vc, cstr_clone(uint_to_cstr(nb, i, 10)));
	}

	for (int i=0; i<n; i++) {
		nb = uint_to_cstr(nb, i, 10);
		char *j = tvec_cstr_get(vc, i);
		CuAssertStrEquals(tc, nb, j);
	}

	tvec(MyType) *mtv = tvec_MyType_new();

	n = 100;
	for (int i=0; i<n; i++) {
		MyType elt = {i, (float)i};
		tvec_MyType_push(mtv, elt);
	}

	for (int i=0; i<n; i++) {
		MyType elt = tvec_MyType_get(mtv, i);
		CuAssertIntEquals(tc, i, elt.a);
		CuAssertDblEquals(tc, (double)i, (double)elt.b, 0.1);
	}
}


void tvec_test_qsort(CuTest *tc) 
{
	tvec_MyType *v = tvec_MyType_new();
	size_t n = 100;
	for (int i=0; i<n; i++) {
		MyType x;
		x.a = rand_range_int(0, n);
		x.b = (float)rand_range_int(0, n);
		tvec_MyType_push(v, x);
	}
	tvec_MyType_qsort(v, MyType_cmp);
	MyType prev = tvec_MyType_get(v, 0);
	DEBUG("cur[0]=(%d,%f)\n", prev.a, prev.b);
	for (int i=1; i<n; i++) {
		MyType cur = tvec_MyType_get(v, i);
		DEBUG("cur[%d]=(%d, %f)\n", i, cur.a, cur.b);
		CuAssert(tc, "out or order\n", MyType_cmp(&prev, &cur)<=0);
		prev = cur;
	}
}




CuSuite *tvec_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, tvec_test_push);
	SUITE_ADD_TEST(suite, tvec_test_qsort);
	return suite;
}
