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
#include <string.h>

#include "CuTest.h"

#include "alphabet.h"
#include "arrays.h"
#include "bytearr.h"
#include "cstrutil.h"
#include "huffcode.h"
#include "mathutil.h"
#include "strstream.h"
#include "wavtree.h"


static Alphabet **alphabets;
static char     **strings;
static usize    *slens;
static usize     nwt;
static WavTree  **wts;

static usize __rank_pos_bf(char *str, usize pos)
{
	usize r = 0;
	usize n = strlen(str);
	if (pos >= n)
		return SIZE_MAX;
	for (usize i = 0; i < pos; i++)
		if (str[i] == str[pos])
			r++;
	return r;
}

static usize __rank_bf(char *str, usize pos, char c)
{
	usize r = 0;
	usize n = strlen(str);
	for (usize i = 0; i < n && i < pos; i++)
		if (str[i] == c)
			r++;
	return r;
}

static usize __sel_bf(char *str, char c, usize rank)
{
	usize r = 0, i;
	usize slen = strlen(str);
	for (i = 0; i < slen; i++)
		if ((str[i] == c) && ((++r) == rank))
			break;
	return i;
}

usize __pred_bf(char *str, usize pos, char c)
{
	usize slen = strlen(str);
	for (pos = MIN(pos, slen); pos > 0 && str[pos - 1] != c; pos--);
	return pos > 0 ? pos - 1 : slen;
}

usize __succ_bf(char *str, usize pos, char c)
{
	usize slen = strlen(str);
	for (pos = MIN(pos, slen); pos < slen && str[pos + 1] != c; pos++);
	return pos < slen ? pos + 1 : slen;
}

static Alphabet *seq_ab(usize len)
{
	char *ab_letters = cstr_new(len);
	for (usize i = 0; i < len; i++)
		ab_letters[i] = 'a' + i;
	return alphabet_new(len, ab_letters);
}

static char *random_str(Alphabet *ab, usize len)
{
	char *ret = cstr_new(len);
	for (usize i = 0; i < len; i++)
		ret[i] = alphabet_char(ab, rand() % alphabet_size(ab));
	return ret;
}

void wavtree_test_setup(CuTest *tc)
{
	char *ascii = cstr_new(128);
	for (int c = 0; c < 128; c++)
		ascii[(usize)c] = (char)c;
	nwt = 2 * 3 * 3; // shape * ab * len

	alphabets = ARR_NEW(Alphabet *, nwt);
	for (int i = 0; i < nwt; i++) {
		if (((i / 3) % 3) == 0)
			alphabets[i] = alphabet_new(1, "a");
		if (((i / 3) % 3) == 1)
			alphabets[i] = alphabet_new('k' - 'a', cstr_substr(ascii, 'a', 'k'));
		if (((i / 3) % 3) == 2)
			alphabets[i] = alphabet_new('~' - ' ', cstr_substr(ascii, ' ', '~'));
	}

	strings = ARR_NEW(char *, nwt);
	slens = ARR_NEW(usize, nwt);
	usize max_len_mult = 10;
	for (int i = 0; i < nwt; i += 3) {
		strings[i + 0] = random_str(alphabets[i + 0], 0);
		strings[i + 1] = random_str(alphabets[i + 1], 1);
		strings[i + 2] = random_str(alphabets[i + 2],
		                            max_len_mult * alphabet_size(alphabets[i + 2]));
		slens[i + 0] = 0;
		slens[i + 1] = 1;
		slens[i + 2] = max_len_mult * alphabet_size(alphabets[i + 2]);
	}

	WavTreeShape shp[2] = {WT_BALANCED, WT_HUFFMAN};
	wts = ARR_NEW(WavTree *, nwt);
	for (int i = 0; i < nwt; i++) {
		wts[i] = wavtree_new(alphabets[i], strings[i], slens[i], shp[i / 9]);
	}
}

void wavtree_test_teardown(CuTest *tc)
{
	for (usize i = 0; i < nwt; i++) {
		wavtree_free(wts[i]);
		alphabet_free(alphabets[i]);
		free(strings[i]);
	}
	FREE(wts);
	FREE(strings);
	FREE(slens);
	FREE(alphabets);
}


