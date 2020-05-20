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

#include "arrutil.h"
#include "bitbyte.h"
#include "bitvec.h"
#include "new.h"
#include "mathutil.h"

static size_t ba_size = 1000;

void bitvec_test_append(CuTest *tc)
{
	bitvec *bv = bitvec_new_with_capacity(0);
	byte_t *array;
	array = NEW_ARR(byte_t, ba_size);
	bool bit;
	for (int i=0; i<ba_size; i++) {
		bit = ((byte_t)rand()%2);
		bitvec_push(bv, bit);
		array[i] = bit;
	}
	//bitvec_print(bv, 8);
	for (int i=0; i<ba_size; i++) {
		bit = bitvec_get_bit(bv, i);
		CuAssertIntEquals(tc, array[i], bit);
	}
	//printf(".");
	bitvec_free(bv);
	FREE(array);
}

void bitvec_test_append_n(CuTest *tc)
{
	bitvec *bv = bitvec_new_with_capacity(0);
	byte_t *array;
	array = NEW_ARR(byte_t, ba_size);
	bool bit = false;
	size_t n=0, s=0;
	while (s<ba_size) {
		n = MAX(1, ((size_t)rand())%(ba_size-s));
		bit = 1-bit;
		//printf("current size=%zu adding %zu %c-bits\n",s, n, bit?'1':'0');
		bitvec_push_n(bv, n, bit);
		FILL_ARR(array, s, s+n, bit);
		s += n;
		//bitvec_print(bv, 8);
	}
	//bitvec_print(bv, 8);
	for (int i=0; i<ba_size; i++) {
		bit = bitvec_get_bit(bv, i);
		CuAssertIntEquals(tc, array[i], bit);
	}
	bitvec_free(bv);
	FREE(array);
}


void bitvec_test_count(CuTest *tc)
{
	bitvec *bv = bitvec_new_with_capacity(0);
	bool bit;
	size_t counts[2] = {0,0};
	for (size_t i=0; i<ba_size; i++) {
		bit = ((byte_t)rand()%2);
		bitvec_push(bv, bit);
		counts[bit]++;
	}
	/*
	    size_t c0 = bitvec_count(bv, 0);
	    CuAssertIntEquals(tc, counts[0], c0);

	    size_t c1 = bitvec_count(bv, 1);
	    CuAssertIntEquals(tc, counts[1], c1);
	*/
	bitvec_free(bv);
}


CuSuite *bitvec_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, bitvec_test_append);
	SUITE_ADD_TEST(suite, bitvec_test_append_n);
	SUITE_ADD_TEST(suite, bitvec_test_count);
	return suite;
}