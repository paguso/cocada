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
#include "memdbg.h"
#include "new.h"
#include "segtree.h"
#include "vec.h"

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
	memdbg_reset();
	int zero = 0;
	size_t max_range = 100;
	for (size_t range = 0; range < max_range; range++) {
		segtree *tree = segtree_new(range, sizeof(int), segtree_merge_sum_int, &zero);
		for (size_t i = 0; i < range; i++) {
			int v = (int)i;
			segtree_upd(tree, i, &v);
		}
		for (size_t i = 0; i < range; i++) {
			int v = *((int *)segtree_qry(tree, i));
			CuAssertIntEquals(tc, i, v);
		}
		for (size_t l = 0; l < range; l++) {
			for (size_t r = l; r < range; r++) {
				int v, ex = 0;
				for (size_t i = l; i < r; ex += (i++));
				segtree_range_qry(tree, l, r, &v);
				CuAssertIntEquals(tc, ex, v);
			}
		}
		for (size_t i = 0; i < range; i++) {
			int v = 2 * (int)i;
			segtree_upd(tree, i, &v);
		}
		for (size_t i = 0; i < range; i++) {
			int v = *((int *)segtree_qry(tree, i));
			CuAssertIntEquals(tc, 2 * i, v);
		}
		for (size_t l = 0; l < range; l++) {
			for (size_t r = l; r < range; r++) {
				int v, ex = 0;
				for (size_t i = l; i < r; ex += (i++));
				ex *= 2;
				segtree_range_qry(tree, l, r, &v);
				CuAssertIntEquals(tc, ex, v);
			}
		}
		segtree_free(tree);
	}
	if (!memdbg_is_empty()) DEBUG_EXEC(memdbg_print_stats(stdout, true));
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}

typedef struct {
	int val;
	double dval;
} obj_t;

void merge_obj(const void *left, const void *right, void *dest)
{
	obj_t *l = (obj_t *)left;
	obj_t *r = (obj_t *)right;
	obj_t *d = (obj_t *)dest;
	d->val = l->val + r->val;
	d->dval = l->dval + r->dval;
}


void test_segtree_upd_obj(CuTest *tc)
{
	memdbg_reset();
	obj_t zero =  {.val = 0, .dval = 0};
	size_t max_range = 100;
	for (size_t range = 0; range < max_range; range++) {
		size_t range = 10;
		segtree *tree = segtree_new(range, sizeof(obj_t), merge_obj, &zero);
		for (size_t i = 0; i < range; i++) {
			obj_t v  = (obj_t) {
				.val = (int)i, .dval = (double)i * 2
			};
			segtree_upd(tree, i, &v);
		}
		for (size_t i = 0; i < range; i++) {
			int v = (*((obj_t *)segtree_qry(tree, i))).val;
			CuAssertIntEquals(tc, i, v);
		}
		obj_t v;
		for (size_t l = 0; l < range; l++) {
			for (size_t r = l; r < range; r++) {
				int ex = 0;
				for (size_t i = l; i < r; ex += (i++));
				segtree_range_qry(tree, l, r, &v);
				CuAssertIntEquals(tc, ex, v.val);
			}
		}
		segtree_free(tree);
	}
	if (!memdbg_is_empty()) DEBUG_EXEC(memdbg_print_stats(stdout, true));
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_segtree_range_qry(CuTest *tc)
{
	size_t max_range = 100;
	uint32_t ZERO32 = 0;
	memdbg_reset();
	for (size_t range = 0; range < max_range; range++) {
		segtree *st = segtree_new(range, sizeof(uint32_t), segtree_merge_sum_uint32_t,
		                          &ZERO32);
		uint32_t val = UINT32_MAX / MAX(1, range);
		for (size_t i = 0; i < range; i++) {
			segtree_upd_uint32_t(st, i, val);
		}
		for (size_t l = 0; l <= range; l++) {
			for (size_t r = l; r <= range; r++) {
				uint32_t actual = segtree_range_qry_uint32_t(st, l, r);
				uint32_t expec = (r - l) * val;
				CuAssertULongEquals(tc, expec, actual);
			}
		}
		segtree_free(st);
	}
	if (!memdbg_is_empty()) DEBUG_EXEC(memdbg_print_stats(stdout, true));
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


CuSuite *segtree_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_segtree_upd);
	SUITE_ADD_TEST(suite, test_segtree_upd_obj);
	SUITE_ADD_TEST(suite, test_segtree_range_qry);
	return suite;
}

