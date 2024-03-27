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

#include "new.h"
#include "memdbg.h"
#include "range.h"


void test_range_new(CuTest *tc)
{
	memdbg_reset();
	range_int rng0 = range_arr_new_int(0, 0, 0);
	CuAssertSizeTEquals(tc, 0, rng0.n);
	FREE(rng0.arr);

	range_int rng1 = range_arr_new_int(0, 10, 1);
	int arr1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	CuAssertSizeTEquals(tc, 10, rng1.n);
	for (int i = 0; i < 10; i++)
		CuAssertIntEquals(tc, arr1[i], rng1.arr[i]);
	FREE(rng1.arr);

	range_int rng2 = range_arr_new_int(0, 10, 2);
	int arr2[5] = {0, 2, 4, 6, 8};
	CuAssertSizeTEquals(tc, 5, rng2.n);
	for (int i = 0; i < 5; i++)
		CuAssertIntEquals(tc, arr2[i], rng2.arr[i]);
	FREE(rng2.arr);

	range_int rng3 = range_arr_new_int(0, 10, 3);
	int arr3[4] = {0, 3, 6, 9};
	CuAssertSizeTEquals(tc, 4, rng3.n);
	for (int i = 0; i < 4; i++)
		CuAssertIntEquals(tc, arr3[i], rng3.arr[i]);
	FREE(rng3.arr);

	range_int rng4 = range_arr_new_int(0, 10, 10);
	int arr4[1] = {0};
	CuAssertSizeTEquals(tc, 1, rng4.n);
	for (int i = 0; i < 1; i++)
		CuAssertIntEquals(tc, arr4[i], rng4.arr[i]);
	FREE(rng4.arr);

	range_int rng5 = range_arr_new_int(0, 10, -1);
	CuAssertSizeTEquals(tc, 0, rng5.n);
	FREE(rng5.arr);

	range_int drng0 = range_arr_new_int(-10, -10, 0);
	CuAssertSizeTEquals(tc, 0, drng0.n);
	FREE(drng0.arr);

	range_int drng1 = range_arr_new_int(0, -10, -1);
	int darr1[10] = {0, -1, -2, -3, -4, -5, -6, -7, -8, -9};
	CuAssertSizeTEquals(tc, 10, drng1.n);
	for (int i = 0; i < 10; i++)
		CuAssertIntEquals(tc, darr1[i], drng1.arr[i]);
	FREE(drng1.arr);

	range_int drng2 = range_arr_new_int(0, -10, -2);
	int darr2[5] = {0, -2, -4, -6, -8};
	CuAssertSizeTEquals(tc, 5, drng2.n);
	for (int i = 0; i < 5; i++)
		CuAssertIntEquals(tc, darr2[i], drng2.arr[i]);
	FREE(drng2.arr);

	range_int drng3 = range_arr_new_int(0, -10, -3);
	int darr3[4] = {0, -3, -6, -9};
	CuAssertSizeTEquals(tc, 4, drng3.n);
	for (int i = 0; i < 4; i++)
		CuAssertIntEquals(tc, darr3[i], drng3.arr[i]);
	FREE(drng3.arr);

	range_int drng4 = range_arr_new_int(0, -10, -10);
	int darr4[1] = {0};
	CuAssertSizeTEquals(tc, 1, drng4.n);
	for (int i = 0; i < 1; i++)
		CuAssertIntEquals(tc, darr4[i], drng4.arr[i]);
	FREE(drng4.arr);

	range_int drng5 = range_arr_new_int(0, -10, 1);
	CuAssertSizeTEquals(tc, 0, drng5.n);
	FREE(drng5.arr);

	CuAssert(tc, "Memory leak!", memdbg_is_empty());
}

CuSuite *range_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_range_new);
	return suite;
}