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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"

#include "cstrutil.h"



void test_sizet_to_cstr(CuTest *tc)
{
	size_t max = ~0x0;
	char str[100];
	memset(str, 0, 100);
	for (size_t n=max-100; n<=max; n++) {
		uint_to_cstr(str, n, 'b');
		//printf ("binary  n = %s\n", str);
		uint_to_cstr(str, n, 'o');
		//printf ("octal   n = %s\n", str);
		uint_to_cstr(str, n, 'd');
		//printf ("decimal n = %s\n", str);
		uint_to_cstr(str, n, 'h');
		//printf ("hexadec n = %s\n", str);
		if (n==max) break;
	}
}


CuSuite *cstrutil_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_sizet_to_cstr);
	return suite;
}