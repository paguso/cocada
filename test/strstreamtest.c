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
#include "strstream.h"

static char *str;
static size_t slen;
static char *filename;
static FILE *file;

static void strstream_test_setup(CuTest *tc)
{
	str = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
	slen = strlen(str);
	filename = "test_strstream.txt";
	file = fopen(filename, "w");
	fprintf(file, "%s", str);
	fclose(file);
	file = fopen(filename, "r");
}

static void strstream_test_teardown(CuTest *tc)
{
	fclose(file);
	remove(filename);
}

void test_strstream_getc(CuTest *tc)
{
	strstream_test_setup(tc);

	strstream *ssst, *fsst;
	size_t i;
	int c;

	ssst = strstream_open_str(str, slen);
	fsst = strstream_open_file(filename);

	for (size_t k=0; k<5; k++) {
		i=0;
		for (i=0; (c=strstream_getc(ssst)) != EOF; i++) {
			//printf ("s i=%zu c=%c\n", i, (char)c);
			CuAssertTrue(tc, str[i]==(char)c);
		}
		CuAssertSizeTEquals(tc, slen, i);
		strstream_reset(ssst);

		i=0;
		for (i=0; (c=strstream_getc(fsst)) != EOF; i++) {
			//printf ("f i=%zu c=%c\n", i, (char)c);
			CuAssertTrue(tc, str[i]==(char)c);
		}
		CuAssertSizeTEquals(tc, slen, i);
		strstream_reset(fsst);
	}

	strstream_close(ssst);
	strstream_close(fsst);

	strstream_test_teardown(tc);
}

void test_strstream_reads(CuTest *tc)
{
	strstream_test_setup(tc);

	strstream *ssst, *fsst;
	size_t dlen = 7;
	char *dest = cstr_new(slen);
	char *exp = cstr_new(slen);
	size_t i,n;

	ssst = strstream_open_str(str, slen);
	fsst = strstream_open_file(filename);

	for (size_t k=0; k<5; k++) {
		for (i=0; (n=strstream_reads(ssst, dest, dlen)); i+=n) {
			exp = strncpy(dest, str+i, n);
			//printf ("s read=%s exp=%s\n", dest, exp);
			CuAssertStrEquals(tc, exp, dest);
		}
		CuAssertSizeTEquals(tc, slen, i);
		strstream_reset(ssst);

		for (i=0; (n=strstream_reads(fsst, dest, dlen)); i+=n) {
			exp = strncpy(dest, str+i, n);
			//printf ("f read=%s exp=%s\n", dest, exp);
			CuAssertStrEquals(tc, exp, dest);
		}
		CuAssertSizeTEquals(tc, slen, i);
		strstream_reset(fsst);
	}

	strstream_close(ssst);
	strstream_close(fsst);

	strstream_test_teardown(tc);
}



CuSuite *strstream_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_strstream_getc);
	SUITE_ADD_TEST(suite, test_strstream_reads);
	return suite;
}