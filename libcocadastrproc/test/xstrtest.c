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
#include "xstr.h"



void test_xstr_get_set(CuTest *tc)
{
	for (size_t len=0; len<1000; len++) {
		xstr *xs = xstr_new_with_capacity(nbytes(len), len);

		for (size_t i=0; i<len; i++) {
			xstr_push(xs, i);
		}

		//xstr_print(xs);

		for (size_t i=0; i<len; i++) {
			//printf("xstr[%zu]=%zu\n", i, xstr_get(xs,i));
			CuAssert(tc, "assertion failed", i==xstr_get(xs, i));
		}

		xstr_free(xs);
	}
}

void test_xstr_to_string(CuTest *tc)
{
	size_t l = 1000;
	xstr *xs = xstr_new(2);
	for (xchar_t i=0; i<l; i++) {
		xstr_push(xs, i);
	}
	strbuf *ds = strbuf_new();
	xstr_to_string(xs, ds);
	printf("%s\n",strbuf_as_str(ds));
}

CuSuite *xstr_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_xstr_get_set);
	//SUITE_ADD_TEST(suite, test_xstr_to_string);
	return suite;
}