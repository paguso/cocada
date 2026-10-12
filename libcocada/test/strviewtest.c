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


void test_strview_tok(CuTest *tc)
{
	memdbg_reset();
	char *str = "The quick brown fox jumps over the lazy dog";
	char *tokens[] =  {"The","quick","brown","fox","jumps","over","the","lazy","dog"};
	char *sep = " ";
	size_t sep_len = strlen(sep);
	size_t len = strlen(str);
	strview sview;
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	strview_iter *iter = strview_tok(&sview, sep, sep_len);
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
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_tok(&sview, sep, sep_len);
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
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_tok(&sview, sep, sep_len);
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
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_tok(&sview, sep, sep_len);
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
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_tok(&sview, sep, sep_len);
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
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_tok(&sview, sep, sep_len);
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
	strview_init(&sview, str, len);
	CuAssertSizeTEquals(tc, len, strview_len(&sview));
	iter = strview_tok(&sview, sep, sep_len);
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
	SUITE_ADD_TEST(suite, test_strview_tok);
	return suite;
}
