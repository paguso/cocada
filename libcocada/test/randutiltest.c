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
#include <stdio.h>
#include <stdlib.h>


#include "CuTest.h"
#include "mathutil.h"
#include "randutil.h"
#include "memdbg.h"


void test_rand_unif(CuTest *tc)
{
	usize n = 1 << 20;
	usize nbins = 100;
	usize *counts = calloc(nbins, sizeof(usize));
	usize max_count = 0;
	for (usize i = 0; i < n; i++) {
		usize bin = rand_unif() * nbins;
		counts[bin]++;
		max_count = MAX(max_count, counts[bin]);
	}
	double resol = 100.0 / max_count;
	printf("Uniform(0,1) bins");
	for (int i = 0; i < nbins; i++) {
		printf("\n|");
		for (int j = 0; j < resol * counts[i]; j++) {
			printf("*");
		}
	}
	printf("\n");
	printf("max_count=%zu", max_count);
	free(counts);
}


void test_rand_norm(CuTest *tc)
{
	usize n = 1 << 20;
	usize nbins = 100;
	double range_min = -5.0, range_max = +5.0;
	double range = range_max - range_min;
	usize *counts = calloc(nbins, sizeof(usize));
	usize max_count = 0;
	for (usize i = 0; i < n; i++) {
		double x;
		do {
			x = rand_norm();
		}
		while (x < range_min || x > range_max);
		usize bin = ( ( x - range_min) / range ) * nbins;
		counts[bin]++;
		max_count = MAX(max_count, counts[bin]);
	}
	double resol = 100.0 / max_count;
	printf("Norm(0,1) bins");
	for (int i = 0; i < nbins; i++) {
		printf("\n|");
		for (int j = 0; j < resol * counts[i]; j++) {
			printf("*");
		}
	}
	printf("\n");
	printf("max_count=%zu", max_count);
	free(counts);
}

CuSuite *randutil_get_test_suite()
{

	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_rand_unif);
	SUITE_ADD_TEST(suite, test_rand_norm);
	return suite;

}


