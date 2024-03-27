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

#include "arrays.h"
#include "alphabet.h"
#include "bitbyte.h"
#include "cstrutil.h"
#include "errlog.h"
#include "sais.h"
#include "memdbg.h"
#include "randutil.h"
#include "xstr.h"
#include "xstrformat.h"

void random_str(alphabet *ab, size_t len, char *dest)
{
	for (size_t i = 0; i < len; i++) {
		dest[i] = (char)ab_char(ab, rand_range_size_t(0, ab_size(ab)));
	}
	dest[len] = '\0';
}

void random_xstr(alphabet *ab, size_t len, xstr *dest)
{
	xstr_clear(dest);
	for (size_t i = 0; i < len; i++) {
		xstr_push(dest, ab_char(ab, rand_range_size_t(0, ab_size(ab))));
	}
}


void sais_test_str(CuTest *tc)
{
	memdbg_reset();
	alphabet *ab = alphabet_new(10, "abcdefghij");
	size_t max_len = 1000;
	char *str = cstr_new(max_len);
	for (size_t len = 0; len < max_len; len++) {
		random_str(ab, len, str);
		size_t *sarr = sais(str, len, ab);
		//DEBUG_EXEC(ARR_PRINT(sarr, SA, %zu, 0, len + 1, 10, "   "));
		CuAssertSizeTEquals(tc, len, sarr[0]);
		for (size_t i = 1; i + 1 < len; i++) {
			CuAssert(tc, "Wrong SARR order", strcmp(&str[sarr[i]], &str[sarr[i + 1]]) < 0);
		}
		FREE(sarr);
	}
	FREE(str);
	alphabet_free(ab);
	CuAssert(tc, "Memory leak", memdbg_is_empty());
}


int xstr_suff_cmp(xstr *str, size_t i, size_t j)
{
	if (i == j) {
		return 0;
	}
	size_t len = xstr_len(str);
	while (true) {
		if (i == len) {
			return -1;
		}
		else if ( j == len ) {
			return +1;
		}
		else if (xstr_get(str, i) < xstr_get(str, j)) {
			return -1;
		}
		else if (xstr_get(str, i) > xstr_get(str, j)) {
			return +1;
		}
		i++;
		j++;
	}
}

void sais_test_xstr(CuTest *tc)
{
	memdbg_reset();
	alphabet *ab = int_alphabet_new(300);
	size_t max_len = 5000;
	xstr *str = xstr_new(nbytes(ab_size(ab)));
	xstrformat *xf = xstrformat_new(str);
	for (size_t len = 0; len < max_len; len++) {
		random_xstr(ab, len, str);
		CuAssertSizeTEquals(tc, len, xstr_len(str));
		//format_print(xstrformat_as_format(xf));
		size_t *sarr = sais_xstr(str, ab);
		CuAssertSizeTEquals(tc, len, sarr[0]);
		for (size_t i = 1; i + 1 < len; i++) {
			CuAssert(tc, "Wrong SARR order", xstr_suff_cmp(str, sarr[i], sarr[i + 1]) < 0);
		}
		FREE(sarr);
	}
	xstrformat_free(xf);
	xstr_free(str);
	alphabet_free(ab);
	if (!memdbg_is_empty()) {
		DEBUG_EXEC(memdbg_print_stats(stdout, true));
	}
	CuAssert(tc, "Memory leak", memdbg_is_empty());

}


CuSuite *sais_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, sais_test_str);
	SUITE_ADD_TEST(suite, sais_test_xstr);
	return suite;
}
