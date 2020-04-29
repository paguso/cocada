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
#include "bitvec.h"
#include "bytearr.h"
#include "cstrutil.h"
#include "huffcode.h"
#include "mathutil.h"
#include "xstring.h"


void test_huffcode_new(CuTest *tc)
{
	char *letters = "abcde";
	size_t freqs[5] = {15, 7, 6, 6, 5};
	alphabet *ab;
	ab = alphabet_new(5, letters);
	huffcode *hc;
	hc = huffcode_new(ab, freqs);
	//huffcode_print(hc);
}

static void _random_str(char *dest, alphabet *ab, size_t len)
{
	for (size_t i=0; i<len; dest[i++] = ab_char(ab, rand()%ab_size(ab)));

}

void test_huffcode_codec(CuTest *tc)
{
	size_t max_len = 1024;
	char *letters = "abcdefghijklmnopqrstuvwxyz";
	for (size_t len=2; len<max_len; len++) {
		alphabet *ab = alphabet_new(MIN(len, strlen(letters)), letters);
		char *str = cstr_new(len);
		_random_str(str, ab, len);
		strstream *sst = strstream_open_str(str, len);
		huffcode *hc = huffcode_new_from_stream(ab, sst);
		strstream_reset(sst);
		//huffcode_print(hc);
		bitvec *code = huffcode_encode(hc, sst);
		xstring *xsdec = huffcode_decode(hc, code);
		//bytearr_print(code.rawcode, (size_t)mult_ceil(code.code_len, BYTESIZE), 4, "");
		//printf("original=%s\n",str);
		//printf("decoded =%s\n",dec);
		char *dec = xstr_detach(xsdec);
		CuAssertStrEquals(tc, str, dec);
		strstream_close(sst);
		FREE(str);
		huffcode_free(hc);
	}
}



static xstring  *_random_xstr(alphabet *ab, size_t len)
{
	xstring *xs = xstring_new_with_capacity(nbytes(ab_size(ab)), len);
	for (size_t i=0; i<len; i++)
		xstr_push(xs, ab_char(ab, rand()%ab_size(ab)));
	return xs;
}



void test_huffcode_xcodec(CuTest *tc)
{
	size_t max_len = 4096;
	for (size_t len=2; len<max_len; len++) {
		alphabet *ab = int_alphabet_new(MAX(2, len/4));
		xstring *xstr = _random_xstr(ab, len);
		strstream *sst = strstream_open_xstr(xstr);
		huffcode *hc = huffcode_new_from_stream(ab, sst);
		strstream_reset(sst);
		//huffcode_print(hc);
		bitvec *code = huffcode_encode(hc, sst);
		xstring *xsdec = huffcode_decode(hc, code);
		int cmp = xstr_cmp(xstr, xsdec);
		if (cmp) {
			xstr_print(xstr);
			xstr_print(xsdec);
		}
		CuAssert(tc, "decoded xstring does not match the encoded one",
		         cmp==0 );
		strstream_close(sst);
		//FREE(str);
		huffcode_free(hc);
	}
}


CuSuite *huffcode_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_huffcode_new);
	SUITE_ADD_TEST(suite, test_huffcode_codec);
	SUITE_ADD_TEST(suite, test_huffcode_xcodec);
	return suite;
}

