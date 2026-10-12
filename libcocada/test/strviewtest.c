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
#include "strview.h"


void test_strview_find_n(CuTest *tc)
{
	memdbg_reset();
	char *s = "0123012301230123012301230123012301230123"
	          "0123012301230123012301230123012301230123"
	          "0123012301230123012301230123012301230123";
	char *pat = "123";
	strview sv = strview_build(s);
	size_t matches[7];
	size_t expected_pos = 1;
	size_t n = strview_find_n(&sv, pat, 7, matches);
	CuAssertSizeTEquals(tc, 7, n);
	for (size_t i = 0; i < n; i++) {
		CuAssertSizeTEquals(tc, expected_pos, matches[i]);
		expected_pos += 4;
	}
	pat = "";
	expected_pos = 0;
	n = strview_find_n(&sv, pat, 7,  matches);
	CuAssertSizeTEquals(tc, 7, n);
	for (size_t i = 0; i < n; i++) {
		CuAssertSizeTEquals(tc, expected_pos, matches[i]);
		expected_pos += 1;
	}
	pat = "45678";
	n = strview_find_n(&sv, pat, 7,  matches);
	CuAssertSizeTEquals(tc, 0, n);
	pat = s;
	n = strview_find_n(&sv, pat, 7,  matches);
	CuAssertSizeTEquals(tc, 1, n);
	CuAssertSizeTEquals(tc, 0, matches[0]);
	if (!memdbg_is_empty()) {
		DEBUG_EXEC(memdbg_print_stats(stdout, true));
	}
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


void test_strview_split(CuTest *tc)
{
	memdbg_reset();
	char *str = "The quick brown fox jumps over the lazy dog";
	char *tokens[] =  {"The","quick","brown","fox","jumps","over","the","lazy","dog"};
	char *sep = " ";
	size_t sep_len = strlen(sep);
	size_t len = strlen(str);
	strview sview = strview_build(str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	strview_iter *iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens[i]));
	}
	strview_iter_free(iter);

	str = "__X__X__";
	char *tokens2[] = {"","X","X",""};
	len = strlen(str);
	sep = "__";
	sep_len = strlen(sep);
	strview_init(&sview, str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens2[i]));
	}
	strview_iter_free(iter);

	str = "_X_X";
	char *tokens3[] = {"","X","X"};
	len = strlen(str);
	sep = "_";
	sep_len = strlen(sep);
	strview_init(&sview, str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens3[i]));
	}
	strview_iter_free(iter);

	str = "X_X_";
	char *tokens4[] = {"X","X",""};
	len = strlen(str);
	sep = "_";
	sep_len = strlen(sep);
	strview_init(&sview, str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens4[i]));
	}
	strview_iter_free(iter);

	str = "__";
	char *tokens5[] = {"","",""};
	len = strlen(str);
	sep = "_";
	sep_len = strlen(sep);
	strview_init(&sview, str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens5[i]));
	}
	strview_iter_free(iter);

	str = "__";
	char *tokens6[] = {"",""};
	len = strlen(str);
	sep = "__";
	sep_len = strlen(sep);
	strview_init(&sview, str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens6[i]));
	}
	strview_iter_free(iter);

	str = "";
	char *tokens7[] = {""};
	len = strlen(str);
	sep = "__";
	sep_len = strlen(sep);
	strview_init(&sview, str);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_split(&sview, sep, sep_len);
	for (int i = 0; strview_iter_has_next(iter); i++) {
		strview tok = strview_iter_next(iter);
		printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
		CuAssertTrue(tc, strview_eq_cstr(&tok, tokens7[i]));
	}
	strview_iter_free(iter);

	CuAssert(tc, "Memory Leak.", memdbg_is_empty());
}


CuSuite *strview_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("strview");
	SUITE_ADD_TEST(suite, test_strview_find_n);
	SUITE_ADD_TEST(suite, test_strview_split);
	return suite;
}
