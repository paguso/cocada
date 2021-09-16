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
#include "cstrutil.h"
#include "errlog.h"
#include "mathutil.h"
#include "randutil.h"
#include "new.h"
#include "vec.h"


void test_vec_new(CuTest *tc)
{
	vec *v = vec_new_with_capacity(sizeof(int), 10);
	CuAssertSizeTEquals(tc, 0, vec_len(v));
	DESTROY_FLAT(v, vec);
}


void test_vec_app(CuTest *tc)
{
	size_t len = 100;
	vec *v = vec_new(sizeof(short));
	CuAssertSizeTEquals(tc, 0, vec_len(v));

	for (size_t i =0; i<len; i++) {
		short d = i;
		vec_push(v, &d);
		CuAssertSizeTEquals(tc, i+1, vec_len(v));
	}

	for (size_t i =0; i<len; i++) {
		const short *d;
		d = vec_get(v, i);
		//printf("get da[%zu]=%d\n",i,*d);
		CuAssertIntEquals(tc, (int)i, *d);
	}

	DESTROY_FLAT(v, vec);
}


void test_vec_get_cpy(CuTest *tc)
{
	size_t len = 100;
	vec *v = vec_new(sizeof(double));
	CuAssertSizeTEquals(tc, 0, vec_len(v));

	for (size_t i =0; i<len; i++) {
		double d = i;
		vec_push(v, &d);
		CuAssertSizeTEquals(tc, i+1, vec_len(v));
	}

	for (size_t i =0; i<len; i++) {
		double d;
		vec_get_cpy(v, i, &d);
		//printf("get da[%zu]=%f\n",i,d);
		CuAssertDblEquals(tc, (double)i, d, 0.2);
	}

	DESTROY_FLAT(v, vec);
}



void test_vec_set(CuTest *tc)
{
	size_t len = 100;
	vec *v = vec_new(sizeof(int));
	CuAssertSizeTEquals(tc, 0, vec_len(v));

	for (int i =0; i<len; i++) {
		vec_push(v, &i);
		CuAssertSizeTEquals(tc, i+1, vec_len(v));
	}

	for (int i=1; i<len; i+=2) {
		int d = -i;
		vec_set(v, i, &d);
	}

	for (int i =0; i<len; i++) {
		const int *d ;
		d = vec_get(v, i);
		//printf("get da[%zu]=%d\n",i,*d);
		CuAssertIntEquals(tc, (i%2)?-i:i, *d);
	}

	DESTROY_FLAT(v, vec);
}


void test_vec_ins(CuTest *tc)
{
	size_t len = 110;
	vec *v = vec_new(sizeof(double *));
	CuAssertSizeTEquals(tc, 0, vec_len(v));

	for (size_t i=1; i<len; i+=2) {
		double d = i;
		vec_push(v, &d);
	}
	for (size_t i=0; i<len; i+=2) {
		double d = i;
		vec_ins(v, i, &d);
	}
	for (size_t i =0; i<len; i++) {
		const double *d;
		d = vec_get(v, i);
		//printf("get da[%zu]=%f\n",i,*d);
		CuAssertDblEquals(tc, (double)i, *d, 0.2);
	}
	DESTROY_FLAT(v, vec);
}



void test_vec_del(CuTest *tc)
{
	size_t len = 100;
	vec *v = vec_new(sizeof(double *));
	CuAssertSizeTEquals(tc, 0, vec_len(v));

	for (size_t i=0; i<len; i++) {
		double d = i;
		vec_push(v, &d);
	}
	for (size_t i=0; i<len/2; i++) {
		double d;
		vec_pop(v, i, &d);
		//printf(">deleted da[%zu]=%f\n",i,d);
		CuAssertDblEquals(tc, (double)2*i, d, 0.2);
	}
	for (size_t i =0; i<vec_len(v); i++) {
		const double *d;
		d = vec_get(v, i);
		//printf("get da[%zu]=%f\n",i,*d);
		CuAssertDblEquals(tc, (double)(2*i)+1.0, *d, 0.2);
	}
	DESTROY_FLAT(v, vec);
}


