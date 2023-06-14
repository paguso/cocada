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
#include <time.h>

#include "arrays.h"
#include "bitarr.h"
#include "bitbyte.h"
#include "errlog.h"
#include "mathutil.h"
#include "memdbg.h"
#include "randutil.h"
#include "memdbg.h"
#include "new.h"
#include "roaring.h"

#include "CuTest.h"

static size_t nof_arrays;
static byte_t *ba_zeros, *ba_ones, *ba_odd, *ba_even, *ba_rand, *ba_alt;
static byte_t **all_ba;
static roaringbitvec *rbv_zeros, *rbv_ones, *rbv_odd, *rbv_even,
       *rbv_rand, *rbv_alt;
static roaringbitvec **all_rbv;

static size_t ba_size = (1 << 18) + 725;

static void reset_arrays()
{
	size_t i, j, ba_byte_size;
	ba_byte_size =  (size_t)DIVCEIL(ba_size, BYTESIZE);
	for (i=0; i<ba_byte_size; i++) {
		ba_zeros[i] = 0x0;
		ba_ones[i] = ~(0x0);
		ba_odd[i] = 0x0;
		ba_even[i] = 0x0;
		for (j=0; j<BYTESIZE; j+=2) {
			ba_odd[i] |= ((0x1)<<j);
			ba_even[i] |= ((0x2)<<j);
		}
		ba_rand[i] = (byte_t)rand();
		ba_alt[i] = (i%2)?(~0x0):0x0;
	}
}


void roaringbitvec_test_setup(CuTest *tc)
{
	size_t ba_byte_size;
	ba_byte_size =  (size_t)DIVCEIL(ba_size, BYTESIZE);
	nof_arrays = 6;
	ba_zeros = malloc(ba_byte_size);
	ba_ones = malloc(ba_byte_size);
	ba_odd = malloc(ba_byte_size);
	ba_even = malloc(ba_byte_size);
	ba_rand = malloc(ba_byte_size);
	ba_alt = malloc(ba_byte_size);
	reset_arrays();
	//ba_print(ba_zeros, ba_size, 4);
	//ba_print(ba_ones, ba_size, 4);
	//bitarr_print(ba_odd, ba_size, 4);
	//ba_print(ba_even, ba_size, 4);
	//ba_print(ba_rand, ba_size, 4);
	all_ba = malloc(nof_arrays * sizeof(byte_t *));
	all_ba[0] = ba_zeros;
	all_ba[1] = ba_ones;
	all_ba[2] = ba_odd;
	all_ba[3] = ba_even;
	all_ba[4] = ba_rand;
	all_ba[5] = ba_alt;
	rbv_zeros= roaringbitvec_new_from_bitarr(ba_zeros, ba_size);
	rbv_ones = roaringbitvec_new_from_bitarr(ba_ones, ba_size);
	rbv_odd  = roaringbitvec_new_from_bitarr(ba_odd, ba_size);
	rbv_even = roaringbitvec_new_from_bitarr(ba_even, ba_size);
	rbv_rand = roaringbitvec_new_from_bitarr(ba_rand, ba_size);
	rbv_alt = roaringbitvec_new_from_bitarr(ba_alt, ba_size);
	//printf("------------ rbv_zeros -------------\n");
	//roaringbitvec_print(rbv_zeros, 4);
	//printf("------------ rbv_ones  -------------\n");
	//roaringbitvec_print(rbv_ones,  4);
	//printf("------------ rbv_odd   -------------\n");
	//roaringbitvec_print(rbv_odd,   4);
	//printf("------------ rbv_even  -------------\n");
	//roaringbitvec_print(rbv_even,  4);
	//printf("------------ rbv_rand  -------------\n");
	//roaringbitvec_print(rbv_rand,  4);
	//printf("------------ rbv_alt  -------------\n");
	//roaringbitvec_print(rbv_alt,  4);
	all_rbv = malloc(nof_arrays * sizeof(roaringbitvec *));
	all_rbv[0] = rbv_zeros;
	all_rbv[1] = rbv_ones;
	all_rbv[2] = rbv_odd;
	all_rbv[3] = rbv_even;
	all_rbv[4] = rbv_rand;
	all_rbv[5] = rbv_alt;

}

void roaringbitvec_test_teardown(CuTest *tc)
{
	for (size_t i = 0 ; i < nof_arrays; i++) {
		roaringbitvec_free(all_rbv[i]);
		free(all_ba[i]);
	}
	free(all_ba);
	free(all_rbv);
}


