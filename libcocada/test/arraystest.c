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

#include "arrays.h"
#include "memdbg.h"

void test_matrix(CuTest *tc)
{
	memdbg_reset();

	for (int m = 0; m < 100; m++) {
		//printf("m = %d\n", m);
		for (int n = 0; n < 100; n++) {
			//printf("n = %d\n", n);
			NEW_MATRIX(mat, int, m, n);

			for (int i = 0; i < m; i++) {
				for (int j = 0; j < n; j++) {
					mat[i][j] = i * j;
				}
			}
			for (int i = 0; i < m; i++) {
				for (int j = 0; j < n; j++)
					CuAssertIntEquals(tc, i * j, mat[i][j]);
			}

			FREE_MATRIX(mat);
		}
	}

	CuAssert(tc, "Memory leak", memdbg_is_empty());
}



CuSuite *arrays_get_test_suite(void)
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_matrix);
	return suite;
}