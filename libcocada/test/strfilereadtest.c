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
#include "memdbg.h"
#include "strread.h"
#include "strfilereader.h"

static char *filename = "__test_strfileread.txt";
static char *line = "0123456789abcdefghijklmnopqrstuvwxyz"
                    "0123456789abcdefghijklmnopqrstuvwxyz"
                    "0123456789abcdefghijklmnopqrstuvwxyz"
                    "0123456789abcdefghijklmnopqrstuvwxyz"
                    "0123456789abcdefghijklmnopqrstuvwxyz\n";
static size_t line_len = 37;
static size_t nlines = 10;

static void test_setup()
{
	line_len = strlen(line);
	FILE *file = fopen(filename, "w");
	for (size_t i = 0; i < nlines; i++) {
		fprintf(file, "%s", line);
	}
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
	int c;
	strread *reader = strfilereader_as_strread(sfr);
	size_t i = 0;
	for (; (c = strread_getc(reader)) != EOF; i++) {
		CuAssertCharEquals(tc, line[i%line_len], (char)c);
	}
	test_teardown();
}


void test_strfileread_read_strbuf(CuTest *tc)
{
	memdbg_reset();
	test_setup();

	FILE *file = fopen(filename, "r");
	strfilereader *sfreader = strfilereader_new(file);
	strread *reader = strfilereader_as_strread(sfreader);
	strbuf *sbuf = strbuf_new();
	for (size_t i = 0; i < nlines; i++) {
		size_t nread = strread_read_strbuf(reader, sbuf);
		CuAssertSizeTEquals(tc, line_len, nread);
	}
	size_t nread = strread_read_strbuf(reader, sbuf);
	CuAssertSizeTEquals(tc, 0, nread);
	fclose(file);

	strfilereader_free(sfreader);
	strbuf_free(sbuf);
	test_teardown();
	CuAssertTrue(tc, memdbg_is_empty());
}

void test_strfileread_read_strbuf_until(CuTest *tc)
{
	memdbg_reset();
	test_setup();

	FILE *file = fopen(filename, "r");
	strfilereader *sfreader = strfilereader_new(file);
	strread *reader = strfilereader_as_strread(sfreader);
	strbuf *sbuf = strbuf_new();
	size_t nread = strread_read_strbuf_until(reader, sbuf, 'a');
	CuAssertSizeTEquals(tc, 10, nread);
	CuAssertCharEquals(tc, 'a', strread_getc(reader));
	nread = strread_read_strbuf_until(reader, sbuf, 'a');
	CuAssertSizeTEquals(tc, 35, nread);
	nread = strread_read_strbuf_until(reader, sbuf, 'a');
	fclose(file);

	strfilereader_free(sfreader);
	strbuf_free(sbuf);
	test_teardown();
	CuAssertTrue(tc, memdbg_is_empty());
}




CuSuite *strfileread_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("strfileread");
	SUITE_ADD_TEST(suite, test_getc);
	SUITE_ADD_TEST(suite, test_strfileread_read_strbuf);
	SUITE_ADD_TEST(suite, test_strfileread_read_strbuf_until);
	return suite;
}




