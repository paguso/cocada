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
#include "cstrutil.h"
#include "new.h"
#include "trait.h"
#include "serialise.h"






void test_serialise_prim(CuTest *tc)
{
	int *i = NEW(int);
	*i = 10;
	FILE *stream = fopen("serialised_int.obj", "wb");
	serialise(i, get_som_int(), stream);
	fclose(stream);

	stream = fopen("serialised_int.obj", "rb");
	int *j = (int *)deserialise(get_som_int(), stream);
	fclose(stream);

	CuAssertIntEquals(tc, *i, *j);
	FREE(i);
	FREE(j);
}


void test_serialise_arr(CuTest *tc)
{
	int n = 10;
	short *arr = arr_short_new(10);
	for (int i=0; i < n; i++) {
		arr[i] = i;
	}
	FILE *stream = fopen("serialised_arr.obj", "wb");
	som *model = som_cons(som_arr_new(), 0, get_som_short());
	serialise(arr, model, stream);
	fclose(stream);

	stream = fopen("serialised_arr.obj", "rb");
	short *arr_cpy = (short *)deserialise(model, stream);
	fclose(stream);

	CuAssertSizeTEquals(tc, arr_short_len(arr), arr_short_len(arr_cpy));
	for (size_t i=0; i<arr_short_len(arr); i++) {
		CuAssertIntEquals(tc, arr[i], arr_cpy[i]);
	}
	arr_free(arr);
	arr_free(arr_cpy);
}


typedef struct {
	char c;
	bool b;
} test_sub_struct;

STR_SOM_INFO(test_sub_struct)

static som *test_sub_struct_som;

som *get_test_sub_struct_som()
{
	if (test_sub_struct_som == NULL) {
		test_sub_struct_som
		    = som_cons(
		          som_cons(
		              som_struct_new(sizeof(test_sub_struct), get_test_sub_struct_som),
		              STR_OFFSET(test_sub_struct, c),
		              get_som_char()
		          ),
		          STR_OFFSET(test_sub_struct, b),
		          get_som_bool()
		      );
	}
	return test_sub_struct_som;
}


typedef struct {
	int i;
	test_sub_struct s;
	float f;
} test_struct;

STR_SOM_INFO(test_struct)

static som *test_struct_som;

som *get_test_struct_som()
{
	if (test_struct_som == NULL) {
		test_struct_som =
		    som_cons(
		        som_struct_new(sizeof(test_struct), get_test_struct_som),
		        STR_OFFSET(test_struct, i),
		        get_som_int()
		    );
		test_struct_som =
		    som_cons(
		        test_struct_som,
		        STR_OFFSET(test_struct, s),
		        som_proxy_new(get_test_sub_struct_som)
		    );
		test_struct_som =
		    som_cons(
		        test_struct_som,
		        STR_OFFSET(test_struct, f),
		        get_som_float()
		    );
	}
	return test_struct_som;
}


void test_serialise_struct(CuTest *tc)
{
	test_struct str = {.i = 1, .s = {.b = true, .c='a'}, .f = 3.14};

	FILE *stream = fopen("serialised_str.obj", "wb");
	serialise(&str, get_test_struct_som(), stream);
	fclose(stream);

	stream = fopen("serialised_str.obj", "rb");
	test_struct *str_cpy = (test_struct *)deserialise(get_test_struct_som(),
	                       stream);
	fclose(stream);

	CuAssertIntEquals(tc, str.i, str_cpy->i);
	CuAssertDblEquals(tc, str.f, str_cpy->f, 0);
	CuAssertCharEquals(tc, str.s.c, str_cpy->s.c);
	CuAssert(tc, "", str.s.b == str_cpy->s.b);

	FREE(str_cpy);
}



typedef struct _node {
	int val;
	char *str;
	int *arr;
	struct _node *next;
} node;

static som *node_som = NULL;
STR_SOM_INFO(node)

som *get_node_som ()
{
	if (node_som==NULL) {
		node_som = som_struct_new(sizeof(node), get_node_som);
		som_cons(node_som, STR_OFFSET(node, val), get_som_int());
		som_cons(node_som, STR_OFFSET(node, str),
		         som_cons(som_ptr_new(), 0, get_som_cstr()));
		som_cons(node_som, STR_OFFSET(node, arr),
		         som_cons(som_ptr_new(), 0,
		                  som_cons(som_arr_new(), 0, get_som_int())));
		som_cons(node_som, STR_OFFSET(node, next),
		         som_cons(som_ptr_new(), 0,
		                  som_proxy_new(get_node_som)));
	}
	return node_som;
}


void test_serialise_list(CuTest *tc)
{
	node *head = NULL;
	int n = 3;
	node **cur = &head;
	node *tail;
	for (int i=1; i<=n; i++) {
		tail = NEW(node);
		tail->val = i;
		tail->str = cstr_new(i);
		for (int j = 0; j < i; tail->str[j++] = '0'+i);   
		tail->arr = arr_int_new(i);
		FILL_ARR(tail->arr, 0, i, i);
		tail->next = NULL;
		(*cur) = tail;
		cur = &tail->next;
	}

	FILE *stream = fopen("serialised_node.out", "wb");
	som *model = get_node_som();
	serialise(head, model, stream);
	fclose(stream);

	stream = fopen("serialised_node.out", "rb");
	node *head_cpy = deserialise(model, stream);
	fclose(stream);

	for ( node *cur = head, *cur_cpy = head_cpy; cur != NULL && cur_cpy; 
		cur=cur->next, cur_cpy=cur_cpy->next ) 
	{
		CuAssertIntEquals(tc, cur->val, cur_cpy->val);
		CuAssertStrEquals(tc, cur->str, cur_cpy->str);
		CuAssertSizeTEquals(tc, arr_int_len(cur->arr), arr_int_len(cur_cpy->arr));
		for (size_t i = 0; i < arr_int_len(cur->arr); i++) {
			CuAssertIntEquals(tc, cur->arr[i], cur_cpy->arr[i]);
		}
	}

}




CuSuite *serialise_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_serialise_prim);
	SUITE_ADD_TEST(suite, test_serialise_arr);
	SUITE_ADD_TEST(suite, test_serialise_struct);
	SUITE_ADD_TEST(suite, test_serialise_list);
	return suite;
}
