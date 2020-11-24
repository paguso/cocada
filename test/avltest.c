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
#include "iter.h"
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


void test_avl_ins(CuTest *tc)
{
	int half_univ = 100;
	// typed primitive values
	avl *tree = avl_new(cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		DEBUG("Insert %d\n", val);
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins_int(tree, val));
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
		CuAssert(tc, "Failed AVL push", avl_ins(tree, obj));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins(tree, obj));
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
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, (void *)vec_get(buf, i)));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins_rawptr(tree, (void *)vec_get(buf, i)));
		DEBUG_ACTION(avl_print(tree, stderr, print_obj_t));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_obj_t));
	DEBUG("\n\n\n");
	FREE(tree, avl);
	FREE(buf, vec);
}


void test_avl_get(CuTest *tc)
{
	int half_univ = 100;
	// typed primitive values
	avl *tree = avl_new(cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		int getval;
		CuAssert(tc, "Wrongly successful AVL search", !avl_get_int(tree, val, &getval));
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		CuAssert(tc, "Failed AVL search", avl_get_int(tree, val, &getval));
		CuAssertIntEquals(tc, val, getval);
	}
	FREE(tree, avl);

	// owned references with plain methods
	tree = avl_new(cmp_obj_t);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t *obj = NEW(obj_t);
		obj_t *get_obj;
		obj->key = key;
		obj->value = (double)key;
		CuAssert(tc, "Wrongly successful AVL search", !avl_get(tree, obj, (void **)&get_obj));
		CuAssert(tc, "Failed AVL push", avl_ins(tree, obj));
		CuAssert(tc, "Failed AVL search", avl_get(tree, obj, (void **)&get_obj));
		CuAssertPtrEquals(tc, obj, get_obj);
	}
	DESTROY(tree, dtor_cons(DTOR(avl), ptr_dtor()));

	// non-owned references with rawptr method
	vec *buf = vec_new(sizeof(obj_t));
	tree = avl_new(cmp_obj_t);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t obj = {.key = key, .value = (double)key};
		obj_t *get_obj;
		vec_push(buf, &obj);
		CuAssert(tc, "Wrongly successful AVL search", !avl_get_rawptr(tree, (void *)vec_get(buf, i), (rawptr *)&get_obj));
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, (void *)vec_get(buf, i)));
		CuAssert(tc, "Failed AVL search", avl_get_rawptr(tree, (void *)vec_get(buf, i), (rawptr *)&get_obj));
		CuAssertPtrEquals(tc, (void *)vec_get(buf,i), get_obj);
	}
	FREE(tree, avl);
	FREE(buf, vec);
}


void test_avl_del(CuTest *tc)
{
	int half_univ = 10;
	// typed primitive values
	avl *tree = avl_new(cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		//DEBUG("Insert %d\n", val);
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		//DEBUG_ACTION(avl_print(tree, stderr, print_int));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	int res;
	// remove non-existent value
	CuAssert(tc, "Attempt to delete non-existent element should return false",
	         ! avl_del_int(tree, 2*half_univ, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	// remove leaf node
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 2, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	CuAssert(tc, "Failed deletion", avl_del_int(tree, 4, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	CuAssert(tc, "Failed deletion", avl_del_int(tree, 6, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	CuAssert(tc, "Failed deletion", avl_del_int(tree, 13, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	CuAssert(tc, "Failed deletion", avl_del_int(tree, 17, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	CuAssert(tc, "Failed deletion", avl_del_int(tree, 19, &res));
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	FREE(tree, avl);
}



void test_avl_get_iter(CuTest *tc)
{
	int half_univ = 10;
	// typed primitive values
	avl *tree = avl_new(cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		//DEBUG("Insert %d\n", val);
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		//DEBUG_ACTION(avl_print(tree, stderr, print_int));
	}
	DEBUG_ACTION(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");

	avl_iter *it = avl_get_iter(tree, PRE_ORDER);
	for (int k=0; iter_has_next(avl_iter_as_iter(it)); k++) {
		int val = *((int *)iter_next(avl_iter_as_iter(it)));
		DEBUG("Pre-order[%d] = %d\n",k, val);		
	}
	avl_iter_free(it);

	DEBUG("\n\n");
	it = avl_get_iter(tree, IN_ORDER);
	for (int k=0; iter_has_next(avl_iter_as_iter(it)); k++) {
		int val = *((int *)iter_next(avl_iter_as_iter(it)));
		DEBUG("In-order[%d] = %d\n",k, val);		
	}
	avl_iter_free(it);

	DEBUG("\n\n");
	it = avl_get_iter(tree, POST_ORDER);
	for (int k=0; iter_has_next(avl_iter_as_iter(it)); k++) {
		int val = *((int *)iter_next(avl_iter_as_iter(it)));
		DEBUG("Post-order[%d] = %d\n",k, val);		
	}
	avl_iter_free(it);

	DEBUG("\n\n");
	it = avl_get_iter(tree, IN_ORDER);
	int k = 0;
	FOREACH_IN_ITER(val, int, avl_iter_as_iter(it)) {
		DEBUG("In-order[%d] = %d\n",k++, *val);		
	}
	avl_iter_free(it);

}



CuSuite *avl_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_avl_ins);
	SUITE_ADD_TEST(suite, test_avl_get);
	SUITE_ADD_TEST(suite, test_avl_del);
	SUITE_ADD_TEST(suite, test_avl_get_iter);
	return suite;
}