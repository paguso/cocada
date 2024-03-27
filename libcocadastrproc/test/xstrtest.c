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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>


#include "CuTest.h"

#include "bitbyte.h"
#include "mathutil.h"
#include "memdbg.h"
#include "xstr.h"
#include "xstrformat.h"



void test_xstr_get_set(CuTest *tc)
{
	for (size_t len = 0; len < 1000; len++) {
		size_t sizeof_char = nbytes(len);
		xstr *xs = xstr_new_with_capacity(sizeof_char, len);
		for (size_t i = 0; i < len; i++) {
			xstr_push(xs, i);
		}
		for (size_t i = 0; i < len; i++) {
			CuAssert(tc, "assertion failed", i == xstr_get(xs, i));
		}
		xstr_free(xs);
	}
}


void print_int16(FILE *stream, xchar_t c)
{
	fprintf(stream, "{%d}", c);
}


void test_xstr_format(CuTest *tc)
{
	memdbg_reset();
	size_t l = 26;
	xstr *xs = xstr_new(1);
	for (xchar_t i = 65; i < 65 + l; i++) {
		xstr_push(xs, i);
	}
	xstrformat *fmt = xstrformat_new_ascii(xs);
	format_fprint(xstrformat_as_format(fmt), stdout);
	xstrformat_free(fmt);
	xstr_free(xs);

	xs = xstr_new(2);
	for (xchar_t i = 65; i < 65 + l; i++) {
		xstr_push(xs, i);
	}
	fmt = xstrformat_new(xs);
	format_fprint(xstrformat_as_format(fmt), stdout);
	xstrformat_free(fmt);
	xstr_free(xs);
	CuAssert(tc, "memory leak.", memdbg_is_empty());
	memdbg_print_stats(stdout, true);
}


CuSuite *xstr_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_xstr_get_set);
	SUITE_ADD_TEST(suite, test_xstr_format);
	return suite;
}