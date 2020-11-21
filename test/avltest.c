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
#include "avl.h"
#include "errlog.h"
#include "order.h"
#include "randutil.h"


void print_int(FILE *stream, const void *ptr)
{
	fprintf(stream, "%d", *((int *)ptr));
}


void test_avl_push(CuTest *tc)
{
	int half_univ = 100;
	avl *tree = avl_new(sizeof(int), cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		DEBUG("Insert %d\n", val);
		avl_push(tree, &val);
		DEBUG_ACTION(avl_print(tree, stderr, print_int));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	DESTROY(tree, dtor_cons(DTOR(avl), ptr_dtor()));
}



CuSuite *avl_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_avl_push);
	return suite;
}