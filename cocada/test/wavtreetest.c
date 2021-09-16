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


static alphabet **alphabets;
static char     **strings;
static size_t    *slens;
static size_t     nwt;
static wavtree  **wts;

static size_t __rank_pos_bf(char *str, size_t pos)
{
	size_t r = 0;
	size_t n = strlen(str);
	if (pos >= n)
		return SIZE_MAX;
	for (size_t i = 0; i < pos; i++)
		if (str[i] == str[pos])
			r++;
	return r;
}

static size_t __rank_bf(char *str, size_t pos, char c)
{
	size_t r = 0;
	size_t n = strlen(str);
	for (size_t i = 0; i < n && i < pos; i++)
		if (str[i] == c)
			r++;
	return r;
}

static size_t __sel_bf(char *str, char c, size_t rank)
{
	size_t r = 0, i;
	size_t slen = strlen(str);
	for (i = 0; i < slen; i++)
		if ((str[i] == c) && ((++r) == rank))
			break;
	return i;
}

size_t __pred_bf(char *str, size_t pos, char c)
{
	size_t slen = strlen(str);
	for (pos = MIN(pos, slen); pos > 0 && str[pos - 1] != c; pos--);
	return pos > 0 ? pos - 1 : slen;
}

size_t __succ_bf(char *str, size_t pos, char c)
{
	size_t slen = strlen(str);
	for (pos = MIN(pos, slen); pos < slen && str[pos + 1] != c; pos++);
	return pos < slen ? pos + 1 : slen;
}

static alphabet *seq_ab(size_t len)
{
	char *ab_letters = cstr_new(len);
	for (size_t i = 0; i < len; i++)
		ab_letters[i] = 'a' + i;
	return alphabet_new(len, ab_letters);
}

static char *random_str(alphabet *ab, size_t len)
{
	char *ret = cstr_new(len);
	for (size_t i = 0; i < len; i++)
		ret[i] = ab_char(ab, rand() % ab_size(ab));
	return ret;
}

void wavtree_test_setup(CuTest *tc)
{
	char *ascii = cstr_new(128);
	for (int c=0; c<128; c++)
		ascii[(size_t)c] = (char)c;
	nwt = 2 * 3 * 3; // shape * ab * len

	alphabets = ARR_NEW(alphabet *, nwt);
	for (int i=0; i<nwt; i++) {
		if (((i/3)%3) == 0)
			alphabets[i] = alphabet_new(1, "a");
		if (((i/3)%3) == 1)
			alphabets[i] = alphabet_new('k'-'a', cstr_substr(ascii, 'a', 'k'));
		if (((i/3)%3) == 2)
			alphabets[i] = alphabet_new('~'-' ', cstr_substr(ascii, ' ', '~'));
	}

	strings = ARR_NEW(char *, nwt);
	slens = ARR_NEW(size_t, nwt);
	size_t max_len_mult = 10;
	for (int i=0; i<nwt; i+=3) {
		strings[i+0] = random_str(alphabets[i+0], 0);
		strings[i+1] = random_str(alphabets[i+1], 1);
		strings[i+2] = random_str(alphabets[i+2], max_len_mult*ab_size(alphabets[i+2]));
		slens[i+0] = 0;
		slens[i+1] = 1;
		slens[i+2] = max_len_mult*ab_size(alphabets[i+2]);
	}

	wtshape shp[2] = {WT_BALANCED, WT_HUFFMAN};
	wts = ARR_NEW(wavtree *, nwt);
	for (int i=0; i<nwt; i++) {
		wts[i] = wavtree_new(alphabets[i], strings[i], slens[i], shp[i/9]);
	}
}

