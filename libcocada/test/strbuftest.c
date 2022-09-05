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

#include "errlog.h"
#include "memdbg.h"
#include "strbuf.h"



void strbuf_test_setup(CuTest *tc)
{
}


void test_strbuf_new_from_str(CuTest *tc)
{
	memdbg_reset();
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	strbuf *dstr;
	dstr = strbuf_new_from_str(str, strlen(str));
	CuAssertSizeTEquals(tc, 10, strbuf_len(dstr));
	CuAssertStrEquals(tc, str, strbuf_as_str(dstr));
	free(str);
	DESTROY_FLAT(dstr, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_get(CuTest *tc)
{
	memdbg_reset();
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	strbuf *dstr;
	dstr = strbuf_new_from_str(str, strlen(str));
	for (size_t i=0; i<strbuf_len(dstr); i++) {
		CuAssertIntEquals(tc, strbuf_get(dstr, i), '0'+(i%10));
	}
	free(str);
	strbuf_free(dstr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_append_char(CuTest *tc)
{
	memdbg_reset();
	char *str = (char *)malloc(11*sizeof(char));
	strcpy(str, "0123456789");
	str[10]='\0';
	strbuf *dstr;
	dstr = strbuf_new_from_str(str, strlen(str));
	for (size_t i=0; i<1000; i++) {
		strbuf_append_char(dstr, '0'+(i%10));
	}
	for (size_t i=0; i<strbuf_len(dstr); i++) {
		CuAssertIntEquals(tc, strbuf_get(dstr, i), '0'+(i%10));
	}
	//printf("final strbuf = %s\ncapacity=%zu\n", strbuf_as_str(dstr), strbuf_capacity(dstr));
	free(str);
	strbuf_free(dstr);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_append(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb = strbuf_new_with_capacity(0);
	strbuf_nappend(sb, "", 0);
	CuAssertSizeTEquals(tc, strbuf_len(sb), 0);
	strbuf_nappend(sb, "The", 3);
	CuAssertSizeTEquals(tc, strbuf_len(sb), 3);
	CuAssertStrEquals(tc, strbuf_as_str(sb), "The");
	strbuf_nappend(sb, " quick brown fox", 16);
	CuAssertStrEquals(tc, strbuf_as_str(sb), "The quick brown fox");
	strbuf_nappend(sb, " jumps over the lazy dog.", 25);
	CuAssertStrEquals(tc, strbuf_as_str(sb),
	                  "The quick brown fox jumps over the lazy dog.");
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_insert(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb = strbuf_new_with_capacity(0);
	strbuf_append_char(sb, '.');
	strbuf_ins(sb, 0, "The", 3);
	strbuf_ins(sb, 3, " the lazy dog", 13);
	strbuf_ins(sb, 3, " quick brown fox", 16);
	strbuf_ins(sb, 19, " jumps over", 11);
	strbuf_ins(sb, 19, " jumps over", 0); // do nothing
	CuAssertStrEquals(tc, strbuf_as_str(sb),
	                  "The quick brown fox jumps over the lazy dog.");
	CuAssertSizeTEquals(tc, 44, strbuf_len(sb));
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_cut(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb = strbuf_new_from_str("The quick brown fox jumps over the lazy dog.",
	                                 44);
	char dest[40];
	// "The quick brown fox jumps over the lazy dog."
	strbuf_cut(sb, 0, 4, dest);
	CuAssertSizeTEquals(tc, 40, strbuf_len(sb));
	CuAssertStrEquals(tc, "The ", dest);
	// "quick brown fox jumps over the lazy dog."
	strbuf_cut(sb, 5, 25, dest);
	CuAssertSizeTEquals(tc, 15, strbuf_len(sb));
	CuAssertStrEquals(tc, " brown fox jumps over the", dest);
	// "quick lazy dog."
	strbuf_cut(sb, 10, 5, dest);
	CuAssertSizeTEquals(tc, 10, strbuf_len(sb));
	CuAssertStrEquals(tc, " dog.", dest);
	// "quick lazy"
	strbuf_cut(sb, 0, 10, dest);
	CuAssertSizeTEquals(tc, 0, strbuf_len(sb));
	CuAssertStrEquals(tc, "quick lazy", dest);
	// ""
	CuAssertStrEquals(tc, strbuf_as_str(sb), "");
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_paste(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb = strbuf_new_with_capacity(0);
	strbuf_paste(sb, 0, "The quick green", 15);
	CuAssertSizeTEquals(tc, 15, strbuf_len(sb));
	CuAssertStrEquals(tc, "The quick green", strbuf_as_str(sb));
	strbuf_paste(sb, 10, "brown", 5);
	CuAssertSizeTEquals(tc, 15, strbuf_len(sb));
	CuAssertStrEquals(tc, "The quick brown", strbuf_as_str(sb));
	strbuf_paste(sb, 10, "brown fox jumps over the lazy dog.", 34);
	CuAssertSizeTEquals(tc, 44, strbuf_len(sb));
	CuAssertStrEquals(tc, "The quick brown fox jumps over the lazy dog.",
	                  strbuf_as_str(sb));
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_find_n(CuTest *tc)
{
	memdbg_reset();
	char *s = "0123012301230123012301230123012301230123"
			  "0123012301230123012301230123012301230123"
			  "0123012301230123012301230123012301230123";
	char *pat = "123";
	strbuf *sb = strbuf_new_from_str(s, strlen(s));
	size_t matches[7];
	size_t from_pos = 0;
	size_t expected_pos = 1;
	size_t n;
	while ((n = strbuf_find_n(sb, pat, 7, from_pos, matches)) > 0 ) {
		for (size_t i = 0; i < n; i++) {
			CuAssertSizeTEquals(tc, expected_pos, matches[i]);
			from_pos = matches[i] + 1;
			expected_pos += 4;
		}
	}
	pat = "";
	from_pos = 0;
	expected_pos = 0;
	while ((n = strbuf_find_n(sb, pat, 7, from_pos, matches)) > 0 ) {
		for (size_t i = 0; i < n; i++) {
			CuAssertSizeTEquals(tc, expected_pos, matches[i]);
			from_pos = matches[i] + 1;
			expected_pos += 1;
		}
	}
	strbuf_free(sb);
	if (!memdbg_is_empty()) {
		DEBUG_EXEC(memdbg_print_stats(stdout, true));
	}
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_replace_n(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb = strbuf_new_from_str("macaca", 6);
	strbuf_replace_n(sb, "ca", "na", 2, 0);
	CuAssertStrEquals(tc, "manana", strbuf_as_str(sb));
	strbuf_replace_n(sb, "ma", "ba", 2, 0);
	CuAssertStrEquals(tc, "banana", strbuf_as_str(sb));
	strbuf_replace_n(sb, "ana", "aca", 2, 0);
	CuAssertStrEquals(tc, "bacana", strbuf_as_str(sb));
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_replace(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb = strbuf_new_from_str("The quick brown fox jumps over the lazy dog.",
	                                 44);
	strbuf_replace(sb, "black fox", "yellow arara", 0); // do nothing
	CuAssertStrEquals(tc, "The quick brown fox jumps over the lazy dog.",
	                  strbuf_as_str(sb));
	strbuf_replace(sb, "brown fox", "yellow arara", 0);
	CuAssertStrEquals(tc, "The quick yellow arara jumps over the lazy dog.",
	                  strbuf_as_str(sb));
	strbuf_replace(sb, "The quick", "My", 0);
	CuAssertStrEquals(tc, "My yellow arara jumps over the lazy dog.",
	                  strbuf_as_str(sb));
	strbuf_replace(sb, "ara", "cob", 0);
	CuAssertStrEquals(tc, "My yellow cobra jumps over the lazy dog.",
	                  strbuf_as_str(sb));
	strbuf_replace(sb, "yellow cobra jumps over the lazy ", "", 0);
	CuAssertStrEquals(tc, "My dog.", strbuf_as_str(sb));
	strbuf_replace(sb, "", "-", 0);
	CuAssertStrEquals(tc, "-My dog.", strbuf_as_str(sb));
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_replace_all(CuTest *tc)
{
	memdbg_reset();
	strbuf *sb =
	    strbuf_new_from_str("The__quick__brown__arara__jumps__over__the__lazy__arara.",
	                        56);
	strbuf_replace_all(sb, "__", " ", 0);
	CuAssertStrEquals(tc, "The quick brown arara jumps over the lazy arara.",
	                  strbuf_as_str(sb));
	strbuf_replace_all(sb, "ara", "ara", 0);
	CuAssertStrEquals(tc, "The quick brown arara jumps over the lazy arara.",
	                  strbuf_as_str(sb));
	strbuf_replace_all(sb, "ara", "cob", 0);
	CuAssertStrEquals(tc, "The quick brown cobra jumps over the lazy cobra.",
	                  strbuf_as_str(sb));
	strbuf_replace_all(sb, "quick brown cobra jumps over the lazy ", "", 0);
	CuAssertStrEquals(tc, "The cobra.", strbuf_as_str(sb));
	strbuf_replace_all(sb, "", "-", 0);
	CuAssertStrEquals(tc, "-T-h-e- -c-o-b-r-a-.-", strbuf_as_str(sb));
	DESTROY_FLAT(sb, strbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strbuf_printf(CuTest *tc)
{
	memdbg_reset();
	char buf[1<<12];
	char *s = buf;
	buf[0] = '\0';
	strbuf *sbuf = strbuf_new();
	int ws, wsbuf;

	ws = sprintf(s, "");
	s += ws;
	wsbuf = sbprintf(sbuf, "");
	CuAssertIntEquals(tc, ws, wsbuf);
	CuAssertStrEquals(tc, buf, strbuf_as_str(sbuf));

	ws = sprintf(s, "The");
	s += ws;
	wsbuf = sbprintf(sbuf, "The");
	CuAssertIntEquals(tc, ws, wsbuf);
	CuAssertStrEquals(tc, buf, strbuf_as_str(sbuf));


	ws = sprintf(s, " quick brown fox jumped over the lazy dog.");
	s += ws;
	wsbuf = sbprintf(sbuf, " quick brown fox jumped over the lazy dog.");
	CuAssertIntEquals(tc, ws, wsbuf);
	CuAssertStrEquals(tc, buf, strbuf_as_str(sbuf));

	int i = (1 << 30) + 1;
	double d = 0.2 / 0.7;
	char *string = "The quick brown fox jumped over the lazy dog. "
	               "The quick brown fox jumped over the lazy dog."
	               "The quick brown fox jumped over the lazy dog."
	               "The quick brown fox jumped over the lazy dog.";

	ws = sprintf(s, " int=%d double=%lf string=%s", i, d, string);
	s += ws;
	wsbuf = sbprintf(sbuf, " int=%d double=%lf string=%s", i, d, string);
	CuAssertIntEquals(tc, ws, wsbuf);
	CuAssertStrEquals(tc, buf, strbuf_as_str(sbuf));

	strbuf_free(sbuf);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void strbuf_test_teardown(CuTest *tc)
{
}


CuSuite *strbuf_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, strbuf_test_setup);
	SUITE_ADD_TEST(suite, test_strbuf_new_from_str);
	SUITE_ADD_TEST(suite, test_strbuf_get);
	SUITE_ADD_TEST(suite, test_strbuf_append_char);
	SUITE_ADD_TEST(suite, test_strbuf_append);
	SUITE_ADD_TEST(suite, test_strbuf_insert);
	SUITE_ADD_TEST(suite, test_strbuf_cut);
	SUITE_ADD_TEST(suite, test_strbuf_paste);
	SUITE_ADD_TEST(suite, test_strbuf_find_n);
	SUITE_ADD_TEST(suite, test_strbuf_replace_n);
	SUITE_ADD_TEST(suite, test_strbuf_replace);
	SUITE_ADD_TEST(suite, test_strbuf_replace_all);
	SUITE_ADD_TEST(suite, test_strbuf_printf);
	SUITE_ADD_TEST(suite, strbuf_test_teardown);
	return suite;
}