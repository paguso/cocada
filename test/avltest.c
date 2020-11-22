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
#include "vec.h"

void print_int(FILE *stream, const void *ptr)
{
	fprintf(stream, "%d", *((int *)ptr));
}


typedef struct {
	int key;
	double value;
} obj_t;


int cmp_obj_t (const void *l, const void *r) 
{
	obj_t *lo = *((obj_t **)l);
	obj_t *ro = *((obj_t **)r);
	if (lo->key == ro->key) return 0;
	if (lo->key < ro->key) return -1;
	else return +1;
}


void print_obj_t(FILE *stream, const void *ptr)
{
	fprintf(stream, "(key=%d value=%f)", (*((obj_t **)ptr))->key, (*((obj_t **)ptr))->value);
}


void test_avl_push(CuTest *tc)
{
	int half_univ = 100;
	// typed primitive values
	avl *tree = avl_new(cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		DEBUG("Insert %d\n", val);
		avl_push_int(tree, val);
		DEBUG_ACTION(avl_print(tree, stderr, print_int));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	FREE(tree, avl);

	// owned references with plain methods	
	tree = avl_new(cmp_obj_t);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t *obj = NEW(obj_t);
		obj->key = key;
		obj->value = (double)key;
		DEBUG("\n\nInsert owned obj %d\n", key);
		avl_push(tree, obj);
		DEBUG_ACTION(avl_print(tree, stderr, print_obj_t));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_obj_t));
	DEBUG("\n\n\n");
	DESTROY(tree, dtor_cons(DTOR(avl), ptr_dtor()));	

	// non-owned references with rawptr method
	vec *buf = vec_new(sizeof(obj_t));
	tree = avl_new(cmp_obj_t);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t obj = {.key = key, .value = (double)key};
		vec_push(buf, &obj);
		DEBUG("\n\nInsert non-owned obj %d\n", key);
		avl_push_rawptr(tree, (void *)vec_get(buf, i));
		DEBUG_ACTION(avl_print(tree, stderr, print_obj_t));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_obj_t));
	DEBUG("\n\n\n");
	FREE(tree, avl);
	FREE(buf, vec);
}



CuSuite *avl_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_avl_push);
	return suite;
}