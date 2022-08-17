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

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"
#include "strread.h"
#include "strfilereader.h"

static char *filename="test_strfileread.txt";

static char *file_content = "acgtacgtacgtacgtacgtacgtacgtacgtacgtacgt";

static void test_setup()
{
	FILE *file = fopen(filename, "w");
	fprintf(file, "%s", file_content);
	fclose(file);
}

static void test_teardown()
{
	remove(filename);
}


void test_getc(CuTest *tc)
{
	test_setup();
	strfilereader *sfr = strfilereader_new_from_path(filename);
	size_t n = strlen(file_content);
	char c;
	for (size_t i=0; i<n; i++) {
		c = strread_getc(strfilereader_as_strread(sfr));
		CuAssertCharEquals(tc, file_content[i], c);
	}
	c = strread_getc(strfilereader_as_strread(sfr));
	CuAssertIntEquals(tc, EOF, c);
	test_teardown();
}



CuSuite *strfileread_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_getc);
	return suite;
}