void test_wavtree_rank(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++) {
		for (usize j = 0; j < alphabet_size(alphabets[k]); j++) {
			char c = alphabet_char(alphabets[k], j);
			char *str = strings[k];
			WavTree *wt = wts[k];
			for (usize i = 0; i < slens[k] + 5; i++) {
				usize rank = wavtree_rank(wt, i, c);
				usize rankbf = __rank_bf(str, i, c);
				//printf("Rank(WT[%zu] %zu, %c) = %zu, bf = %zu\n",k, i, c, rank, rankbf);
				CuAssertSizeTEquals(tc, rankbf, rank);
			}
		}
	}
	wavtree_test_teardown(tc);
}


void test_wavtree_rank_pos(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)   {
		char *str = strings[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize i = 0; i < slens[k]; i++) {
			usize rank = wavtree_rank_pos(wt, i);
			//printf("Rank[%zu] = %zu\n",i, rank);
			CuAssertSizeTEquals(tc, __rank_pos_bf(str, i), rank);
		}
	}
	wavtree_test_teardown(tc);
}


void test_wavtree_select(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)   {
		char *str = strings[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize i = 0; i < slens[k]; i++) {
			usize rank = wavtree_rank_pos(wt, i);
			usize sel = wavtree_select(wt, str[i], rank);
			//printf("Rank[%zu] = %zu\n",i, rank);
			//printf("Sel['%c', rk=%zu] = %zu\n",str[i], rank, sel);
			CuAssertSizeTEquals(tc, i, sel);
		}
		// test nonexistent positions
		for (usize c = 0; c < alphabet_size(alphabets[k]); ++c) {
			for (usize ex = 1; ex <= 5; ex++) {
				usize sel = wavtree_select(wt, alphabet_char(alphabets[k], c), slens[k] + ex);
				//printf("Nonex Sel['%c', rk=%zu] = %zu\n",ab_char(alphabets[k], c), slens[k]+ex, sel);
				CuAssertSizeTEquals(tc, slens[k], sel);
			}
		}
	}
	wavtree_test_teardown(tc);
}


void test_wavtree_pred(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)  {
		char *str = strings[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize l = 0; l < alphabet_size(alphabets[k]); l++) {
			char c = alphabet_char(alphabets[k], l);
			for (usize i = 0; i < slens[k]; i++) {
				usize pred = wavtree_pred(wt, i, c);
				//printf("Pred[%c, %zu] = %zu\n",c, i, pred);
				CuAssertSizeTEquals(tc, __pred_bf(str, i, c), pred);
			}
			for (usize i = slens[k]; i < slens[k] + 5; i++) {
				usize pred = wavtree_pred(wt, i, c);
				//printf("Nonex Pred[%c, %zu] = %zu\n",c, i, pred);
				CuAssertSizeTEquals(tc, __pred_bf(str, i, c), pred);
			}
		}
	}
	wavtree_test_teardown(tc);
}


void test_wavtree_succ(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++) {
		char *str = strings[k];
		WavTree *wt = wts[k];
		//printf("-------------- wavelet tree -------------------\n");
		//printf("#=%zu ab_size=%zu str=%s online=%s\n",k, ab_size(ab[k]), strings[k], online?"true":"false");
		//wavtree_print(wt);
		for (usize i = 0; i < slens[k]; i++) {
			for (usize l = 0; l < alphabet_size(alphabets[k]); l++) {
				char c = alphabet_char(alphabets[k], l);
				usize succ = wavtree_succ(wt, i, c);
				//printf("Succ(WT[%zu], %c, %zu) = %zu\n",k, c, i, succ);
				CuAssertSizeTEquals(tc, __succ_bf(str, i, c), succ);
			}
		}
	}
	wavtree_test_teardown(tc);
}


void test_wavtree_char(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)   {
		char *str = strings[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize i = 0; i < slens[k]; i++) {
			char c = wavtree_char(wt, i);
			//printf("Char[%zu, %zu] = %c\n", k, i, c);
			CuAssertCharEquals(tc, str[i], c);
		}
	}
	wavtree_test_teardown(tc);
}



static xstr **xstrs;

static Alphabet *xseq_ab(usize len)
{
	return alphabet_new_int_ab(len);
}

