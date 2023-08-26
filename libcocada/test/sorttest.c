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
#include "arrays.h"
#include "memdbg.h"
#include "order.h"
#include "randutil.h"
#include "range.h"
#include "sort.h"

void test_q_sort(CuTest *tc)
{
	memdbg_reset();
	size_t n = 100;
	size_t typesize = sizeof(int);
	int *arr = (int *)malloc(n * typesize);
	for (size_t i = 0; i < n; i++) {
		arr[i] = rand_range_size_t(0, 100);
	}
	quicksort(arr, n, typesize, cmp_int);
	for (size_t i = 0; i < n - 1; i++) {
		CuAssertTrue(tc, arr[i] <= arr[i + 1]);
	}
	free(arr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());

}

void test_index_q_sort(CuTest *tc)
{
	memdbg_reset();
	size_t n = 100;
	size_t typesize = sizeof(int);
	int *arr = (int *)malloc(n * typesize);
	for (size_t i = 0; i < n; i++) {
		arr[i] = rand_range_size_t(0, 100);
	}
	size_t *idx = index_quicksort(arr, n, typesize, cmp_int);
	for (size_t i = 0; i < n - 1; i++) {
		CuAssertTrue(tc, arr[idx[i]] <= arr[idx[i + 1]]);
	}
	free(arr);
	free(idx);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}

void test_succ(CuTest *tc)
{
	memdbg_reset();
	range_int r = range_arr_new_int(0, 500, 5);
	for (int i = 0; i < 505; i++) {
		size_t idx = succ(r.arr, r.n, sizeof(int), cmp_int, &i);
		if (idx == 0) {
			CuAssertTrue(tc, i <= r.arr[0]);
		}
		else if (0 < idx && idx < r.n) {
			CuAssertTrue(tc, r.arr[idx-1] < i);
			CuAssertTrue(tc, i <= r.arr[idx]);
		}
		else { // no successor
			CuAssertTrue(tc, r.arr[r.n-1] < i);
		}
	}
	free(r.arr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}

void test_strict_succ(CuTest *tc)
{
	memdbg_reset();
	range_int r = range_arr_new_int(0, 500, 5);
	for (int i = 0; i < 505; i++) {
		size_t idx = strict_succ(r.arr, r.n, sizeof(int), cmp_int, &i);
		if (idx == 0) {
			CuAssertTrue(tc, i < r.arr[0]);
		}
		else if (0 < idx && idx < r.n) {
			CuAssertTrue(tc, r.arr[idx-1] <= i);
			CuAssertTrue(tc, i < r.arr[idx]);
		}
		else { // no successor
			CuAssertTrue(tc, r.arr[r.n-1] <= i);
		}
	}
	free(r.arr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_pred(CuTest *tc)
{
	memdbg_reset();
	range_int r = range_arr_new_int(20, 520, 5);
	for (int i = 0; i < 525; i++) {
		size_t idx = pred(r.arr, r.n, sizeof(int), cmp_int, &i);
		if (0 <= idx && idx < r.n-1) {
			CuAssertTrue(tc, r.arr[idx] <= i);
			CuAssertTrue(tc, i < r.arr[idx+1]);
		}
		else if (idx == r.n-1) {
			CuAssertTrue(tc, r.arr[r.n-1] <= i);
		}
		else { // no predecessor
			CuAssertTrue(tc, i < r.arr[0]);
		}
	}
	free(r.arr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}

void test_strict_pred(CuTest *tc)
{
	memdbg_reset();
	range_int r = range_arr_new_int(20, 520, 5);
	for (int i = 0; i < 525; i++) {
		size_t idx = strict_pred(r.arr, r.n, sizeof(int), cmp_int, &i);
		if (0 <= idx && idx < r.n-1) {
			CuAssertTrue(tc, r.arr[idx] < i);
			CuAssertTrue(tc, i <= r.arr[idx+1]);
		}
		else if (idx == r.n-1) {
			CuAssertTrue(tc, r.arr[r.n-1] < i);
		}
		else { // no predecessor
			CuAssertTrue(tc, i <= r.arr[0]);
		}
	}
	free(r.arr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


CuSuite *sort_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_q_sort);
	SUITE_ADD_TEST(suite, test_index_q_sort);
	SUITE_ADD_TEST(suite, test_succ);
	SUITE_ADD_TEST(suite, test_strict_succ);
	SUITE_ADD_TEST(suite, test_pred);
	SUITE_ADD_TEST(suite, test_strict_pred);
	return suite;
}