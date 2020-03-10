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

#include "dynstr.h"



void dynstr_test_setup(CuTest *tc)
{
}

void test_dstr_new_from_str(CuTest *tc)
{
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	dynstr *dstr;
	dstr = dynstr_new_from_str(str);
	CuAssertSizeTEquals(tc, 10, dstr_len(dstr));
	CuAssertStrEquals(tc, str, dstr_as_str(dstr));
	dynstr_free(dstr);
}

void test_dstr_get(CuTest *tc)
{
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	dynstr *dstr;
	dstr = dynstr_new_from_str(str);
	for (size_t i=0; i<dstr_len(dstr); i++) {
		CuAssertIntEquals(tc, dstr_get(dstr, i), '0'+(i%10));
	}
	dynstr_free(dstr);
}

void test_dstr_append_char(CuTest *tc)
{
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	dynstr *dstr;
	dstr = dynstr_new_from_str(str);
	for (size_t i=0; i<1000; i++) {
		dstr_append_char(dstr, '0'+(i%10));
	}
	for (size_t i=0; i<dstr_len(dstr); i++) {
		CuAssertIntEquals(tc, dstr_get(dstr, i), '0'+(i%10));
	}
	//printf("final dynstr = %s\ncapacity=%zu\n", dstr_as_str(dstr), dstr_capacity(dstr));
	dynstr_free(dstr);
}

void dynstr_test_teardown(CuTest *tc)
{
}


CuSuite *dynstr_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, dynstr_test_setup);
	SUITE_ADD_TEST(suite, test_dstr_new_from_str);
	SUITE_ADD_TEST(suite, test_dstr_get);
	SUITE_ADD_TEST(suite, test_dstr_append_char);
	SUITE_ADD_TEST(suite, dynstr_test_teardown);
	return suite;
}