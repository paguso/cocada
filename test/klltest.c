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

#include "arrays.h"
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
	double err = 0.01;
	kllsumm *summ = kll_new(sizeof(int), cmp_int, err);
	kll_print(summ, stderr, print_int);
	size_t n = 10000;
	int univ = 300;
	int *ranks = NEW_ARR_0(int, univ);
	for (int i = 0; i < n; i++) {
		double xval;
		do {
			xval = (rand_norm() + 1.0) / 7.0;
		} while (xval < 0 || xval >= 1);
		int val = xval * univ;
		ranks[val]++;
        DEBUG("KLL insert #%d value = %d\n", i, val);        
		kll_upd(summ, &val);
	}
	DEBUG_ACTION(kll_print(summ, stderr, print_int));
	double epsN = err * n;
	int nerr = 0;
	for (int sum=0, nxtsum=0, i=0; i<univ; i++) {
		nxtsum += ranks[i];
		ranks[i] = sum;
		size_t kllrk =  kll_rank(summ, &i);
		if( abs((double)kllrk - (double)ranks[i]) > epsN) {
			DEBUG("val=%d  rank=%d  kll_rank=%zu  rel.err=%f\n", i, ranks[i], kllrk, abs(((double)kllrk - (double)ranks[i]))/(double)n);
			nerr++;
		}
		sum = nxtsum;
	}
	DEBUG("# wrong ranks = %d\n", nerr);
	DEBUG("Observed P[Err>eN] = %f  (Predicted = %f)", (double)nerr/n, err);

}




CuSuite *kll_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_kll_upd);
	return suite;
}