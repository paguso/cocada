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
#include "errlog.h"
#include "memdbg.h"



void test_sizet_to_cstr(CuTest *tc)
{
	memdbg_reset();
	size_t max = ~0x0;
	char str[100];
	memset(str, 0, 100);
	for (size_t n = max - 100; n <= max; n++) {
		uint_to_cstr(str, n, 'b');
		DEBUG("binary  n = %s\n", str);
		uint_to_cstr(str, n, 'o');
		DEBUG("octal   n = %s\n", str);
		uint_to_cstr(str, n, 'd');
		DEBUG("decimal n = %s\n", str);
		uint_to_cstr(str, n, 'h');
		DEBUG("hexadec n = %s\n", str);
		if (n == max) break;
	}
	ERROR_ASSERT(memdbg_is_empty(), "Memory leak.");
}


void test_cstr_join(CuTest *tc)
{
	memdbg_reset();
	char *s1 = "sunday";
	char *s2 = "monday";
	char *s3 = "tuesday";
	char *s4 = "wednesday";
	char *s5 = "thursday";
	char *s6 = "friday";
	char *s7 = "saturday";
	char *sep = " then ";
	char *res = cstr_join(sep, 0, s1, s2, s3, s4, s5, s6, s7);
	DEBUG("Joined 0 = %s\n", res);
	CuAssertStrEquals(tc, "", res);
	FREE(res);
	res = cstr_join(sep, 1, s1, s2, s3, s4, s5, s6, s7);
	DEBUG("Joined 1 = %s\n", res);
	CuAssertStrEquals(tc, s1, res);
	FREE(res);
	res = cstr_join(sep, 2, s1, s2, s3, s4, s5, s6, s7);
	DEBUG("Joined 2 = %s\n", res);
	CuAssertStrEquals(tc, "sunday then monday", res);
	FREE(res);
	res = cstr_join(sep, 7, s1, s2, s3, s4, s5, s6, s7);
	DEBUG("Joined 7 = %s\n", res);
	CuAssertStrEquals(tc,
	                  "sunday then monday then tuesday then wednesday then thursday then friday then saturday",
	                  res);
	FREE(res);
	ERROR_ASSERT(memdbg_is_empty(), "Memory leak.");
}



CuSuite *cstrutil_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_sizet_to_cstr);
	SUITE_ADD_TEST(suite, test_cstr_join);
	return suite;
}