void test_vec_swap(CuTest *tc)
{
	size_t len = 100;
	vec *v = vec_new(sizeof(double *));
	CuAssertSizeTEquals(tc, 0, vec_len(v));

	for (size_t i=0; i<len; i++) {
		double d = i;
		vec_push(v, &d);
	}
	for (size_t i=0; i<len/2; i++) {
		vec_swap(v, i, len-1-i);
	}
	for (size_t i =0; i<vec_len(v); i++) {
		const double *d;
		d = vec_get(v, i);
		//printf("get da[%zu]=%f\n",i,*d);
		CuAssertDblEquals(tc, (double)(len-1-i), *d, 0.2);
	}
	DESTROY_FLAT(v, vec);
}


void test_vec_iter(CuTest *tc)
{
	vec *v = vec_new(sizeof(int));

	vec_iter *it = vec_get_iter(v);
	FOREACH_IN_ITER(j, int, vec_iter_as_iter(it)) {
		CuFail(tc, "Vector has no element to iterate.");
	}
	FREE(it);

	int n = 10;
	for (int i=0; i<n; i++) {
		vec_push_int(v, i);
	}
	int i = 0;
	it = vec_get_iter(v);
	FOREACH_IN_ITER(j, int, vec_iter_as_iter(it)) {
		//printf("Iterator[%d]==%d\n", i, *j);
		CuAssertIntEquals(tc, i, *j);
		i++;
	}
	FREE(it);
	DESTROY_FLAT(v, vec);

	char *strings[8] = {"The", "quick", "fox", "jumps", "over", "the", "lazy", "dog"};
	v = vec_new(sizeof(char *));
	n = 8;
	for (int i=0; i<n; i++) {
		vec_push_rawptr(v, cstr_clone(strings[i]));
	}
	i = 0;
	it = vec_get_iter(v);
	FOREACH_IN_ITER(j, char *, vec_iter_as_iter(it)) {
		//printf("Iterator[%d]==%s\n", i, *j);
		CuAssertStrEquals(tc, strings[i], *j);
		i++;
	}
	FREE(it);
	DESTROY(v, finaliser_cons(FNR(vec), finaliser_new_ptr()));
}


typedef struct _triple {
	uint values[3];
} triple;

static size_t triple_key(const void *tp, size_t d)
{
	return ((triple *)tp)->values[2-d%3];
}

static int triple_cmp(const void *p1, const void *p2)
{
	triple *t1 = (triple *)p1;
	triple *t2 = (triple *)p2;
	if (t1->values[0]<t2->values[0]) return -1;
	else if (t1->values[0]>t2->values[0]) return +1;
	else if (t1->values[1]<t2->values[1]) return -1;
	else if (t1->values[1]>t2->values[1]) return +1;
	else if (t1->values[2]<t2->values[2]) return -1;
	else if (t1->values[2]>t2->values[2]) return +1;
	else return 0;
}


void test_vec_radixsort(CuTest *tc)
{
	size_t max_key = 3;
	vec *v;
	v = vec_new(sizeof(triple));
	for (size_t d1=0; d1<max_key; d1++) {
		for (size_t d0=0; d0<max_key; d0++) {
			for (size_t d2=0; d2<max_key; d2++) {
				triple t;
				t.values[0] = d0;
				t.values[1] = d1;
				t.values[2] = d2;
				vec_push(v, &t);
			}
		}
	}

	/*
	printf("Before sort:\n");
	for (size_t i=0; i<vec_len(v); i++)
	{
	    triple *t = (triple *)vec_get(v, i);
	    printf("v[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
	}
	*/

	vec_radixsort(v, &triple_key, 3, max_key);
	/*
	printf("After sort:\n");
	for (size_t i=0; i<vec_len(da); i++)
	{
	    triple *t = (triple *)vec_get(da, i);
	    printf("da[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
	}
	*/
	for (size_t i=0; i<vec_len(v)-1; i++) {
		triple *p = (triple *)vec_get(v, i);
		triple *q = (triple *)vec_get(v, i+1);
		CuAssertIntEquals(tc, -1, triple_cmp(p, q));
	}
}


