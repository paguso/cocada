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
#include <stdlib.h>
#include <math.h>

#include "CuTest.h"

#include "order.h"
#include "gk.h"
#include "mathutil.h"
#include "randutil.h"

static void print_int(FILE *stream, const void *val)
{
	fprintf(stream, "%d", *((int *)val));
}


void test_gk_upd(CuTest *tc)
{
	size_t univ = 1 << 10;
	size_t *bfcount = calloc(univ, sizeof(size_t));
	size_t nupd = 4 * univ;
	gksumm *sk = gk_new(sizeof(int), cmp_int, 0.05);
	size_t max_count = 0;
	for (size_t i=0; i<nupd; i++) {
		double xval;
		do {
			xval = (rand_norm() + 1.0) / 7.0;
		}
		while (xval < 0 || xval >= 1);
		int val = xval * univ;
		//printf("Insert %d:\n", val);
		gk_upd(sk, &val);
		bfcount[val]++;
		max_count = MAX(max_count, bfcount[val]);
		//gk_print(sk, stdout, print_int);
		//printf("\n");
	}

	printf("\nGK for %zu values in [0,%zu):\n", nupd, univ );
	gk_print(sk, stdout, print_int);


	size_t nbins = 50;
	double resol = 100.0 / (max_count * (univ/nbins));
	printf("\n\nGK Values distribution (each * for approx %f units)",
	       (max_count * ((double)univ/(double)nbins))/100.0 );
	for (size_t i = 0, bin = 0; bin < nbins; bin++) {
		size_t bin_count = 0;
		while ( i < ((bin + 1) * univ) / nbins ) {
			bin_count += bfcount[i++];
		}
		printf("\n%10zu|",(bin*univ)/nbins);
		for (size_t j=0; j < resol * bin_count; j++) {
			printf("*");
		}
	}
	printf("\n");

	free(bfcount);
}


CuSuite *gk_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_gk_upd);
	return suite;
}