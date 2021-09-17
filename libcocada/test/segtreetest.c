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

#include "errlog.h"
#include "mathutil.h"
#include "segtree.h"


void sum_int(const void *left, const void *right, void *dest)
{
	int *res = (int *) dest;
	*res = *((int *)left) + *((int *)right);
}

void max_float(const void *left, const void *right, void *dest)
{
	float *res = (float *) dest;
	*res = MAX(*((float *)left), *((float *)right));

}

void test_segtree_upd(CuTest *tc)
{
	int zero = 0;
	size_t range = 10;
	segtree *tree = segtree_new(range, sizeof(int), segtree_merge_sum_int, &zero);
	for (size_t i=0; i<range; i++) {
		int v = (int)i;
		segtree_upd(tree, i, &v);
	}
	for (size_t i=0; i<range; i++) {
		int v = *((int *)segtree_qry(tree, i));
		CuAssertIntEquals(tc, i, v);
	}
	for (size_t l=0; l<range; l++) {
		for (size_t r=l; r<range; r++) {
			int v, ex=0;
			for (size_t i=l; i<r; ex+=(i++));
			segtree_range_qry(tree, l, r, &v);
			CuAssertIntEquals(tc, ex, v);
		}
	}
	for (size_t i=0; i<range; i++) {
		int v = 2 * (int)i;
		segtree_upd(tree, i, &v);
	}
	for (size_t i=0; i<range; i++) {
		int v = *((int *)segtree_qry(tree, i));
		CuAssertIntEquals(tc, 2*i, v);
	}
	for (size_t l=0; l<range; l++) {
		for (size_t r=l; r<range; r++) {
			int v, ex=0;
			for (size_t i=l; i<r; ex+=(i++));
			ex *= 2;
			segtree_range_qry(tree, l, r, &v);
			CuAssertIntEquals(tc, ex, v);
		}
	}
	segtree_free(tree);
}



CuSuite *segtree_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_segtree_upd);
	return suite;
}