static xstr *random_xstr(Alphabet *ab, usize len)
{
	xstr *ret = xstr_new_with_capacity(nbytes(alphabet_size(ab)), len);
	for (usize i = 0; i < len; i++)
		xstr_push(ret, alphabet_char(ab, rand() % alphabet_size(ab)));
	return ret;
}


void xwavtree_test_setup(CuTest *tc)
{
	nwt = 2 * 3 * 3; // shape * ab * len
	alphabets = ARR_NEW(Alphabet *, nwt);
	for (int i = 0; i < nwt; i++) {
		if (((i / 3) % 3) == 0)
			alphabets[i] = alphabet_new_int_ab(1);
		if (((i / 3) % 3) == 1)
			alphabets[i] = alphabet_new_int_ab(10);
		if (((i / 3) % 3) == 2)
			alphabets[i] = alphabet_new_int_ab(300);
	}

	xstrs = ARR_NEW(xstr *, nwt);
	for (int i = 0; i < nwt; i += 3) {
		xstrs[i + 0] = random_xstr(alphabets[i + 0], 0);
		xstrs[i + 1] = random_xstr(alphabets[i + 1], 1);
		xstrs[i + 2] = random_xstr(alphabets[i + 2],
		                           5 * alphabet_size(alphabets[i + 2]));
	}

	WavTreeShape shp[2] = {WT_BALANCED, WT_HUFFMAN};
	wts = ARR_NEW(WavTree *, nwt);
	for (int i = 0; i < nwt; i++) {
		wts[i] = wavtree_new_from_xstr(alphabets[i], xstrs[i], shp[i / 9]);
	}
}


void xwavtree_test_teardown(CuTest *tc)
{
	for (usize i = 0; i < nwt; i++) {
		wavtree_free(wts[i]);
		alphabet_free(alphabets[i]);
		xstr_free(xstrs[i]);
	}
	FREE(alphabets);
	FREE(xstrs);
	FREE(wts);
}


static usize xrank_bf(xstr *str, usize pos, xchar c)
{
	usize r = 0;
	usize n = xstr_len(str);
	for (usize i = 0; i < n && i < pos; i++)
		if (xstr_get(str, i) == c)
			r++;
	return r;
}

void test_xwavtree_rank(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)  {
		WavTree *wt = wts[k];
		//printf("-------------- wavelet tree -------------------\n");
		//wavtree_print(wt);
		for (usize j = 0; j < alphabet_size(alphabets[k]); j++) {
			xchar c = alphabet_char(alphabets[k], j);
			xstr *str = xstrs[k];
			for (usize i = 0, l = xstr_len(str); i < l + 5; i++) {
				usize rank = wavtree_rank(wt, i, c);
				usize rankbf = xrank_bf(str, i, c);
				//printf("Rank[%zu, %zu] = %zu, bf = %zu\n", i, c, rank, rankbf);
				CuAssertSizeTEquals(tc, rankbf, rank);
			}
		}
	}
	xwavtree_test_teardown(tc);
}


static usize xrank_pos_bf(xstr *str, usize pos)
{
	usize r = 0;
	usize n = xstr_len(str);
	if (pos >= n)
		return SIZE_MAX;
	xchar c = xstr_get(str, pos);
	for (usize i = 0; i < pos; i++)
		if (xstr_get(str, i) == c)
			r++;
	return r;
}


void test_xwavtree_rank_pos(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)  {
		xstr *str = xstrs[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize i = 0, l = xstr_len(str); i < l; i++) {
			usize rank = wavtree_rank_pos(wt, i);
			//printf("Rank[%zu] = %zu\n",i, rank);
			CuAssertSizeTEquals(tc, xrank_pos_bf(str, i), rank);
		}
	}
	xwavtree_test_teardown(tc);
}


void test_xwavtree_select(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++) {
		xstr *str = xstrs[k];
		usize sl = xstr_len(str);
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize i = 0; i < sl; i++) {
			usize rank = wavtree_rank_pos(wt, i);
			usize sel = wavtree_select(wt, xstr_get(str, i), rank);
			//printf("Rank[%zu] = %zu\n",i, rank);
			//printf("Sel['"XCHAR_FMT"'], rk=%zu] = %zu\n", xstr_get(str,i), rank, sel);
			CuAssertSizeTEquals(tc, i, sel);
		}
		// test nonexistent positions
		for (usize c = 0; c < alphabet_size(alphabets[k]); ++c) {
			for (usize ex = 1; ex <= 5; ex++) {
				usize sel = wavtree_select(wt, alphabet_char(alphabets[k], c), sl + ex);
				//printf("Nonex Sel['"XCHAR_FMT"', rk=%zu] = %zu\n",ab_char(alphabets[k], c), l+ex, sel);
				CuAssertSizeTEquals(tc, sl, sel);
			}
		}
	}
	xwavtree_test_teardown(tc);
}


