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

#include "CuTest.h"

#include "errlog.h"
#include "kll.h"
#include "mathutil.h"
#include "randutil.h"


static void print_int(FILE *stream, const void *val)
{
	fprintf(stream, "%d", *((int*)val));
}


void test_kll_upd(CuTest *tc)
{
	kllsumm *summ = kll_new(sizeof(int), cmp_int, 0.2, 0.6);
	kll_print(summ, stderr, print_int);
	size_t n = 100;
	int max_val = 30;
	for (size_t i = 0; i < n; i++) {
		int val =  rand_next() % max_val;
        DEBUG("KLL insert %d\n", val);        
		kll_upd(summ, &val);
		DEBUG_ACTION(kll_print(summ, stderr, print_int));
	}

}




CuSuite *kll_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_kll_upd);
	return suite;
}