void roaringbitvec_test_get(CuTest *tc)
{
	memdbg_reset();
	roaringbitvec_test_setup(tc);
	for (size_t j = 0; j < nof_arrays; j++) {
		byte_t *ba = all_ba[j];
		roaringbitvec *rbv = all_rbv[j];
		for (size_t i=0; i < ba_size; i++) {
			bool a = bitarr_get_bit(ba,  i);
			bool b = roaringbitvec_get(rbv, i);
			if (a != b) {
				DEBUG_EXEC(printf("i=%zu\n", i));
				roaringbitvec_fprint(stdout, rbv);
			}
			CuAssert(tc, "Wrong value", a == b);
		}
	}
	roaringbitvec_test_teardown(tc);
	if (!memdbg_is_empty()) {
		DEBUG_EXEC(memdbg_print_stats(stdout, true));
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void roaringbitvec_test_memsize(CuTest *tc)
{
	size_t n = 1 << 20;
	for (float density = 0.005; density < 1; density *= 1.25) {
		memdbg_reset();
		roaringbitvec *rbv = roaringbitvec_new(n);
		for (size_t i = 0; i < n; i++) {
			if (rand() < density * RAND_MAX) {
				roaringbitvec_set(rbv, i, true);
			}
		}
		size_t mem = memdbg_total();
		//CuAssertSizeTEquals(tc, mem, roaringbitvec_memsize(rbv));
		DEBUG_IF(true,
		         "N=%zu density=%f Roaring card=%zu memsize = %zu bytes (estimated = %zu)\n",
		         n, density, roaringbitvec_card(rbv), mem, (size_t)(density * n));
		roaringbitvec_free(rbv);
		CuAssert(tc, "Memory leak.", memdbg_is_empty());
	}

}


void roaringbitvec_test_rank(CuTest *tc)
{
	byte_t bit_patterns[6] = {0x00, 0xFF, 0x0F, 0xF0, 0x55, 0xAA};
	memdbg_reset();
	for (int intbit=0; intbit<2; intbit++) {
		bool bit = (bool)intbit;
		for (int j=-1; j <= 18; j++) {
			size_t len = (j < 0) ? 0 : (1 << j) + ((j % 2) * 25);
			for (int pat = 0; pat < 6; pat++) {	
				printf("rank pat=%d len=%zu bit=%d\n",pat ,len, (int)bit);
				byte_t *ba = bitarr_new(len);
				memset(ba, bit_patterns[pat], DIVCEIL(len, BYTESIZE));
				roaringbitvec *bv = roaringbitvec_new_from_bitarr(ba, len);
				uint32_t expec_rank = 0;
				for (size_t i = 0; i < len; i++) {
					uint32_t rank = roaringbitvec_rank(bv, bit, i);
					if (rank != expec_rank) 
					CuAssertULlongEquals(tc, expec_rank, rank);
					expec_rank += (roaringbitvec_get(bv, i) == bit);
				}
				CuAssertULongEquals(tc, roaringbitvec_count(bv, bit), expec_rank);
				for (size_t i = len; i < len + 20; i++) {
					uint32_t rank = roaringbitvec_rank(bv, bit, i);
					if (rank != expec_rank)
					CuAssertULlongEquals(tc, expec_rank, rank);
				}
				FREE(ba);
				roaringbitvec_free(bv);
			}
		}
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void roaringbitvec_test_select(CuTest *tc)
{
	byte_t bit_patterns[6] = {0x00, 0xFF, 0x0F, 0xF0, 0x55, 0xAA};
	memdbg_reset();
	for (int intbit=0; intbit<2; intbit++) {
		bool bit = (bool)intbit;
		for (int j=-1; j <= 18; j++) {
			size_t len = (j < 0) ? 0 : (1 << j) + ((j % 2) * 25);
			for (int i = 0; i < 6; i++) {	
				printf("sel i=%d len=%zu bit=%d\n",i,len, (int)bit);
				byte_t *ba = bitarr_new(len);
				memset(ba, bit_patterns[i], DIVCEIL(len, BYTESIZE));
				roaringbitvec *bv = roaringbitvec_new_from_bitarr(ba, len);
				size_t bitcount = (bit) ? roaringbitvec_card(bv) : len - roaringbitvec_card(bv);
				size_t rank = 0;
				size_t pos = 0;
				for (size_t r = 0; r < bitcount; r++) {
					while(pos < len && rank < r) {
						rank += (roaringbitvec_get(bv, pos++) == bit);
					}
					while (pos < len && roaringbitvec_get(bv, pos) != bit) {
						pos++;
					}
					size_t sel = roaringbitvec_select(bv, bit, r);
					if (pos != sel) 
					CuAssertSizeTEquals(tc, pos, sel);
				}
				for (size_t r = bitcount; r < bitcount + 20; r++) {
					size_t sel = roaringbitvec_select(bv, bit, r);
					if (len != sel)
					CuAssertSizeTEquals(tc, len, sel);
				}
				FREE(ba);
				roaringbitvec_free(bv);
			}
		}
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}



void test_roaringbitvec_speed_rank(CuTest *tc)
{
	size_t size = 1<<30;
	memdbg_reset();
	roaringbitvec *bv = roaringbitvec_new(size);
	size_t count = 0;
	time_t t = time(NULL);
	for (size_t i=0; i<size; i++) {
		bool bit = rand() % 2;
		roaringbitvec_set(bv, i, bit);
		//size_t rank = roaringbitvec_rank(bv, i);
		//CuAssertSizeTEquals(tc, rank, count1);
		count += bit;
	}
	t = time(NULL) - t;
	printf("count=%zu build time=%ld\n", count, t);
	t = time(NULL);
	size_t old_rank = 0, nop=0;
	for (size_t i=0; i < size; i += (1<<7)) {
		//bool bit = rand() % 2;
		//roaringbitvec_set(bv, i, bit);
		size_t rank = roaringbitvec_rank1(bv, i);
		//CuAssertSizeTEquals(tc, rank, count1);
		count -= (rank - old_rank);
		old_rank = rank;
		nop++;
	}
	t = time(NULL) - t;
	roaringbitvec_fit(bv);
	size_t mem = memdbg_total();
	printf("count=%zu mem=%zu nop=%zu rank time=%ld\n", count, mem, nop, t);
}


CuSuite *roaringbitvec_get_test_suite()
{
	CuSuite *suite;
	suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, roaringbitvec_test_memsize);
	SUITE_ADD_TEST(suite, roaringbitvec_test_get);
	SUITE_ADD_TEST(suite, roaringbitvec_test_rank);
	//SUITE_ADD_TEST(suite, test_roaringbitvec_speed_rank);
	SUITE_ADD_TEST(suite, roaringbitvec_test_select);

	return suite;
}
