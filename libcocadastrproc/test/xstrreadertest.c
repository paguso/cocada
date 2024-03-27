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

#include "CuTest.h"
#include "memdbg.h"
#include "strread.h"
#include "strreader.h"
#include "xstr.h"
#include "xstrread.h"
#include "xstrreader.h"


void test_xstrreader(CuTest *tc)
{
	memdbg_reset();
	xstr *s = xstr_new_from_arr_cpy("0123456789012345678901234567890123456789", 40,
	                                1);
	xstrreader *xrdr = xstrreader_open(s);
	xchar_wt c;
	c = xstrread_getc(xstrreader_as_xstrread(xrdr));
	CuAssertCharEquals(tc, '0', c);
	size_t nread = xstrread_read_until(xstrreader_as_xstrread(xrdr), NULL, '0');
	CuAssertSizeTEquals(tc, 9, nread);
	c = xstrread_getc(xstrreader_as_xstrread(xrdr));
	CuAssertCharEquals(tc, '0', c);
	xstr *dest = xstr_new(1);
	nread = xstrread_read(xstrreader_as_xstrread(xrdr), dest, 19);
	CuAssertSizeTEquals(tc, 19, xstr_len(dest));
	xstr_clear(dest);
	c = xstrread_getc(xstrreader_as_xstrread(xrdr));
	CuAssertCharEquals(tc, '0', c);
	nread = xstrread_read(xstrreader_as_xstrread(xrdr), dest, 19);
	CuAssertSizeTEquals(tc, 9, xstr_len(dest));
	xstr_free(dest);
	xstrreader_close(xrdr);
	xstr_free(s);
	if (!memdbg_is_empty()) {
		memdbg_print_stats(stderr, true);
	}
	CuAssert(tc, "Memory leak!", memdbg_is_empty());
}

void test_xstrreader_from_strread(CuTest *tc)
{
	memdbg_reset();
	char *s = "0123456789012345678901234567890123456789";
	strreader *srdr = strreader_new(s, 40);
	xstrreader *xrdr = xstrreader_open_strread(strreader_as_strread(srdr));
	xchar_wt c;
	c = xstrread_getc(xstrreader_as_xstrread(xrdr));
	CuAssertCharEquals(tc, '0', c);
	size_t nread = xstrread_read_until(xstrreader_as_xstrread(xrdr), NULL, '0');
	CuAssertSizeTEquals(tc, 9, nread);
	c = xstrread_getc(xstrreader_as_xstrread(xrdr));
	CuAssertCharEquals(tc, '0', c);
	xstr *dest = xstr_new(1);
	nread = xstrread_read(xstrreader_as_xstrread(xrdr), dest, 19);
	CuAssertSizeTEquals(tc, 19, xstr_len(dest));
	xstr_clear(dest);
	c = xstrread_getc(xstrreader_as_xstrread(xrdr));
	CuAssertCharEquals(tc, '0', c);
	nread = xstrread_read(xstrreader_as_xstrread(xrdr), dest, 19);
	CuAssertSizeTEquals(tc, 9, xstr_len(dest));
	xstr_free(dest);
	xstrreader_close(xrdr);
	strreader_free(srdr);
	if (!memdbg_is_empty()) {
		memdbg_print_stats(stderr, true);
	}
	CuAssert(tc, "Memory leak!", memdbg_is_empty());

}


CuSuite *xstrreader_get_test_suite(void)
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_xstrreader);
	SUITE_ADD_TEST(suite, test_xstrreader_from_strread);
	return suite;
}