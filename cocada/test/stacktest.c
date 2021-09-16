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
#include <string.h>

#include "CuTest.h"

#include "stack.h"


void test_stack_push_pop(CuTest *tc)
{
	size_t len = 100;
	stack *q = stack_new(sizeof(double));
	CuAssertTrue(tc, stack_empty(q));

	for (size_t i=0; i<len; i++) {
		double d = (double)(i);
		//printf("pushing %f\n", d);
		stack_push(q, &d);
	}

	for (size_t i=len; i>=1; i--) {
		double d;
		stack_pop(q, &d);
		//printf("popping %f\n", *d);
		CuAssertDblEquals(tc, (double)(i-1), d, 0.2);
	}

	CuAssertTrue(tc, stack_empty(q));

}


void test_stack_push_pop_int(CuTest *tc)
{
	size_t len = 100;
	stack *q = stack_new(sizeof(int));
	CuAssertTrue(tc, stack_empty(q));

	for (size_t i=0; i<len; i++) {
		int d = i;
		//printf("pushing %d\n", d);
		stack_push_int(q, d);
	}

	for (size_t i=len; i>=1; i--) {
		int d = stack_pop_int(q);
		//printf("popping %d\n", d);
		CuAssertIntEquals(tc, i-1, d);
	}

	CuAssertTrue(tc, stack_empty(q));

}



CuSuite *stack_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_stack_push_pop);
	SUITE_ADD_TEST(suite, test_stack_push_pop_int);
	return suite;
}