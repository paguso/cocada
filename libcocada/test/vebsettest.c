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
#include <stdio.h>

#include "CuTest.h"
#include "errlog.h"
#include "memdbg.h"
#include "vebset.h"
#include "hashset.h"
#include "randutil.h"
#include "vec.h"
#include "order.h"

void vebset_test_add(CuTest *tc)
{
	memdbg_reset();
	rand_reset(123);
	setbuf(stdout, NULL);

	uint n = 10000;
	vec *vec_vals = vec_new_uint32_t();
	for (uint i = 0; i < n; i++) {
		uint32_t x = rand_next() % UINT32_MAX;
		vec_push_uint32_t(vec_vals, x);
	}
	vec_qsort(vec_vals, cmp_uint32_t);
	vec_ins_uint32_t(vec_vals, 0, 0);
	vec_push_uint32_t(vec_vals,  UINT32_MAX);

	vebset *vset = vebset_new();
	for (size_t i = 0; i < vec_len(vec_vals); i++) {
		uint32_t x = vec_get_uint32_t(vec_vals, i);
		CuAssert(tc, "vset contains non-added element", !vebset_contains(vset, x));
		vebset_add(vset, x);
		CuAssert(tc, "vset does not contain added element", vebset_contains(vset, x));
		if (i > 0) {
			uint32_t prev_x = vec_get_uint32_t(vec_vals, i-1);
			for (uint32_t y = prev_x + 1; y < x - ((x-prev_x)/2); y+=((x-prev_x)/2)) {
				if (vebset_contains(vset, y)) {
					CuAssert(tc, "vset contains non-added element", !vebset_contains(vset, y));
				}
			}
		}
	}

	DESTROY_FLAT(vec_vals, vec);
	vebset_free(vset);
	if (!memdbg_is_empty()) {
		//memdbg_print_stats(stdout, true);
		CuAssert(tc, "Memory leak!", memdbg_is_empty());
	}
}


void vebset_test_del(CuTest *tc)
{
	memdbg_reset();
	rand_reset(123);
	setbuf(stdout, NULL);

	uint n = 10000;
	vec *vec_vals = vec_new_uint32_t();
	for (uint i = 0; i < n; i++) {
		uint32_t x = rand_next() % UINT32_MAX;
		vec_push_uint32_t(vec_vals, x);
	}
	vec_push_uint32_t(vec_vals, 0);
	vec_push_uint32_t(vec_vals,  UINT32_MAX);

	vebset *vset = vebset_new();
	size_t size = 0;
	for (size_t i = 0; i < vec_len(vec_vals); i++) {
		uint32_t x = vec_get_uint32_t(vec_vals, i);
		size += vebset_add(vset, x);
	}

	for (size_t i = 0; i < vec_len(vec_vals); i++) {
		uint32_t x = vec_get_uint32_t(vec_vals, i);
		bool has = vebset_contains(vset, x);
		bool del = vebset_del(vset, x);
		CuAssert(tc, "Wrong deletion result", has==del);
		size -= del;
	}
	CuAssertSizeTEquals(tc, 0, size);

	for (size_t i = 0; i < 100; i++) {
		uint32_t x = rand_next() % UINT32_MAX;
		bool del = vebset_del(vset, x);
		CuAssert(tc, "Wrong deletion result", !del);
	}
	CuAssertSizeTEquals(tc, 0, size);


	DESTROY_FLAT(vec_vals, vec);
	vebset_free(vset);
	if (!memdbg_is_empty()) {
		//memdbg_print_stats(stdout, true);
		CuAssert(tc, "Memory leak!", memdbg_is_empty());
	}
}


