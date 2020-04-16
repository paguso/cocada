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
#include "xstring.h"


void test_xstring_new(CuTest *tc)
{
	for (size_t len=0; len<1000; len++) {
		xstring *xs = xstring_new_with_len(len, (len>0)?(size_t)DIVCEIL(log2(len), BYTESIZE):0);
		xstring_free(xs);
	}
}


void test_xstring_get_set(CuTest *tc)
{
	for (size_t len=0; len<1000; len++) {
		xstring *xs = xstring_new_with_len(len, (len>0)?(size_t)DIVCEIL(log2(len), BYTESIZE):0);

		for (size_t i=0; i<len; i++) {
			xstr_set(xs, i, i);
		}

		//xstr_print(xs);

		for (size_t i=0; i<len; i++) {
			//printf("xstr[%zu]=%zu\n", i, xstr_get(xs,i));
			CuAssert(tc, "assertion failed", i==xstr_get(xs, i));
		}

		xstring_free(xs);
	}
}

void test_xstring_to_string(CuTest *tc)
{
	size_t l = 1000;
	xstring *xs = xstring_new(2);
	for (xchar_t i=0; i<l; i++) {
		xstr_push(xs, i);
	}
	strbuf *ds = strbuf_new();
	xstr_to_string(xs, ds);
	printf("%s\n",strbuf_as_str(ds));
}

CuSuite *xstring_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_xstring_new);
	SUITE_ADD_TEST(suite, test_xstring_get_set);
	//SUITE_ADD_TEST(suite, test_xstring_to_string);
	return suite;
}