void wavtree_test_teardown(CuTest *tc)
{
	for (size_t i = 0; i < nwt; i++) {
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
	for (size_t k = 0; k < nwt; k++) {
		for (size_t j = 0; j < ab_size(alphabets[k]); j++) {
			char c = ab_char(alphabets[k], j);
			char *str = strings[k];
			wavtree *wt = wts[k];
			for (size_t i = 0; i < slens[k] + 5; i++) {
				size_t rank = wavtree_rank(wt, i, c);
				size_t rankbf = __rank_bf(str, i, c);
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
	for (size_t k = 0; k < nwt; k++)   {
		char *str = strings[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t i = 0; i < slens[k]; i++) {
			size_t rank = wavtree_rank_pos(wt, i);
			//printf("Rank[%zu] = %zu\n",i, rank);
			CuAssertSizeTEquals(tc, __rank_pos_bf(str, i), rank);
		}
	}
	wavtree_test_teardown(tc);
}


void test_wavtree_select(CuTest *tc)
{
	wavtree_test_setup(tc);
	for (size_t k = 0; k < nwt; k++)   {
		char *str = strings[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t i = 0; i < slens[k]; i++) {
			size_t rank = wavtree_rank_pos(wt, i);
			size_t sel = wavtree_select(wt, str[i], rank);
			//printf("Rank[%zu] = %zu\n",i, rank);
			//printf("Sel['%c', rk=%zu] = %zu\n",str[i], rank, sel);
			CuAssertSizeTEquals(tc, i, sel);
		}
		// test nonexistent positions
		for (size_t c = 0; c < ab_size(alphabets[k]); ++c) {
			for (size_t ex = 1; ex <= 5; ex++) {
				size_t sel = wavtree_select(wt, ab_char(alphabets[k], c), slens[k] + ex);
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
	for (size_t k = 0; k < nwt; k++)  {
		char *str = strings[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t l = 0; l < ab_size(alphabets[k]); l++) {
			char c = ab_char(alphabets[k], l);
			for (size_t i = 0; i < slens[k]; i++) {
				size_t pred = wavtree_pred(wt, i, c);
				//printf("Pred[%c, %zu] = %zu\n",c, i, pred);
				CuAssertSizeTEquals(tc, __pred_bf(str, i, c), pred);
			}
			for (size_t i = slens[k]; i < slens[k] + 5; i++) {
				size_t pred = wavtree_pred(wt, i, c);
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
	for (size_t k = 0; k < nwt; k++) {
		char *str = strings[k];
		wavtree *wt = wts[k];
		//printf("-------------- wavelet tree -------------------\n");
		//printf("#=%zu ab_size=%zu str=%s online=%s\n",k, ab_size(ab[k]), strings[k], online?"true":"false");
		//wavtree_print(wt);
		for (size_t i = 0; i < slens[k]; i++) {
			for (size_t l = 0; l < ab_size(alphabets[k]); l++) {
				char c = ab_char(alphabets[k], l);
				size_t succ = wavtree_succ(wt, i, c);
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
	for (size_t k = 0; k < nwt; k++)   {
		char *str = strings[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t i = 0; i < slens[k]; i++) {
			char c = wavtree_char(wt, i);
			//printf("Char[%zu, %zu] = %c\n", k, i, c);
			CuAssertCharEquals(tc, str[i], c);
		}
	}
	wavtree_test_teardown(tc);
}



static xstr **xstrs;

static alphabet *xseq_ab(size_t len)
{
	return int_alphabet_new(len);
}

static xstr *random_xstr(alphabet *ab, size_t len)
{
	xstr *ret = xstr_new_with_capacity(nbytes(ab_size(ab)), len);
	for (size_t i = 0; i < len; i++)
		xstr_push(ret, ab_char(ab, rand() % ab_size(ab)));
	return ret;
}


void xwavtree_test_setup(CuTest *tc)
{
	nwt = 2 * 3 * 3; // shape * ab * len
	alphabets = ARR_NEW(alphabet *, nwt);
	for (int i=0; i<nwt; i++) {
		if (((i/3)%3) == 0)
			alphabets[i] = int_alphabet_new(1);
		if (((i/3)%3) == 1)
			alphabets[i] = int_alphabet_new(10);
		if (((i/3)%3) == 2)
			alphabets[i] = int_alphabet_new(300);
	}

	xstrs = ARR_NEW(xstr *, nwt);
	for (int i=0; i<nwt; i+=3) {
		xstrs[i+0] = random_xstr(alphabets[i+0], 0);
		xstrs[i+1] = random_xstr(alphabets[i+1], 1);
		xstrs[i+2] = random_xstr(alphabets[i+2], 5*ab_size(alphabets[i+2]));
	}

	wtshape shp[2] = {WT_BALANCED, WT_HUFFMAN};
	wts = ARR_NEW(wavtree *, nwt);
	for (int i=0; i<nwt; i++) {
		wts[i] = wavtree_new_from_xstr(alphabets[i], xstrs[i], shp[i/9]);
	}
}


void xwavtree_test_teardown(CuTest *tc)
{
	for (size_t i = 0; i < nwt; i++) {
		wavtree_free(wts[i]);
		alphabet_free(alphabets[i]);
		xstr_free(xstrs[i]);
	}
	FREE(alphabets);
	FREE(xstrs);
	FREE(wts);
}


static size_t xrank_bf(xstr *str, size_t pos, xchar_t c)
{
	size_t r = 0;
	size_t n = xstr_len(str);
	for (size_t i = 0; i < n && i < pos; i++)
		if (xstr_get(str, i) == c)
			r++;
	return r;
}

void test_xwavtree_rank(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (size_t k = 0; k < nwt; k++)  {
		wavtree *wt = wts[k];
		//printf("-------------- wavelet tree -------------------\n");
		//wavtree_print(wt);
		for (size_t j = 0; j < ab_size(alphabets[k]); j++) {
			xchar_t c = ab_char(alphabets[k], j);
			xstr *str = xstrs[k];
			for (size_t i = 0, l=xstr_len(str); i < l + 5; i++) {
				size_t rank = wavtree_rank(wt, i, c);
				size_t rankbf = xrank_bf(str, i, c);
				//printf("Rank[%zu, %zu] = %zu, bf = %zu\n", i, c, rank, rankbf);
				CuAssertSizeTEquals(tc, rankbf, rank);
			}
		}
	}
	xwavtree_test_teardown(tc);
}


static size_t xrank_pos_bf(xstr *str, size_t pos)
{
	size_t r = 0;
	size_t n = xstr_len(str);
	if (pos >= n)
		return SIZE_MAX;
	xchar_t c = xstr_get(str, pos);
	for (size_t i = 0; i < pos; i++)
		if (xstr_get(str, i) == c)
			r++;
	return r;
}


void test_xwavtree_rank_pos(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (size_t k = 0; k < nwt; k++)  {
		xstr *str = xstrs[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t i = 0, l=xstr_len(str); i < l; i++) {
			size_t rank = wavtree_rank_pos(wt, i);
			//printf("Rank[%zu] = %zu\n",i, rank);
			CuAssertSizeTEquals(tc, xrank_pos_bf(str, i), rank);
		}
	}
	xwavtree_test_teardown(tc);
}


void test_xwavtree_select(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (size_t k = 0; k < nwt; k++) {
		xstr *str = xstrs[k];
		size_t sl = xstr_len(str);
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t i = 0; i < sl; i++) {
			size_t rank = wavtree_rank_pos(wt, i);
			size_t sel = wavtree_select(wt, xstr_get(str, i), rank);
			//printf("Rank[%zu] = %zu\n",i, rank);
			//printf("Sel['"XCHAR_FMT"'], rk=%zu] = %zu\n", xstr_get(str,i), rank, sel);
			CuAssertSizeTEquals(tc, i, sel);
		}
		// test nonexistent positions
		for (size_t c = 0; c < ab_size(alphabets[k]); ++c) {
			for (size_t ex = 1; ex <= 5; ex++) {
				size_t sel = wavtree_select(wt, ab_char(alphabets[k], c), sl + ex);
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
	for (size_t k = 0; k < nwt; k++)  {
		xstr *str = xstrs[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t i = 0, l=xstr_len(str); i < l; i++) {
			xchar_t c = wavtree_char(wt, i);
			//printf("Char[%zu, %zu] = %c\n", k, i, c);
			CuAssertIntEquals(tc, (int)xstr_get(str, i), (int)c);
		}
	}
	xwavtree_test_teardown(tc);
}


size_t __xpred_bf(xstr *str, size_t pos, xchar_t c)
{
	size_t slen = xstr_len(str);
	for (pos = MIN(pos, slen); pos > 0 && xstr_get(str, pos-1)!=c; pos--);
	return pos > 0 ? pos - 1 : slen;
}

size_t __xsucc_bf(xstr *str, size_t pos, xchar_t c)
{
	size_t slen = xstr_len(str);
	for (pos = MIN(pos, slen); pos<slen && xstr_get(str, pos+1)!=c; pos++);
	return pos < slen ? pos + 1 : slen;
}


void test_xwavtree_pred(CuTest *tc)
{
	xwavtree_test_setup(tc);
	for (size_t k = 0; k < nwt; k++)  {
		xstr *str = xstrs[k];
		wavtree *wt = wts[k];
		//printf("wavelet tree\n");
		//wavtree_print(wt);
		for (size_t l = 0; l < ab_size(alphabets[k]); l++) {
			xchar_t c = ab_char(alphabets[k], l);
			for (size_t i = 0, l=xstr_len(str); i < l; i++) {
				size_t pred = wavtree_pred(wt, i, c);
				//printf("Pred[%c, %zu] = %zu\n",c, i, pred);
				CuAssertSizeTEquals(tc, __xpred_bf(str, i, c), pred);
			}
			for (size_t i=xstr_len(str), l=xstr_len(str); i < l + 5; i++) {
				size_t pred = wavtree_pred(wt, i, c);
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
	for (size_t k = 0; k < nwt; k++) {
		xstr *str = xstrs[k];
		wavtree *wt = wts[k];
		//printf("-------------- wavelet tree -------------------\n");
		//printf("#=%zu ab_size=%zu str=%s online=%s\n",k, ab_size(ab[k]), strings[k], online?"true":"false");
		//wavtree_print(wt);
		for (size_t i = 0, sl=xstr_len(str); i < sl;  i++) {
			for (size_t l = 0; l < ab_size(alphabets[k]); l++) {
				xchar_t c = ab_char(alphabets[k], l);
				size_t succ = wavtree_succ(wt, i, c);
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
