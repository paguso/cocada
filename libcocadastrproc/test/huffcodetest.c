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
#include "randutil.h"
#include "xstr.h"


void test_huffcode_new(CuTest *tc)
{
	char *letters = "abcde";
	size_t freqs[5] = {15, 7, 6, 6, 5};
	alphabet *ab;
	ab = alphabet_new(5, letters);
	huffcode *hc = huffcode_new(ab, freqs);
	//huffcode_print(hc);
	huffcode_free(hc);
}

static void _random_str(char *dest, alphabet *ab, size_t len)
{
	for (size_t i=0; i<len;
	        dest[i++] = ab_char(ab, rand_range_size_t(0, ab_size(ab))));
}


void test_huffcode_codec(CuTest *tc)
{
	size_t max_len = 1024;
	char *letters = "abcdefghijklmnopqrstuvwxyz";
	for (size_t len=2; len<max_len; len++) {
		alphabet *ab = alphabet_new(MIN(len, strlen(letters)), letters);
		char *str = cstr_new(len);
		_random_str(str, ab, len);
		huffcode *hc = huffcode_new_from_str(ab, str);
		//huffcode_print(hc);
		bitvec *code = huffcode_encode(str, len, hc);
		xstr *xsdec = huffcode_decode(code, hc);
		//bytearr_print(code.rawcode, (size_t)mult_ceil(code.code_len, BYTESIZE), 4, "");
		//printf("original=%s\n",str);
		//printf("decoded =%s\n",dec);
		char *dec = xstr_detach(xsdec);
		CuAssertStrEquals(tc, str, dec);
		FREE(str);
		huffcode_free(hc);
	}
}


CuSuite *huffcode_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_huffcode_new);
	SUITE_ADD_TEST(suite, test_huffcode_codec);
	return suite;
}

