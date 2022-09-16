
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "arrays.h"
#include "bitarr.h"
#include "bitbyte.h"
#include "errlog.h"
#include "mathutil.h"
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
	/*
	roaringbitvec_free(rbv_zeros);
	roaringbitvec_free(rbv_ones);
	roaringbitvec_free(rbv_odd);
	roaringbitvec_free(rbv_even);
	roaringbitvec_free(rbv_rand);
	roaringbitvec_free(rbv_alt);
	free(ba_zeros);
	free(ba_ones);
	free(ba_odd);
	free(ba_even);
	free(ba_rand);
	free(ba_alt);*/
	free(all_ba);
	free(all_rbv);
}

void roaringbitvec_test_get(CuTest *tc) {
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
	for (float density = 0; density < 1; density += 0.1) {
		memdbg_reset();
		roaringbitvec *rbv = roaringbitvec_new(n);
		for (size_t i = 0; i < n; i++) {
			if (rand() < density * RAND_MAX) {
				roaringbitvec_set(rbv, i, true);
			}
		}
		size_t mem = memdbg_total();
		CuAssertSizeTEquals(tc, mem, roaringbitvec_memsize(rbv));
		DEBUG_IF(true, "N=%zu density=%f Roaring card=%zu memsize = %zu bytes (estimated = %zu)\n", 
				n, density, roaringbitvec_card(rbv), mem, (size_t)(density * n));
		roaringbitvec_free(rbv);
    	CuAssert(tc, "Memory leak.", memdbg_is_empty());
	}

}

