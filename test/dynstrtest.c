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
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"

#include "strbuf.h"



void strbuf_test_setup(CuTest *tc)
{
}

void test_strbuf_new_from_str(CuTest *tc)
{
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	strbuf *dstr;
	dstr = strbuf_new_from_str(str);
	CuAssertSizeTEquals(tc, 10, strbuf_len(dstr));
	CuAssertStrEquals(tc, str, strbuf_as_str(dstr));
	strbuf_free(dstr);
}

void test_strbuf_get(CuTest *tc)
{
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	strbuf *dstr;
	dstr = strbuf_new_from_str(str);
	for (size_t i=0; i<strbuf_len(dstr); i++) {
		CuAssertIntEquals(tc, strbuf_get(dstr, i), '0'+(i%10));
	}
	strbuf_free(dstr);
}

void test_strbuf_append_char(CuTest *tc)
{
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	strbuf *dstr;
	dstr = strbuf_new_from_str(str);
	for (size_t i=0; i<1000; i++) {
		strbuf_append_char(dstr, '0'+(i%10));
	}
	for (size_t i=0; i<strbuf_len(dstr); i++) {
		CuAssertIntEquals(tc, strbuf_get(dstr, i), '0'+(i%10));
	}
	//printf("final strbuf = %s\ncapacity=%zu\n", strbuf_as_str(dstr), strbuf_capacity(dstr));
	strbuf_free(dstr);
}

void strbuf_test_teardown(CuTest *tc)
{
}


CuSuite *strbuf_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, strbuf_test_setup);
	SUITE_ADD_TEST(suite, test_strbuf_new_from_str);
	SUITE_ADD_TEST(suite, test_strbuf_get);
	SUITE_ADD_TEST(suite, test_strbuf_append_char);
	SUITE_ADD_TEST(suite, strbuf_test_teardown);
	return suite;
}