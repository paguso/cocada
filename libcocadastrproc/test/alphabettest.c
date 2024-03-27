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
#include <string.h>

#include "CuTest.h"

#include "alphabet.h"
#include "memdbg.h"


void test_ab(CuTest *tc)
{
	memdbg_reset();
	size_t size = 16;
	char *letters =
	    "0123456789ABCDEF0123456789ABCDEF"; // len=60
	alphabet *ab;
	ab = alphabet_new(strlen(letters), letters);
	CuAssertSizeTEquals(tc, 16, ab_size(ab));
	for (size_t i = 0; i < ab_size(ab); i++) {
		size_t rk = ab_rank(ab, letters[i]);
		CuAssertSizeTEquals(tc, i, rk);
		xchar_t c = ab_char(ab, i);
		CuAssert(tc, "ab_char error", letters[i] == c);
	}
	CuAssertSizeTEquals(tc, size, ab_rank(ab, 'G'));
	alphabet_free(ab);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_int_ab(CuTest *tc)
{
	memdbg_reset();
	size_t size = 16;
	alphabet *ab;
	ab = int_alphabet_new(size);
	for (size_t i = 0; i < size; i++) {
		xchar_t c = (xchar_t)i;
		size_t rk = ab_rank(ab, c);
		CuAssertSizeTEquals(tc, i, rk);
		xchar_t d = ab_char(ab, i);
		CuAssert(tc, "ab_char error", c == d);
	}
	CuAssertSizeTEquals(tc, size, ab_rank(ab, size + 1));
	alphabet_free(ab);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_ab_with_equivs(CuTest *tc)
{
	char *letters[4] = {"aA@0", "bB1", "cCc2", "dD3"};
	memdbg_reset();
	alphabet *ab = alphabet_new_with_equivs(4, letters);

	CuAssertSizeTEquals(tc, 4, ab_size(ab));

	for (size_t r = 0; r < 4; r++) {
		for (size_t j = 0, l = strlen(letters[r]); j < l; j++) {
			CuAssertSizeTEquals(tc, r, ab_rank(ab, letters[r][j]));
		}
	}

	for (char c = 0; c < CHAR_MAX; c++) {
		CuAssert(tc, "rank error", !ab_contains(ab, c) || ab_rank(ab, c) < 4);
	}

	alphabet_free(ab);
	if (!memdbg_is_empty()) {
		memdbg_print_stats(stdout, true);
	}
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}

CuSuite *alphabet_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_ab);
	SUITE_ADD_TEST(suite, test_int_ab);
	SUITE_ADD_TEST(suite, test_ab_with_equivs);
	return suite;
}
