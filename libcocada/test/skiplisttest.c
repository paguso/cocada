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
#include "errlog.h"
#include "memdbg.h"
#include "new.h"
#include "order.h"
#include "skiplist.h"


int cmp_ptr_to_int(const void *left, const void *right)
{
	return cmp_int(*(int **)left, *(int **)right);
}

void skiplist_ins_test(CuTest *tc)
{
	memdbg_reset();
	//insert owned objects
	skiplist *sl = skiplist_new(sizeof(int), cmp_int);
	skiplist_iter *it;
	for (int key = 10; key < 100; key +=10) {
		skiplist_ins(sl, &key);
	}
	for (int key = 5; key < 100; key +=10) {
		skiplist_ins(sl, &key);
	}
	size_t len = skiplist_len(sl);
	for (int key = 5; key < 100; key +=10) {
		// duplicate insertions should do nothing
		skiplist_ins(sl, &key);
	}
	CuAssertSizeTEquals(tc, len, skiplist_len(sl));
	it = skiplist_get_iter(sl);
	FOREACH_IN_ITER(key, int *, skiplist_iter_as_iter(it)) {
		DEBUG("key = %d\n", *key);
	}
	FREE(it);
	DESTROY(sl, finaliser_cons(FNR(skiplist), finaliser_new_empty()));
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak!", memdbg_is_empty());
	//insert non-owned objects
	sl = skiplist_new(sizeof(int *), cmp_ptr_to_int);
	int keys[10] = {8, 9, 2, 3, 1, 4, 0, 7, 5, 6 };
	for (int *key = keys; key < keys + 10; key++) {
		skiplist_ins(sl, &key);
	}
	len = skiplist_len(sl);
	for (int *key = keys; key < keys + 10; key++) {
		// duplicate insertions should do nothing
		skiplist_ins(sl, &key);
	}
	CuAssertSizeTEquals(tc, len, skiplist_len(sl));
	it = skiplist_get_iter(sl);
	FOREACH_IN_ITER(key, int **, skiplist_iter_as_iter(it)) {
		DEBUG("key = %d\n", **key);
	}
	FREE(it);
	DESTROY_FLAT(sl, skiplist);
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak!", memdbg_is_empty());
}

void skiplist_del_test(CuTest *tc)
{
	memdbg_reset();
	//insert owned objects
	skiplist *sl = skiplist_new(sizeof(int), cmp_int);
	skiplist_iter *it;
	int del_key;
	for (int key = 5; key < 100; key +=5) {
		skiplist_ins(sl, &key);
	}
	for (int key = 5; key < 100; key +=10) {
		bool res = skiplist_remv(sl, &key, &del_key);
		CuAssertIntEquals(tc, del_key, key);
		CuAssert(tc, "SL Deletion failed.", res);
	}
	for (int key = 5; key < 100; key +=10) {
		bool res = skiplist_del(sl, &key);
		CuAssert(tc, "SL Deletion succeeded, but should have failed.", !res);
	}
	it = skiplist_get_iter(sl);
	FOREACH_IN_ITER(key, int *, skiplist_iter_as_iter(it)) {
		DEBUG("key = %d\n", *key);
	}
	FREE(it);
	DESTROY(sl, finaliser_cons(FNR(skiplist), finaliser_new_empty()));
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak!", memdbg_is_empty());

	//insert non-owned objects
	sl = skiplist_new(sizeof(int *), cmp_ptr_to_int);
	int keys[10] = {8, 9, 2, 3, 1, 4, 0, 7, 5, 6 };
	for (int *key = keys; key < keys + 10; key++) {
		skiplist_ins(sl, &key);
	}
	for (int *key = keys; key < keys + 10; key++) {
		bool res = skiplist_del(sl, &key);
		CuAssert(tc, "SL Deletion failed.", res);
	}
	for (int *key = keys; key < keys + 10; key++) {
		bool res = skiplist_del(sl, &key);
		CuAssert(tc, "SL Deletion succeeded, but should have failed.", !res);
	}
	it = skiplist_get_iter(sl);
	FOREACH_IN_ITER(key, int **, skiplist_iter_as_iter(it)) {
		DEBUG("key = %d\n", **key);
	}
	FREE(it);
	DESTROY_FLAT(sl, skiplist);
	memdbg_print_stats(stdout, true);
	CuAssert(tc, "Memory leak!", memdbg_is_empty());
}


CuSuite *skiplist_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, skiplist_ins_test);
	SUITE_ADD_TEST(suite, skiplist_del_test);
	return suite;
}