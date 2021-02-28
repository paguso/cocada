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



#include "arrays.h"
#include "bitarr.h"
#include "bitbyte.h"
#include "csrsbitarray.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"

#include "CuTest.h"

static size_t nof_arrays;
static byte_t *ba_zeros, *ba_ones, *ba_odd, *ba_even, *ba_rand, *ba_alt;
static byte_t **all_ba;
static csrsbitarray *csrsba_zeros, *csrsba_ones, *csrsba_odd, *csrsba_even,
       *csrsba_rand, *csrsba_alt;
static csrsbitarray **all_srsba;

static size_t ba_size=7000;

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


void csrsbitarr_test_setup(CuTest *tc)
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
	all_ba = malloc(nof_arrays*sizeof(byte_t *));
	all_ba[0] = ba_zeros;
	all_ba[1] = ba_ones;
	all_ba[2] = ba_odd;
	all_ba[3] = ba_even;
	all_ba[4] = ba_rand;
	all_ba[5] = ba_alt;
	csrsba_zeros= csrsbitarr_new(ba_zeros, ba_size);
	csrsba_ones = csrsbitarr_new(ba_ones, ba_size);
	csrsba_odd  = csrsbitarr_new(ba_odd, ba_size);
	csrsba_even = csrsbitarr_new(ba_even, ba_size);
	csrsba_rand = csrsbitarr_new(ba_rand, ba_size);
	csrsba_alt = csrsbitarr_new(ba_alt, ba_size);
	//printf("------------ csrsba_zeros -------------\n");
	//csrsbitarr_print(csrsba_zeros, 4);
	//printf("------------ csrsba_ones  -------------\n");
	//csrsbitarr_print(csrsba_ones,  4);
	//printf("------------ csrsba_odd   -------------\n");
	//csrsbitarr_print(csrsba_odd,   4);
	//printf("------------ csrsba_even  -------------\n");
	//csrsbitarr_print(csrsba_even,  4);
	//printf("------------ csrsba_rand  -------------\n");
	//csrsbitarr_print(csrsba_rand,  4);
	//printf("------------ csrsba_alt  -------------\n");
	//csrsbitarr_print(csrsba_alt,  4);
	all_srsba = malloc(nof_arrays*sizeof(csrsbitarray *));
	all_srsba[0] = csrsba_zeros;
	all_srsba[1] = csrsba_ones;
	all_srsba[2] = csrsba_odd;
	all_srsba[3] = csrsba_even;
	all_srsba[4] = csrsba_rand;
	all_srsba[5] = csrsba_alt;

}

void csrsbitarr_test_teardown(CuTest *tc)
{
	csrsbitarr_free(csrsba_zeros, false);
	csrsbitarr_free(csrsba_ones, false);
	csrsbitarr_free(csrsba_odd, false);
	csrsbitarr_free(csrsba_even, false);
	csrsbitarr_free(csrsba_rand, false);
	free(ba_zeros);
	free(ba_ones);
	free(ba_odd);
	free(ba_even);
	free(ba_rand);
	free(all_ba);
	free(all_srsba);
}


void test_csrsbitarr_rank0(CuTest *tc)
{
	csrsbitarray *ba;
	size_t b, i, count;
	size_t *bf_ranks;
	size_t max_rank;

	bf_ranks = calloc(ba_size, sizeof(size_t));
	for (b=0; b<nof_arrays; b++) {
		//printf("Testing rank0 with bitarray #%zu:\n", b);

		ba = all_srsba[b];
		//compute ranks by brute force
		count = 0;
		for (i=0; i<ba_size; i++) {
			bf_ranks[i] = count;
			if (!bitarr_get_bit(all_ba[b], i))
				++count;
			//printf("rank0[%zu]=%zu\n", i, count);
		}
		max_rank=count;
		// then compare them with the function results
		for (i=0; i<ba_size; i++) {
			count = csrsbitarr_rank0(ba, i);
			//printf("rank0 of %zu = %zu  (expected %zu)\n", i, count, bf_ranks[i]);
			CuAssertSizeTEquals(tc, bf_ranks[i], count);
		}
		for (i=ba_size; i<ba_size+(3*BYTESIZE); i++) {
			count = csrsbitarr_rank0(ba, i);
			//printf("rank0 of %zu (>N) = %zu  (expected %zu)\n", i, count, max_rank);
			CuAssertSizeTEquals(tc, max_rank, count);
		}

	}
	free(bf_ranks);
}



void test_csrsbitarr_rank1(CuTest *tc)
{
	csrsbitarray *ba;
	size_t b, i, count;
	size_t *bf_ranks;
	size_t max_rank;

	bf_ranks = calloc(ba_size, sizeof(size_t));
	for (b=0; b<nof_arrays; b++) {
		//printf("Testing rank1 with bitarray #%zu:\n", b);

		ba = all_srsba[b];
		//compute ranks by brute force
		count = 0;
		for (i=0; i<ba_size; i++) {
			bf_ranks[i] = count;
			if (bitarr_get_bit(all_ba[b], i)) {
				++count;
			}
			//printf("rank1[%zu]=%zu\n", i, count);
		}
		max_rank = count;
		// then compare them with the function results
		for (i=0; i<ba_size; i++) {
			count = csrsbitarr_rank1(ba, i);
			//printf("rank1 of %zu = %zu  (expected %zu)\n", i, count, bf_ranks[i]);
			CuAssertSizeTEquals(tc, bf_ranks[i], count);
		}
		for (i=ba_size; i<ba_size+(3*BYTESIZE); i++) {
			count = csrsbitarr_rank1(ba, i);
			//printf("rank1 of %zu (>N) = %zu  (expected %zu)\n", i, count, max_rank);
			CuAssertSizeTEquals(tc, max_rank, count);
		}

	}
	free(bf_ranks);
}

