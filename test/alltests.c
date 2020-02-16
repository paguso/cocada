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

#include "CuTest.h"

CuSuite *alphabet_get_test_suite();
CuSuite *binheap_get_test_suite();
CuSuite *bitsandbytes_get_test_suite();
CuSuite *bitarray_get_test_suite();
CuSuite *bitvector_get_test_suite();
CuSuite *bytearray_get_test_suite();
CuSuite *csrsbitarr_get_test_suite();
CuSuite *csarray_get_test_suite();
CuSuite *cstrutil_get_test_suite();
CuSuite *dbgraph_get_test_suite();
CuSuite *dynstr_get_test_suite();
CuSuite *vec_get_test_suite();
CuSuite *hashset_get_test_suite();
CuSuite *huffcode_get_test_suite();
CuSuite *marshall_get_test_suite();
CuSuite *queue_get_test_suite();
CuSuite *stack_get_test_suite();
CuSuite *strstream_get_test_suite();
CuSuite *vebwtree_get_test_suite();
CuSuite *wtree_get_test_suite();
CuSuite *wavtree_get_test_suite();



void run_all_tests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    
    //CuSuiteAddSuite(suite, alphabet_get_test_suite());
    //CuSuiteAddSuite(suite, binheap_get_test_suite());
    CuSuiteAddSuite(suite, bitsandbytes_get_test_suite());
    //CuSuiteAddSuite(suite, bitarray_get_test_suite());
    //CuSuiteAddSuite(suite, bitvector_get_test_suite());
    //CuSuiteAddSuite(suite, bytearray_get_test_suite());
    //CuSuiteAddSuite(suite, csarray_get_test_suite());
    //CuSuiteAddSuite(suite, csrsbitarr_get_test_suite());
    //CuSuiteAddSuite(suite, cstrutil_get_test_suite());
    //CuSuiteAddSuite(suite, vec_get_test_suite());
    //CuSuiteAddSuite(suite, dynstr_get_test_suite());
    //CuSuiteAddSuite(suite, hashtable_get_test_suite());
    //CuSuiteAddSuite(suite, huffcode_get_test_suite());
    //CuSuiteAddSuite(suite, queue_get_test_suite());
    //CuSuiteAddSuite(suite, stack_get_test_suite());
    //CuSuiteAddSuite(suite, strstream_get_test_suite());
    //CuSuiteAddSuite(suite, xstring_get_test_suite());

            
    //CuSuiteAddSuite(suite, dbgraph_get_test_suite());
    //CuSuiteAddSuite(suite, wavtree_get_test_suite());
    
        
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

void print_count() ;

int main(void) 
{
    run_all_tests();
    return 0;
}