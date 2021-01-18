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

#include "arrays.h"
#include "new.h"
#include "trait.h"
#include "serialise.h"

#define STR_SOM_INFO(STRUCT)\
	static const STRUCT __dummy_##STRUCT;\

#define STR_OFFSET(STRUCT, FIELD)\
	((size_t)(&__dummy_##STRUCT.FIELD) - (size_t)(&(__dummy_##STRUCT)))


typedef struct _node {
	int val;
	int *arr;
	struct _node *next;
} node;

som *_node_som = NULL;
STR_SOM_INFO(node)

som *get_node_som ()
{
	if (_node_som==NULL) {
		_node_som = som_struct_new(sizeof(node), get_node_som);
	}
	som_cons(_node_som, STR_OFFSET(node, val), get_som_int());
	som_cons(_node_som, STR_OFFSET(node, arr),
	         som_cons(som_ptr_new(), 0,
	                  som_cons(som_arr_new(), 0, get_som_int())));
	som_cons(_node_som, STR_OFFSET(node, next),
	         som_cons(som_ptr_new(), 0,
                      som_proxy_new(get_node_som)));
	return _node_som;
}



node *head = NULL;

void test_setup()
{
	int n = 3;
	node **cur = &head;
	node *tail;
	for (int i=1; i<=n; i++) {
		tail = NEW(node);
		tail->val = i;
		tail->arr = arr_int_new(i);
		FILL_ARR(tail->arr, 0, i, i);
		tail->next = NULL;
		(*cur) = tail;
		cur = &tail->next;
	}
}


void test_get_som(CuTest *tc)
{
	test_setup();
	FILE *stream = fopen("serialised_node.out", "wb");
	som *model = get_node_som();
	serialise(head, model, stream);
	fclose(stream);
}


CuSuite *serialise_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_get_som);
	return suite;
}
