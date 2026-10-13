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
#include <string.h>

#include "CuTest.h"


CuSuite *arrays_get_test_suite();
CuSuite *avl_get_test_suite();
CuSuite *avlmap_get_test_suite();
CuSuite *binheap_get_test_suite();
CuSuite *bitarray_get_test_suite();
CuSuite *bitbyte_get_test_suite();
CuSuite *bitvec_get_test_suite();
CuSuite *bytearray_get_test_suite();
CuSuite *cli_get_test_suite();
CuSuite *csrsbitarr_get_test_suite();
CuSuite *cstrutil_get_test_suite();
CuSuite *deque_get_test_suite();
CuSuite *strbuf_get_test_suite();
CuSuite *strview_get_test_suite();
CuSuite *hashmap_get_test_suite();
CuSuite *hashset_get_test_suite();
//CuSuite *kwayrng_get_test_suite();
CuSuite *mathutil_get_test_suite();
CuSuite *minqueue_get_test_suite();
CuSuite *quadtree_get_test_suite();
CuSuite *queue_get_test_suite();
CuSuite *randutil_get_test_suite();
CuSuite *range_get_test_suite();
CuSuite *segtree_get_test_suite();
CuSuite *serialise_get_test_suite();
CuSuite *sort_get_test_suite();
CuSuite *stack_get_test_suite();
CuSuite *strfileread_get_test_suite();
//CuSuite *tvec_get_test_suite();
//CuSuite *twuhash_get_test_suite();
CuSuite *vec_get_test_suite();
CuSuite *vebset_get_test_suite();

CuSuite* create_test_suite()
{
	CuSuite *suite = CuSuiteNew("all");
	CuSuiteAddSuite(suite, arrays_get_test_suite());
	CuSuiteAddSuite(suite, avl_get_test_suite());
	CuSuiteAddSuite(suite, avlmap_get_test_suite());
	CuSuiteAddSuite(suite, binheap_get_test_suite());
	CuSuiteAddSuite(suite, bitarray_get_test_suite());
	CuSuiteAddSuite(suite, bitbyte_get_test_suite());
	CuSuiteAddSuite(suite, bitvec_get_test_suite());
	CuSuiteAddSuite(suite, bytearray_get_test_suite());
	CuSuiteAddSuite(suite, cstrutil_get_test_suite());
	CuSuiteAddSuite(suite, deque_get_test_suite());
	CuSuiteAddSuite(suite, hashmap_get_test_suite());
	CuSuiteAddSuite(suite, hashset_get_test_suite());
	CuSuiteAddSuite(suite, mathutil_get_test_suite());
	CuSuiteAddSuite(suite, minqueue_get_test_suite());
	CuSuiteAddSuite(suite, randutil_get_test_suite());
	CuSuiteAddSuite(suite, range_get_test_suite());
	CuSuiteAddSuite(suite, serialise_get_test_suite());
	CuSuiteAddSuite(suite, segtree_get_test_suite());
	CuSuiteAddSuite(suite, sort_get_test_suite());
	CuSuiteAddSuite(suite, stack_get_test_suite());
	CuSuiteAddSuite(suite, strbuf_get_test_suite());
	CuSuiteAddSuite(suite, strview_get_test_suite());
	CuSuiteAddSuite(suite, strfileread_get_test_suite());
	CuSuiteAddSuite(suite, vebset_get_test_suite());
	CuSuiteAddSuite(suite, vec_get_test_suite());
	return suite;
}

int main(int argc, char **argv)
{
	CuSuite *rootSuite = create_test_suite();
	bool allOk = true;
	if (argc > 1 && !strcmp(argv[1], "-l")) {
		fprintf(stdout, "Available tests\n");
		CuSuitePrint(rootSuite);
		return 0;
	}
	if (argc == 1) {
		allOk = allOk && CuSuiteRun(rootSuite);
	} else {
		for (int i = 1; i < argc; i++) {
			fprintf(stdout, "Running suite %s tests...\n", argv[i]);
			allOk = allOk && CuSuiteRunSuite(rootSuite, argv[i]);
		}
	}
	CuSuitePrintResults(rootSuite, stdout);
	CuSuiteDelete(rootSuite);
	return allOk?0:1;
}

