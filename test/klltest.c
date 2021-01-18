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
	fprintf(stream, "%d", *((int *)val));
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
		}
		while (xval < 0 || xval >= 1);
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
		DEBUG("val=%d  rank=%d  kll_rank=%zu  rel.err=%f\n", i, ranks[i], kllrk,
		      abs(((double)kllrk - (double)ranks[i]))/(double)n);
		if ( abs((double)kllrk - (double)ranks[i]) > epsN) {
			nerr++;
		}
		sum = nxtsum;
	}
	DEBUG("# wrong ranks = %d\n", nerr);
	DEBUG("Observed P[Err>eN] = %f  (Predicted = %f)", (double)nerr/n, err);

}


typedef struct _kll_obj {
	int key;
	double dkey;
} kll_obj;


int cmp_kll_obj(const void *l, const void *r)
{
	kll_obj *lo = *((kll_obj **)l);
	kll_obj *ro = *((kll_obj **)r);
	if (lo->key < ro->key) return -1;
	else if (lo->key > ro->key) return +1;
	else return 0;
}


void test_kll_upd_obj (CuTest *tc)
{
	int n = 10000;
	int univ = 300;
	double err = 0.1;
	kllsumm *summ = kll_new(sizeof(kllsumm *), cmp_kll_obj, err);
	for (int i=0; i<n; i++) {
		int k = rand_range_int(0, univ);
		kll_obj *obj = NEW(kll_obj);
		obj->key = k;
		obj->dkey = 2.0 * k;
		kll_upd(summ, &obj);
	}
	kll_obj *val = NEW(kll_obj);
	for (int i=0; i<univ; i++) {
		val->key = i;
		size_t r = kll_rank(summ, &val);
		DEBUG("KLL Obj rank (key=%d) = %zu\n", i, r);
	}
	FREE(val);
	DESTROY(summ, dtor_cons(DTOR(kll), ptr_dtor()));
}





CuSuite *kll_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_kll_upd);
	SUITE_ADD_TEST(suite, test_kll_upd_obj);
	return suite;
}