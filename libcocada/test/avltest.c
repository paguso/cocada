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
#include "memdbg.h"
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
	obj_t *lo = (obj_t *)l;
	obj_t *ro = (obj_t *)r;
	if (lo->key == ro->key) return 0;
	if (lo->key < ro->key) return -1;
	else return +1;
}


void print_obj_t(FILE *stream, const void *ptr)
{
	fprintf(stream, "(key=%d value=%f)", ((obj_t *)ptr)->key,
	        ((obj_t *)ptr)->value);
}


int cmp_obj_t_refs (const void *l, const void *r)
{
	obj_t *lo = *((obj_t **)l);
	obj_t *ro = *((obj_t **)r);
	if (lo->key == ro->key) return 0;
	if (lo->key < ro->key) return -1;
	else return +1;
}


void print_obj_t_ref(FILE *stream, const void *ptr)
{
	fprintf(stream, "@%p(key=%d value=%f)", *((obj_t **)ptr),
	        (*(((obj_t **)ptr)))->key,
	        (*((obj_t **)ptr))->value);
}



void test_avl_ins(CuTest *tc)
{
	memdbg_reset();
	int half_univ = 100;
	// typed primitive values
	avl *tree = avl_new(sizeof(int), cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		DEBUG("Insert %d\n", val);
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins_int(tree, val));
		DEBUG_EXEC(avl_print(tree, stderr, print_int));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	DESTROY_FLAT(tree, avl);
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// nodes with flat objects
	tree = avl_new(sizeof(obj_t), cmp_obj_t);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t obj;
		obj.key = key;
		obj.value = (double)key;
		DEBUG("\n\nInsert flat obj %d\n", key);
		CuAssert(tc, "Failed AVL push", avl_ins(tree, &obj));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins(tree, &obj));
		DEBUG_EXEC(avl_print(tree, stderr, print_obj_t));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t));
	DEBUG("\n\n\n");
	DESTROY(tree, finaliser_cons(FNR(avl), finaliser_new_empty()));
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// non-owned references with rawptr method
	vec *buf = vec_new(sizeof(obj_t));
	tree = avl_new(sizeof(obj_t *), cmp_obj_t_refs);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t obj = {.key = key, .value = (double)key};
		vec_push(buf, &obj);
		DEBUG("\n\nInsert non-owned obj %d\n", key);
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, (void *)vec_get(buf, i)));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins_rawptr(tree,
		         (void *)vec_get(buf, i)));
		DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	DEBUG("\n\n\n");
	DESTROY_FLAT(tree, avl);
	DESTROY_FLAT(buf, vec);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// owned references
	tree = avl_new(sizeof(obj_t *), cmp_obj_t_refs);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t *obj = NEW(obj_t);
		obj->key = key;
		obj->value = (double)key;
		DEBUG("\n\nInsert owned obj %d\n", key);
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, obj));
		CuAssert(tc, "Wrongly well-succeeded AVL push", !avl_ins_rawptr(tree, obj));
		DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	DEBUG("\n\n\n");
	DESTROY(tree, finaliser_cons(FNR(avl), finaliser_new_ptr()));
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_avl_get(CuTest *tc)
{
	DEBUG("AVL Get");
	memdbg_reset();
	int half_univ = 100;
	// typed primitive values
	avl *tree = avl_new(sizeof(int), cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		DEBUG("Get %d\n", val);
		CuAssert(tc, "AVL get error", !avl_contains_int(tree, val));
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		CuAssert(tc, "AVL get error", avl_contains_int(tree, val));
		DEBUG_EXEC(avl_print(tree, stderr, print_int));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	DESTROY_FLAT(tree, avl);
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// nodes with flat objects
	tree = avl_new(sizeof(obj_t), cmp_obj_t);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t obj;
		obj.key = key;
		obj.value = (double)key;
		DEBUG("\n\nGet flat obj %d\n", key);
		const obj_t *ans = avl_get(tree, &obj);
		CuAssert(tc, "AVL Get Fail", ans == NULL);
		CuAssert(tc, "Failed AVL push", avl_ins(tree, &obj));
		ans = avl_get(tree, &obj);
		CuAssert(tc, "AVL Get Fail", cmp_obj_t(&obj, ans) == 0);
		DEBUG_EXEC(avl_print(tree, stderr, print_obj_t));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t));
	DEBUG("\n\n\n");
	DESTROY(tree, finaliser_cons(FNR(avl), finaliser_new_empty()));
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// non-owned references with rawptr method
	vec *buf = vec_new(sizeof(obj_t));
	tree = avl_new(sizeof(obj_t *), cmp_obj_t_refs);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t obj = {.key = key, .value = (double)key};
		vec_push(buf, &obj);
		obj_t *obj_ref = (obj_t *)vec_get(buf, i);
		DEBUG("\n\nInsert non-owned obj %d\n", key);
		CuAssert(tc, "AVL Get error", avl_get(tree, &obj_ref) == NULL);
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, obj_ref));
		CuAssert(tc, "AVL Get error", avl_contains_rawptr(tree, obj_ref));
		DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	DEBUG("\n\n\n");
	DESTROY_FLAT(tree, avl);
	DESTROY_FLAT(buf, vec);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// owned references
	tree = avl_new(sizeof(obj_t *), cmp_obj_t_refs);
	for (int i = 0; i < half_univ; i++) {
		int key = half_univ + ((i % 2) ? i : -i);
		obj_t *obj = NEW(obj_t);
		obj->key = key;
		obj->value = (double)key;
		DEBUG("\n\nInsert owned obj %d\n", key);
		CuAssert(tc, "AVL Get error", avl_get(tree, &obj) == NULL);
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, obj));
		CuAssert(tc, "AVL Get error", cmp_obj_t_refs(avl_get(tree, &obj), &obj) == 0);
		DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	DEBUG("\n\n\n");
	DESTROY(tree, finaliser_cons(FNR(avl), finaliser_new_ptr()));
	memdbg_print_stats(stdout, false);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_avl_remv(CuTest *tc)
{
	DEBUG("AVL remove");
	memdbg_reset();
	int half_univ = 10;
	// typed primitive values
	avl *tree = avl_new(sizeof(int), cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		//DEBUG("Insert %d\n", val);
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		//DEBUG_ACTION(avl_print(tree, stderr, print_int));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	// remove non-existent value
	CuAssert(tc, "Attempt to delete non-existent element should return false",
	         ! avl_del_int(tree, 2*half_univ));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	// remove leaf node
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 2));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 4));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 6));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 13));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 17));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	CuAssert(tc, "Failed deletion", avl_del_int(tree, 19));
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
	DEBUG("\n\n\n");
	DESTROY_FLAT(tree, avl);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());

	// owned references
	tree = avl_new(sizeof(obj_t), cmp_obj_t_refs);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		//DEBUG("Insert %d\n", val);
		obj_t *obj = NEW(obj_t);
		obj->key = val;
		obj->value = (double) val;
		CuAssert(tc, "Failed AVL push", avl_ins_rawptr(tree, obj));
		//DEBUG_ACTION(avl_print(tree, stderr, print_int));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	DEBUG("\n\n\n");
	// remove non-existent value
	obj_t obj;
	obj_t *probe = &obj, *dest;
	obj.key = 2*half_univ;
	CuAssert(tc, "Attempt to delete non-existent element should return false",
	         ! avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	DEBUG("\n\n\n");
	// remove leaf node
	probe->key = 2;
	CuAssert(tc, "Failed deletion", avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	CuAssertIntEquals(tc, probe->key, dest->key);
	free(dest);
	DEBUG("\n\n\n");
	probe->key = 4;
	CuAssert(tc, "Failed deletion", avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	CuAssertIntEquals(tc, probe->key, dest->key);
	free(dest);
	DEBUG("\n\n\n");
	probe->key = 6;
	CuAssert(tc, "Failed deletion", avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	CuAssertIntEquals(tc, probe->key, dest->key);
	free(dest);
	DEBUG("\n\n\n");
	probe->key = 13;
	CuAssert(tc, "Failed deletion", avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	CuAssertIntEquals(tc, probe->key, dest->key);
	free(dest);
	DEBUG("\n\n\n");
	probe->key = 17;
	CuAssert(tc, "Failed deletion", avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	CuAssertIntEquals(tc, probe->key, dest->key);
	free(dest);
	DEBUG("\n\n\n");
	probe->key = 10;
	CuAssert(tc, "Failed deletion", avl_remv(tree, &probe, &dest));
	DEBUG_EXEC(avl_print(tree, stderr, print_obj_t_ref));
	CuAssertIntEquals(tc, probe->key, dest->key);
	free(dest);
	DEBUG("\n\n\n");
	DESTROY(tree, finaliser_cons(FNR(avl), finaliser_new_ptr()));
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}



void test_avl_get_iter(CuTest *tc)
{
	memdbg_reset();
	int half_univ = 10;
	// typed primitive values
	avl *tree = avl_new(sizeof(int), cmp_int);
	for (int i = 0; i < half_univ; i++) {
		int val = half_univ + ((i % 2) ? i : -i);
		//DEBUG("Insert %d\n", val);
		CuAssert(tc, "Failed AVL push", avl_ins_int(tree, val));
		//DEBUG_EXECUTE(avl_print(tree, stderr, print_int));
	}
	DEBUG_EXEC(avl_print(tree, stderr, print_int));
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
	DESTROY_FLAT(tree, avl);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}



CuSuite *avl_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_avl_ins);
	SUITE_ADD_TEST(suite, test_avl_get);
	SUITE_ADD_TEST(suite, test_avl_remv);
	SUITE_ADD_TEST(suite, test_avl_get_iter);
	return suite;
}