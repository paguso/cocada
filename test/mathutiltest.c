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

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "CuTest.h"

#include "errlog.h"
#include "mathutil.h"


void test_mult_floor(CuTest *tc)
{
	size_t i, ntests = 1000000;
	double value, base, expected, actual;

	for (i=0; i<ntests; i++) {
		value = rand();
		base = rand();
		if (((unsigned int)rand())%2) value = -value;
		if (((unsigned int)rand())%2) base = -base;
		actual = DIVFLOOR(value, base);
		if (base==0) {
			if (value>=0) {
				expected = 0;
			} else {
				expected = -INFINITY;
			}
		} else if (base<0) {
			if (value<0) {
				expected = 0;
				while (expected*base>value) {
					expected+=1;
				}
			} else if (value>=0) {
				expected = 0;
				while ((expected-1)*base<value) {
					expected-=1;
				}
			}
		} else if (base>0) {
			if (value<0) {
				expected=0;
				while (expected*base>value) {
					expected-=1;
				}
			} else if (value>=0) {
				expected = 0;
				while ((expected+1)*base<value) {
					expected+=1;
				}
			}
		}
		//printf("mult_floor(%f, %f) = %f   exp=%f\n", value, base, actual, expected);
		CuAssertDblEquals(tc, expected, actual,0);
	}
}

void test_mult_ceil(CuTest *tc)
{
	size_t i, ntests = 1000000;
	double value, base, expected, actual;

	for (i=0; i<ntests; i++) {
		value = rand();
		base = rand();
		if (((unsigned int)rand())%2) value = -value;
		if (((unsigned int)rand())%2) base = -base;
		actual = DIVCEIL(value, base);
		if (base==0) {
			if (value>=0) {
				expected = 0;
			} else {
				expected = INFINITY;
			}
		} else if (base<0) {
			if (value<0) {
				expected = 0;
				while ((expected+1)*base>=value) {
					expected+=1;
				}
			} else if (value>=0) {
				expected = 0;
				while (expected*base<value) {
					expected-=1;
				}
			}
		} else if (base>0) {
			if (value<0) {
				expected=0;
				while ((expected-1)*base>=value) {
					expected-=1;
				}
			} else if (value>=0) {
				expected = 0;
				while (expected*base<value) {
					expected+=1;
				}
			}
		}
		//printf("mult_ceil(%f, %f) = %f   exp=%f\n", value, base, actual, expected);
		CuAssertDblEquals(tc, expected, actual,0);
	}
}



void test_is_prime(CuTest *tc)
{
	//bool a = is_prime(18023405708736723011);
	//bool b = naive_is_prime(18023405708736723011);

	uint64_t val = 1;
	for (size_t p=0; p<63; p++) {
		for (size_t n=0; n<1000; n++) {
			bool ispr = is_prime(val);
			DEBUG("%zu is %s prime\n", val, ispr?"":"NOT");
			if (ispr!=naive_is_prime(val)) {
				ispr = is_prime(val);
				CuAssert(tc, "primality test failed", ispr==naive_is_prime(val));
			}
			val++;
		}
		val <<= 1;
	}
}


void test_prime_succ(CuTest *tc)
{
	for (uint64_t n=0; n<100000; n++) {
		uint64_t psucc = prime_succ(n);
		DEBUG("The prime successor of %"PRIu64" is %"PRIu64"\n", n, psucc);
		for (uint64_t s=n+1; s<psucc; s++) {
			CuAssert(tc, "Prime successor not minimal", !is_prime(s));
		}
		CuAssert(tc, "Prime successor is NOT a prime", is_prime(psucc));
	}
}


CuSuite* mathutil_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_mult_floor);
	SUITE_ADD_TEST(suite, test_mult_ceil);
	//SUITE_ADD_TEST(suite, test_is_prime);
	SUITE_ADD_TEST(suite, test_prime_succ);
	return suite;
}