void test_xwavtree_char(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)  {
		xstr *str = xstrs[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize i = 0, l = xstr_len(str); i < l; i++) {
			xchar c = wavtree_char(wt, i);
			//printf("Char[%zu, %zu] = %c\n", k, i, c);
			CuAssertIntEquals(tc, (int)xstr_get(str, i), (int)c);
		}
	}
	xwavtree_test_teardown(tc);
}


usize __xpred_bf(xstr *str, usize pos, xchar c)
{
	usize slen = xstr_len(str);
	for (pos = MIN(pos, slen); pos > 0 && xstr_get(str, pos - 1) != c; pos--);
	return pos > 0 ? pos - 1 : slen;
}

usize __xsucc_bf(xstr *str, usize pos, xchar c)
{
	usize slen = xstr_len(str);
	for (pos = MIN(pos, slen); pos < slen && xstr_get(str, pos + 1) != c; pos++);
	return pos < slen ? pos + 1 : slen;
}


void test_xwavtree_pred(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++)  {
		xstr *str = xstrs[k];
		WavTree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (usize l = 0; l < alphabet_size(alphabets[k]); l++) {
			xchar c = alphabet_char(alphabets[k], l);
			for (usize i = 0, l = xstr_len(str); i < l; i++) {
				usize pred = wavtree_pred(wt, i, c);
				//printf("Pred[%c, %zu] = %zu\n",c, i, pred);
				CuAssertSizeTEquals(tc, __xpred_bf(str, i, c), pred);
			}
			for (usize i = xstr_len(str), l = xstr_len(str); i < l + 5; i++) {
				usize pred = wavtree_pred(wt, i, c);
				//printf("Nonex Pred[%c, %zu] = %zu\n",c, i, pred);
				CuAssertSizeTEquals(tc, __xpred_bf(str, i, c), pred);
			}
		}
	}
	xwavtree_test_teardown(tc);
}


void test_xwavtree_succ(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (usize k = 0; k < nwt; k++) {
		xstr *str = xstrs[k];
		WavTree *wt = wts[k];
		//printf("-------------- wavelet tree -------------------\n");
		//printf("#=%zu ab_size=%zu str=%s online=%s\n",k, ab_size(ab[k]), strings[k], online?"true":"false");
		//wavtree_print(wt);
		for (usize i = 0, sl = xstr_len(str); i < sl;  i++) {
			for (usize l = 0; l < alphabet_size(alphabets[k]); l++) {
				xchar c = alphabet_char(alphabets[k], l);
				usize succ = wavtree_succ(wt, i, c);
				//xstr_print(str);
				//printf("Succ(WT[%zu], "XCHAR_FMT" (%c), %zu) = %zu\n",k, c, (char)c, i, succ);
				CuAssertSizeTEquals(tc, __xsucc_bf(str, i, c), succ);
			}
		}
	}
	xwavtree_test_teardown(tc);
}



CuSuite *wavtree_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_wavtree_rank);
	SUITE_ADD_TEST(suite, test_wavtree_rank_pos);
	SUITE_ADD_TEST(suite, test_wavtree_select);
	SUITE_ADD_TEST(suite, test_wavtree_pred);
	SUITE_ADD_TEST(suite, test_wavtree_succ);
	SUITE_ADD_TEST(suite, test_wavtree_char);
	SUITE_ADD_TEST(suite, test_xwavtree_rank);
	SUITE_ADD_TEST(suite, test_xwavtree_rank_pos);
	SUITE_ADD_TEST(suite, test_xwavtree_select);
	SUITE_ADD_TEST(suite, test_xwavtree_char);
	SUITE_ADD_TEST(suite, test_xwavtree_succ);
	SUITE_ADD_TEST(suite, test_xwavtree_pred);

	return suite;
}
