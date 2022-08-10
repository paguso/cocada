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

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "CuTest.h"

#include "minqueue.h"
#include "deque.h"
#include "randutil.h"

typedef struct _minqobj {
	int i;
	double f;
	char c;
} minqobj;

int minqobj_cmp(const void *first, const void *second)
{
	minqobj *f = (minqobj *)first;
	minqobj *s = (minqobj *)second;
	if (f->i < s->i) return -1;
	else if (f->i > s->i) return +1;
	else if (f->f < s->f) return -1;
	else if (f->f > s->f) return +1;
	else if (f->c < s->c) return -1;
	else if (f->c > s->c) return +1;
	else return 0;
}


void test_minqueue_push_pop(CuTest *tc)
{
	minqueue *q = minqueue_new(sizeof(minqobj), minqobj_cmp);
	deque *v = deque_new(sizeof(minqobj));

	size_t n = 1000;
	for (int i=0; i<n; i++) {
		int r = rand_range_int(0, n);
		minqobj o = {(int)r, (double)r, (char)r};
		minqueue_push(q, &o);
		deque_push_back(v, &o);
	}
	CuAssertSizeTEquals(tc, minqueue_len(q), n);
	CuAssertSizeTEquals(tc, deque_len(v), n);

	const minqobj *min;
	for (int i=0; i<n/2; i++) {
		min = minqueue_min(q);
		for (int j=0; j<deque_len(v); j++) {
			CuAssert(tc, "Min is not minimmal", minqobj_cmp(min, deque_get(v, j)) <= 0);
		}
		minqueue_remv(q);
		deque_del_front(v);
	}
	CuAssertSizeTEquals(tc, minqueue_len(q), n-(n/2));


	for (int i=0; i<n/2; i++) {
		int r = rand_range_int(0, n);
		minqobj o = {(int)r, (double)r, (char)r};
		minqueue_push(q, &o);
		deque_push_back(v, &o);
	}
	CuAssertSizeTEquals(tc, minqueue_len(q), n);
	CuAssertSizeTEquals(tc, deque_len(v), n);

	for (int i=0; i<n/2; i++) {
		min = minqueue_min(q);
		for (int j=0; j<deque_len(v); j++) {
			CuAssert(tc, "Min is not minimmal", minqobj_cmp(min, deque_get(v, j)) <= 0);
		}
		minqueue_remv(q);
		deque_del_front(v);
	}
	CuAssertSizeTEquals(tc, minqueue_len(q), n-(n/2));

	min = minqueue_min(q);
	size_t nmin = 0;
	minqueue_iter *iter = minqueue_all_min(q);
	for (int j=0; j<deque_len(v); j++) {
		if (minqobj_cmp(min, deque_get(v, j))==0) {
			nmin++;
		}
	}
	CuAssert(tc, "must have at least one min", nmin>0);
	while (minqueue_iter_has_next(iter)) {
		CuAssert(tc, "iterator returns not min", minqobj_cmp(min,
		         minqueue_iter_next(iter))==0);
		nmin--;
	}
	CuAssertSizeTEquals(tc, 0, nmin);
	minqueue_free(iter);

}


CuSuite *minqueue_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_minqueue_push_pop);
	return suite;
}