void vebset_test_succ(CuTest *tc)
{
	memdbg_reset();
	rand_reset(123);
	setbuf(stdout, NULL);

	uint n = 1000;
	vec *vec_vals = vec_new_uint32_t();
	vebset *vset = vebset_new();

	for (uint i = 0; i < n; i++) {
		uint32_t x = rand_next() % UINT32_MAX;
		vec_push_uint32_t(vec_vals, x);
		vebset_add(vset, x);
	}
	vec_qsort(vec_vals, cmp_uint32_t);

	size_t i = 0;
	while ( i < vec_len(vec_vals) ) {
		uint32_t x = vec_get_uint32_t(vec_vals, i);
		int64_t succ = vebset_succ(vset, x);
		int64_t exp_succ;
		while (i < vec_len(vec_vals)
		        && (exp_succ = vec_get_uint32_t(vec_vals, i)) == x) {
			i++;
		}
		if (i == vec_len(vec_vals)) {
			exp_succ = ((int64_t)1 << 32);
		}
		if (succ != exp_succ)
			CuAssert(tc, "Wrong successor", exp_succ == succ);
		if (x + 1 < succ) {
			succ = vebset_succ(vset, x + 1);
			CuAssert(tc, "Wrong successor", exp_succ == succ);
		}
	}

	DESTROY_FLAT(vec_vals, vec);
	vebset_free(vset);
	if (!memdbg_is_empty()) {
		memdbg_print_stats(stdout, true);
		CuAssert(tc, "Memory leak!", memdbg_is_empty());
	}
}


void vebset_test_pred(CuTest *tc)
{
	memdbg_reset();
	rand_reset(123);
	setbuf(stdout, NULL);

	uint n = 1000;
	vec *vec_vals = vec_new_uint32_t();
	vebset *vset = vebset_new();

	for (uint i = 0; i < n; i++) {
		uint32_t x = rand_next() % UINT32_MAX;
		vec_push_uint32_t(vec_vals, x);
		vebset_add(vset, x);
	}
	vec_qsort(vec_vals, cmp_uint32_t);

	int64_t i = vec_len(vec_vals) - 1;
	while ( i > 0 ) {
		uint32_t x = vec_get_uint32_t(vec_vals, i);
		int64_t exp_pred;
		while (i >= 0 && (exp_pred = vec_get_uint32_t(vec_vals, i)) == x) {
			i--;
		}
		if (i < 0) {
			exp_pred = -1;
		}
		//DEBUG("\n\n computing pred(%u)\n", x);
		int64_t pred = vebset_pred(vset, x);
		if (pred != exp_pred)
			CuAssert(tc, "Wrong successor", exp_pred == pred);
		if (pred < x - 1) {
			pred = vebset_pred(vset, x - 1);
			if (pred != exp_pred)
				CuAssert(tc, "Wrong successor", exp_pred == pred);
		}
	}

	DESTROY_FLAT(vec_vals, vec);
	vebset_free(vset);
	if (!memdbg_is_empty()) {
		//memdbg_print_stats(stdout, true);
		CuAssert(tc, "Memory leak!", memdbg_is_empty());
	}
}


void vebset_test_memory(CuTest *tc)
{
	memdbg_reset();
	rand_reset(123);
	setbuf(stdout, NULL);

	uint n = 40000;
	vebset *vset = vebset_new();
	size_t size = 0;
	for (uint i = 0; i < n; i++) {
		uint32_t x = rand_next() % UINT32_MAX;
		size += vebset_add(vset, x);
		if (size % 1000 == 0) {
			DEBUG("size=%zu \t\tmemory=%zu\n", size, memdbg_total());
		}
	}

	vebset_free(vset);
	if (!memdbg_is_empty()) {
		//memdbg_print_stats(stdout, true);
		CuAssert(tc, "Memory leak!", memdbg_is_empty());
	}
}


CuSuite *vebset_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, vebset_test_add);
	SUITE_ADD_TEST(suite, vebset_test_del);
	SUITE_ADD_TEST(suite, vebset_test_succ);
	SUITE_ADD_TEST(suite, vebset_test_pred);
	SUITE_ADD_TEST(suite, vebset_test_memory);
	return suite;

}