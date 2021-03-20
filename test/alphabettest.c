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
	char *letters = "0123456789ABCDEF";
	alphabet *ab;
	ab = alphabet_new(size, letters);
	for (size_t i = 0; i < size; i++) {
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


CuSuite *alphabet_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_ab);
	SUITE_ADD_TEST(suite, test_int_ab);
	return suite;
}