/*
void test_roaringbitvec_rank0(CuTest *tc)
{
	roaringbitvec *ba;
	size_t b, i, count;
	size_t *bf_ranks;
	size_t max_rank;

	bf_ranks = calloc(ba_size, sizeof(size_t));
	for (b=0; b<nof_arrays; b++) {
		//printf("Testing rank0 with bitarray #%zu:\n", b);

		ba = all_rbv[b];
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
			count = roaringbitvec_rank0(ba, i);
			//printf("rank0 of %zu = %zu  (expected %zu)\n", i, count, bf_ranks[i]);
			CuAssertSizeTEquals(tc, bf_ranks[i], count);
		}
		for (i=ba_size; i<ba_size+(3*BYTESIZE); i++) {
			count = roaringbitvec_rank0(ba, i);
			//printf("rank0 of %zu (>N) = %zu  (expected %zu)\n", i, count, max_rank);
			CuAssertSizeTEquals(tc, max_rank, count);
		}

	}
	free(bf_ranks);
}



void test_roaringbitvec_rank1(CuTest *tc)
{
	roaringbitvec *ba;
	size_t b, i, count;
	size_t *bf_ranks;
	size_t max_rank;

	bf_ranks = calloc(ba_size, sizeof(size_t));
	for (b=0; b<nof_arrays; b++) {
		//printf("Testing rank1 with bitarray #%zu:\n", b);

		ba = all_rbv[b];
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
			count = roaringbitvec_rank1(ba, i);
			//printf("rank1 of %zu = %zu  (expected %zu)\n", i, count, bf_ranks[i]);
			CuAssertSizeTEquals(tc, bf_ranks[i], count);
		}
		for (i=ba_size; i<ba_size+(3*BYTESIZE); i++) {
			count = roaringbitvec_rank1(ba, i);
			//printf("rank1 of %zu (>N) = %zu  (expected %zu)\n", i, count, max_rank);
			CuAssertSizeTEquals(tc, max_rank, count);
		}

	}
	free(bf_ranks);
}

void test_roaringbitvec_select0(CuTest *tc)
{
	roaringbitvec *ba;
	size_t *bf_selects;

	bf_selects = calloc(ba_size+1, sizeof(size_t));
	for (size_t b=0; b<nof_arrays; b++) {
		//printf("Testing select0 with bitarray #%zu:\n", b);
		ba = all_rbv[b];
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
			size_t i = roaringbitvec_select0(ba, rank);
			//printf("select0 of %zu = %zu  (expected %zu)\n", rank, i, bf_selects[rank]);
			CuAssertSizeTEquals(tc, bf_selects[rank], i);
		}
		for (rank=totalzeroes+1; rank<totalzeroes+(3*BYTESIZE); rank++) {
			size_t i = roaringbitvec_select0(ba, rank);
			//printf("select0 of %zu (>total) = %zu (expected %zu)\n", rank, i, ba_size);
			CuAssertSizeTEquals(tc, ba_size, i);
		}
	}
	free(bf_selects);
}


void test_roaringbitvec_select1(CuTest *tc)
{
	roaringbitvec *ba;
	size_t *bf_selects;

	bf_selects = calloc(ba_size+1, sizeof(size_t));
	for (size_t b=0; b<nof_arrays; b++) {
		//printf("Testing select1 with bitarray #%zu:\n", b);
		ba = all_rbv[b];
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
			size_t i = roaringbitvec_select1(ba, rank);
			//printf("select1 of %zu = %zu  (expected %zu)\n", rank, i, bf_selects[rank]);
			CuAssertSizeTEquals(tc, bf_selects[rank], i);
		}
		for (rank=totalones+1; rank<totalones+(3*BYTESIZE); rank++) {
			size_t i = roaringbitvec_select1(ba, rank);
			//printf("select1 of %zu (>total) = %zu (expected %zu)\n", rank, i, ba_size);
			CuAssertSizeTEquals(tc, ba_size, i);
		}
	}
	free(bf_selects);
}



void test_roaringbitvec_pred(CuTest *tc)
{
	for (byte_t bit=0; bit<=1; bit++) {
		for (size_t b=0; b<nof_arrays; b++) {
			roaringbitvec *ba = all_rbv[b];

			size_t exp_pred = ba_size;
			for (size_t i=0; i<ba_size; i++) {
				size_t pred = roaringbitvec_pred(ba, i, bit);

				CuAssertSizeTEquals(tc, exp_pred, pred);

				if (roaringbitvec_get(ba, i)==bit)
					exp_pred = i;
			}
		}
	}
}


void test_roaringbitvec_succ(CuTest *tc)
{
	for (byte_t bit=0; bit<=1; bit++) {
		for (size_t b=0; b<nof_arrays; b++) {
			roaringbitvec *ba = all_rbv[b];

			size_t exp_succ = ba_size;
			for (long i=ba_size-1; i>0; i--) {
				size_t succ = roaringbitvec_succ(ba, i, bit);

				CuAssertSizeTEquals(tc, exp_succ, succ);

				if (roaringbitvec_get(ba, i)==bit)
					exp_succ = i;
			}
		}
	}
}


void roaringbitvec_test_empty(CuTest *tc)
{
	byte_t *ba_empty = ARR_NEW(byte_t, 0);
	roaringbitvec *b = roaringbitvec_new(0);
	for (size_t i=0; i<2; i++) {
		//roaringbitvec_get(b, i);
		CuAssertSizeTEquals(tc, 0, roaringbitvec_rank0(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_rank1(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_select0(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_select1(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_pred0(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_pred1(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_succ0(b, i));
		CuAssertSizeTEquals(tc, 0, roaringbitvec_succ1(b, i));
	}
	roaringbitvec_free(b);
}
*/


CuSuite *roaringbitvec_get_test_suite()
{
	CuSuite *suite;
	suite = CuSuiteNew();
    /*
	SUITE_ADD_TEST(suite, roaringbitvec_test_setup);
	SUITE_ADD_TEST(suite, test_roaringbitvec_rank0);
	SUITE_ADD_TEST(suite, test_roaringbitvec_rank1);
	SUITE_ADD_TEST(suite, test_roaringbitvec_select0);
	SUITE_ADD_TEST(suite, test_roaringbitvec_select1);
	SUITE_ADD_TEST(suite, test_roaringbitvec_pred);
	SUITE_ADD_TEST(suite, test_roaringbitvec_succ);
	SUITE_ADD_TEST(suite, roaringbitvec_test_empty);
	SUITE_ADD_TEST(suite, roaringbitvec_test_teardown);
    */
	SUITE_ADD_TEST(suite, roaringbitvec_test_memsize);
	SUITE_ADD_TEST(suite, roaringbitvec_test_get);

	return suite;
}
