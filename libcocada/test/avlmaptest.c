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

#include "avlmap.h"
#include "errlog.h"
#include "memdbg.h"
#include "new.h"
#include "order.h"

typedef struct {
	int key;
} testkey_t;

typedef struct {
	double val;
} testval_t;

int cmp_testkey_t(const void *left, const void *right)
{
	return cmp_int(left, right);
}

int cmp_testkey_ptr_t(const void *left, const void *right)
{
	return cmp_int(*(const void **)left, *(const void **)right);
}


void avlmap_test_ins(CuTest *tc)
{
	memdbg_reset();

	// FLAT MAP

	avlmap *map = avlmap_new(sizeof(testkey_t), sizeof(testval_t), cmp_testkey_t);
	size_t n = 100;

	for (int i = 0, step = 10; i < step * n; i += step) {
		testkey_t key = {.key = i};
		testval_t val = {.val = (double)i};
		CuAssert(tc, "Should not contain key.", !avlmap_contains(map, &key));
		avlmap_ins(map, &key, &val);
		CuAssert(tc, "Should contain key.", avlmap_contains(map, &key));
		testval_t *v = avlmap_get(map, &key);
		CuAssertDblEquals(tc, val.val, v->val, 0.1);
	}

	avlmap_iter *amit = avlmap_get_iter(map, IN_ORDER);
	FOREACH_IN_ITER(entry, avlmap_entry, avlmap_iter_as_iter(amit)) {
		testkey_t *k = (testkey_t *)entry->key;
		testval_t *v = (testval_t *)entry->val;
		DEBUG("Key={%d} val={%lf}\n", k->key, v->val);
	}
	avlmap_iter_free(amit);

	DESTROY_FLAT(map, avlmap);

	// TWO-LEVEL MAP

	map = avlmap_new(sizeof(testkey_t *), sizeof(testval_t *), cmp_testkey_ptr_t);

	for (int i = 0, step = 10; i < step * n; i += step) {
		testkey_t *key = NEW(testkey_t);
		key->key = i;
		testval_t *val = NEW(testval_t);
		val->val = (double)i;
		CuAssert(tc, "Should not contain key.", !avlmap_contains(map, &key));
		avlmap_ins(map, &key, &val);
		CuAssert(tc, "Should contain key.", avlmap_contains(map, &key));
		testval_t **v = (testval_t **)avlmap_get(map, &key);
		CuAssertDblEquals(tc, val->val, (*v)->val, 0.1);
	}

	amit = avlmap_get_iter(map, IN_ORDER);
	FOREACH_IN_ITER(entry, avlmap_entry, avlmap_iter_as_iter(amit)) {
		testkey_t **k = entry->key;
		testval_t **v = entry->val;
		DEBUG("Key={%d} val={%lf}\n", (*k)->key, (*v)->val);
	}
	avlmap_iter_free(amit);

	DESTROY(map, finaliser_cons(finaliser_cons(FNR(avlmap), finaliser_new_ptr()),
	                            finaliser_new_ptr()));


	if (!memdbg_is_empty()) {
		memdbg_print_stats(stdout, true);
		CuAssert(tc, "Memory leak!", memdbg_is_empty());
	}

}


CuSuite *avlmap_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, avlmap_test_ins);

	return suite;
}