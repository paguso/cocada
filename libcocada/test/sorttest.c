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
	q_sort(arr, n, typesize, cmp_int);
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
	size_t *idx = index_q_sort(arr, n, typesize, cmp_int);
	for (size_t i = 0; i < n - 1; i++) {
		CuAssertTrue(tc, arr[idx[i]] <= arr[idx[i + 1]]);
	}
	free(arr);
	free(idx);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}

CuSuite *sort_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_q_sort);
	SUITE_ADD_TEST(suite, test_index_q_sort);
	return suite;
}