void test_csrsbitarr_select0(CuTest *tc)
{
	csrsbitarray *ba;
	size_t *bf_selects;

	bf_selects = calloc(ba_size+1, sizeof(size_t));
	for (size_t b=0; b<nof_arrays; b++) {
		//printf("Testing select0 with bitarray #%zu:\n", b);
		ba = all_srsba[b];
		//compute selects by brute force
		size_t totalzeroes = 0;
		size_t rank = 0;
		for (size_t i=0; i<ba_size; i++) {
			if (bitarr_get_bit(all_ba[b], i)==0) {
				bf_selects[rank++] = i;
				//printf("select0[%zu]=%zu\n", rank-1, i);
			}
		}
		totalzeroes = rank;
		// then compare them with the function results
		for (rank=0; rank<totalzeroes; rank++) {
			size_t i = csrsbitarr_select0(ba, rank);
			//printf("select0 of %zu = %zu  (expected %zu)\n", rank, i, bf_selects[rank]);
			CuAssertSizeTEquals(tc, bf_selects[rank], i);
		}
		for (rank=totalzeroes+1; rank<totalzeroes+(3*BYTESIZE); rank++) {
			size_t i = csrsbitarr_select0(ba, rank);
			//printf("select0 of %zu (>total) = %zu (expected %zu)\n", rank, i, ba_size);
			CuAssertSizeTEquals(tc, ba_size, i);
		}
	}
	free(bf_selects);
}


void test_csrsbitarr_select1(CuTest *tc)
{
	csrsbitarray *ba;
	size_t *bf_selects;

	bf_selects = calloc(ba_size+1, sizeof(size_t));
	for (size_t b=0; b<nof_arrays; b++) {
		//printf("Testing select1 with bitarray #%zu:\n", b);
		ba = all_srsba[b];
		//compute selects by brute force
		size_t totalones = 0;
		size_t rank = 0;
		for (size_t i=0; i<ba_size; i++) {
			if (bitarr_get_bit(all_ba[b], i)==1) {
				bf_selects[rank++] = i;
				//printf("select1[%zu]=%zu\n", rank-1, i);
			}
		}
		totalones = rank;
		// then compare them with the function results
		for (rank=0; rank<totalones; rank++) {
			size_t i = csrsbitarr_select1(ba, rank);
			//printf("select1 of %zu = %zu  (expected %zu)\n", rank, i, bf_selects[rank]);
			CuAssertSizeTEquals(tc, bf_selects[rank], i);
		}
		for (rank=totalones+1; rank<totalones+(3*BYTESIZE); rank++) {
			size_t i = csrsbitarr_select1(ba, rank);
			//printf("select1 of %zu (>total) = %zu (expected %zu)\n", rank, i, ba_size);
			CuAssertSizeTEquals(tc, ba_size, i);
		}
	}
	free(bf_selects);
}



void test_csrsbitarr_pred(CuTest *tc)
{
	for (byte_t bit=0; bit<=1; bit++) {
		for (size_t b=0; b<nof_arrays; b++) {
			csrsbitarray *ba = all_srsba[b];

			size_t exp_pred = ba_size;
			for (size_t i=0; i<ba_size; i++) {
				size_t pred = csrsbitarr_pred(ba, i, bit);

				CuAssertSizeTEquals(tc, exp_pred, pred);

				if (csrsbitarr_get(ba, i)==bit)
					exp_pred = i;
			}
		}
	}
}


void test_csrsbitarr_succ(CuTest *tc)
{
	for (byte_t bit=0; bit<=1; bit++) {
		for (size_t b=0; b<nof_arrays; b++) {
			csrsbitarray *ba = all_srsba[b];

			size_t exp_succ = ba_size;
			for (long i=ba_size-1; i>0; i--) {
				size_t succ = csrsbitarr_succ(ba, i, bit);

				CuAssertSizeTEquals(tc, exp_succ, succ);

				if (csrsbitarr_get(ba, i)==bit)
					exp_succ = i;
			}
		}
	}
}


void csrsbitarr_test_empty(CuTest *tc)
{
	byte_t *ba_empty = ARR_NEW(byte_t, 0);
	csrsbitarray *b = csrsbitarr_new(ba_empty, 0);
	for (size_t i=0; i<2; i++) {
		//csrsbitarr_get(b, i);
		CuAssertSizeTEquals(tc, 0, csrsbitarr_rank0(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_rank1(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_select0(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_select1(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_pred0(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_pred1(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_succ0(b, i));
		CuAssertSizeTEquals(tc, 0, csrsbitarr_succ1(b, i));
	}
	csrsbitarr_free(b, true);
}



CuSuite *csrsbitarr_get_test_suite()
{
	CuSuite *suite;
	suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, csrsbitarr_test_setup);
	SUITE_ADD_TEST(suite, test_csrsbitarr_rank0);
	SUITE_ADD_TEST(suite, test_csrsbitarr_rank1);
	SUITE_ADD_TEST(suite, test_csrsbitarr_select0);
	SUITE_ADD_TEST(suite, test_csrsbitarr_select1);
	SUITE_ADD_TEST(suite, test_csrsbitarr_pred);
	SUITE_ADD_TEST(suite, test_csrsbitarr_succ);
	SUITE_ADD_TEST(suite, csrsbitarr_test_empty);
	SUITE_ADD_TEST(suite, csrsbitarr_test_teardown);
	return suite;
}
