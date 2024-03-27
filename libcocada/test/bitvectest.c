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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "arrays.h"
#include "bitbyte.h"
#include "bitvec.h"
#include "bitarr.h"
#include "new.h"
#include "mathutil.h"
#include "memdbg.h"

static size_t ba_size = 1043;


void bitvec_test_new_with_len(CuTest *tc)
{
	memdbg_reset();
	for (size_t len = 0; len < ba_size; len++) {
		bitvec *bv = bitvec_new_with_len(len);
		CuAssertSizeTEquals(tc, len, bitvec_len(bv));
		for (int i = 0; i < len ; i++) {
			CuAssert(tc, "Wrong bit", 0 == bitvec_get_bit(bv, i));
		}
		bitvec_free(bv);
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void bitvec_test_get_set(CuTest *tc)
{
	memdbg_reset();
	for (size_t len = 0; len < ba_size; len++) {
		bitvec *bv = bitvec_new_with_len(len);
		CuAssertSizeTEquals(tc, len, bitvec_len(bv));
		for (int i = 0; i < len ; i++) {
			bool bit = rand() % 2;
			bitvec_set_bit(bv, i, bit);
			CuAssert(tc, "Wrong bit", bit == bitvec_get_bit(bv, i));
		}
		bitvec_free(bv);
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void bitvec_test_push(CuTest *tc)
{
	memdbg_reset();
	bitvec *bv = bitvec_new_with_capacity(ba_size);
	byte_t *array;
	array = ARR_NEW(byte_t, ba_size);
	bool bit;
	for (int i = 0; i < ba_size; i++) {
		bit = ((byte_t)rand() % 2);
		bitvec_push(bv, bit);
		array[i] = bit;
	}
	for (int i = 0; i < ba_size; i++) {
		bit = bitvec_get_bit(bv, i);
		CuAssertIntEquals(tc, array[i], bit);
	}
	bitvec_free(bv);
	FREE(array);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}

void bitvec_test_push_n(CuTest *tc)
{
	memdbg_reset();
	bitvec *bv = bitvec_new_with_capacity(0);
	byte_t *array;
	array = ARR_NEW(byte_t, ba_size);
	bool bit = false;
	size_t n = 0, s = 0;
	while (s < ba_size) {
		n = MAX(1, ((size_t)rand()) % (ba_size - s));
		bit = 1 - bit;
		//printf("current size=%zu adding %zu %c-bits\n",s, n, bit?'1':'0');
		bitvec_push_n(bv, n, bit);
		ARR_FILL(array, s, s + n, bit);
		s += n;
		//bitvec_print(bv, 8);
	}
	//bitvec_print(bv, 8);
	for (int i = 0; i < ba_size; i++) {
		bit = bitvec_get_bit(bv, i);
		CuAssertIntEquals(tc, array[i], bit);
	}
	bitvec_free(bv);
	FREE(array);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void bitvec_test_count(CuTest *tc)
{
	memdbg_reset();
	bitvec *bv = bitvec_new_with_capacity(0);
	bool bit;
	byte_t *ba = bitarr_new(ba_size);
	size_t count1 = 0;
	for (size_t i = 0; i < ba_size; i++) {
		bit = ((byte_t)rand() % 2);
		bitarr_set_bit(ba, i, bit);
		bitvec_push(bv, bit);
		count1 += bit;
	}
	if (count1 != bitvec_count(bv, 1))
		CuAssertSizeTEquals(tc, count1, bitvec_count(bv, 1));
	CuAssertSizeTEquals(tc, ba_size - count1, bitvec_count(bv, 0));
	for (size_t i = 0; i < ba_size; i++) {
		for (size_t j = i; j < ba_size; j++) {
			count1 = 0;
			for (size_t k = i; k < j; k++) {
				count1 += bitarr_get_bit(ba, k);
			}
			CuAssertSizeTEquals(tc, count1, bitvec_count_range(bv, 1, i, j));
			CuAssertSizeTEquals(tc, (j - i) - count1, bitvec_count_range(bv, 0, i, j));
		}
	}
	FREE(ba);
	bitvec_free(bv);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void bitvec_test_select(CuTest *tc)
{
	byte_t bit_patterns[6] = {0x00, 0xFF, 0x0F, 0xF0, 0x55, 0xAA};
	memdbg_reset();
	for (int intbit = 0; intbit < 2; intbit++) {
		bool bit = (bool)intbit;
		for (size_t len = 0; len < ba_size; len++) {
			for (int i = 0; i < 6; i++) {
				byte_t *ba = bitarr_new(len);
				memset(ba, bit_patterns[i], DIVCEIL(len, BYTESIZE));
				bitvec *bv = bitvec_new_from_bitarr(ba, len);
				size_t bitcount = bitvec_count(bv, bit);
				size_t rank = 0;
				size_t pos = 0;
				for (size_t r = 0; r < bitcount; r++) {
					while (pos < len && rank < r) {
						rank += (bitvec_get_bit(bv, pos++) == bit);
					}
					while (pos < len && bitvec_get_bit(bv, pos) != bit) {
						pos++;
					}
					size_t sel = bitvec_select(bv, bit, r);
					CuAssertSizeTEquals(tc, pos, sel);
				}
				for (size_t r = bitcount; r < bitcount + 20; r++) {
					size_t sel = bitvec_select(bv, bit, r);
					CuAssertSizeTEquals(tc, len, sel);
				}
				FREE(ba);
				bitvec_free(bv);
			}
		}
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void bitvec_test_format(CuTest *tc)
{
	memdbg_reset();
	bitvec *bv = bitvec_new_with_capacity(0);
	bool bit;
	for (size_t i = 0; i < ba_size; i++) {
		bit = ((byte_t)rand() % 2);
		bitvec_push(bv, bit);
	}
	bitvec_format *fmt = bitvec_get_format(bv, 4);
	format_fprint(bitvec_format_as_format(fmt), stdout);
	bitvec_format_free(fmt);
	bitvec_free(bv);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *bitvec_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, bitvec_test_new_with_len);
	SUITE_ADD_TEST(suite, bitvec_test_get_set);
	SUITE_ADD_TEST(suite, bitvec_test_push);
	SUITE_ADD_TEST(suite, bitvec_test_push_n);
	SUITE_ADD_TEST(suite, bitvec_test_count);
	SUITE_ADD_TEST(suite, bitvec_test_select);
	SUITE_ADD_TEST(suite, bitvec_test_format);
	return suite;
}