void test_vec_qsort(CuTest *tc)
{
	size_t max_key = 10000;
	triple *arr = ARR_NEW(triple, max_key);
	for (size_t i=0; i<max_key; i++) {
		triple t;
		t.values[0] = rand_range_int(0, max_key);
		t.values[1] = rand_range_int(0, max_key);
		t.values[2] = rand_range_int(0, max_key);
		arr[i] = t;
	}
	shuffle_arr(arr, max_key, sizeof(triple));
	vec *v = vec_new(sizeof(triple));
	for (size_t i=0; i<max_key; i++) {
		vec_push(v, &arr[i]);
	}
	FREE(arr);

	/*
	DEBUG("Before sort:\n");
	for (size_t i=0; i<vec_len(v); i++)
	{
	    triple *t = (triple *)vec_get(v, i);
	    DEBUG("v[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
	}
	*/

	vec_qsort(v, triple_cmp);

	/*
	DEBUG("After sort:\n");
	for (size_t i=0; i<vec_len(v); i++)
	{
	    triple *t = (triple *)vec_get(v, i);
	    DEBUG("v[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
	}
	*/

	for (size_t i=0; i<vec_len(v)-1; i++) {
		triple *p = (triple *)vec_get(v, i);
		triple *q = (triple *)vec_get(v, i+1);
		CuAssertIntEquals(tc, -1, triple_cmp(p, q));
	}

	DESTROY_FLAT(v, vec);
}



void test_vec_bsearch(CuTest *tc)
{
	vec *v = vec_new(sizeof(int));
	int maxval = 100;
	CuAssertIntEquals(tc, 0, (int)vec_bsearch(v, &maxval, cmp_int));
	for (int i=0; i<maxval; i+=2) {
		for (int j=0; j<i; j++) {
			vec_push_int(v, i);
		}
	}
	for (int i=0; i<maxval; i++) {
		size_t pos = vec_bsearch(v, &i, cmp_int);
		size_t exp_pos = (i>0 && IS_EVEN(i)) ? ((i/2) * ((i/2)-1)) : vec_len(v);
		CuAssertIntEquals(tc, exp_pos, pos);
	}
	FREE(v);
}



typedef struct _vecobj {
	int i;
	double d;
} vobj;

void test_vec_free(CuTest *tc)
{
	size_t n = 10;
	vec *v = vec_new(sizeof(vec *));
	for (size_t i=0; i<n; i++) {
		vec *c = vec_new(sizeof(vobj *));
		for (size_t j=0; j<i; j++) {
			vobj *e = NEW(vobj);
			e->i = (int)i;
			e->d = (double)i;
			vec_push(c, &e);
		}
		CuAssertSizeTEquals(tc, i, vec_len(c));
		vec_push(v, &c);
	}
	CuAssertSizeTEquals(tc, n, vec_len(v));
	DESTROY(v,  finaliser_cons(FNR(vec), finaliser_cons (finaliser_new_ptr(),
	                           finaliser_cons (FNR(vec),
	                                   finaliser_new_ptr())))) ;
}

void test_vec_flat_free(CuTest *tc)
{
	size_t n = 10;
	vec *v = vec_new(vec_sizeof());
	for (size_t i=0; i<n; i++) {
		vec *c = vec_new(sizeof(vobj));
		for (size_t j=0; j<i; j++) {
			vobj e = {(int)i, (double)i};
			vec_push(c, &e);
		}
		CuAssertSizeTEquals(tc, i, vec_len(c));
		vec_push(v, c);
		FREE(c);
	}
	CuAssertSizeTEquals(tc, n, vec_len(v));
	for (size_t i=0; i<n; i++) {
		vec *c = (vec *)vec_get(v,i);
		for (size_t j=0; j<i; j++) {
			vobj e = {(int)i, (double)i};
			vec_push(c, &e);
		}
		CuAssertSizeTEquals(tc, 2*i, vec_len(c));
	}
	CuAssertSizeTEquals(tc, n, vec_len(v));

	DESTROY(v,  finaliser_cons  ( FNR(vec),  finaliser_cons ( FNR(vec),
	                              finaliser_new_empty()  ) ) ) ;

}

CuSuite *vec_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_vec_new);
	SUITE_ADD_TEST(suite, test_vec_app);
	SUITE_ADD_TEST(suite, test_vec_bsearch);
	SUITE_ADD_TEST(suite, test_vec_get_cpy);
	SUITE_ADD_TEST(suite, test_vec_set);
	SUITE_ADD_TEST(suite, test_vec_ins);
	SUITE_ADD_TEST(suite, test_vec_del);
	SUITE_ADD_TEST(suite, test_vec_swap);
	SUITE_ADD_TEST(suite, test_vec_iter);
	SUITE_ADD_TEST(suite, test_vec_radixsort);
	SUITE_ADD_TEST(suite, test_vec_qsort);
	SUITE_ADD_TEST(suite, test_vec_free);
	SUITE_ADD_TEST(suite, test_vec_flat_free);
	return suite;
}