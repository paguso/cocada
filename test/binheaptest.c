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
#include <string.h>

#include "CuTest.h"

#include "arrayutil.h"
#include "binheap.h"



int cmp_dbl(const void *p1, const void *p2) 
{
    double d1 = *((double *)p1);
    double d2 = *((double *)p2);
    if ( d1 < d2 ) 
        return -1;
    else if ( d1 == d2 ) 
        return 0;
    else
        return +1;
}


/*
 * Tests storing pointers to double
 */
void test_binheap_push_pop(CuTest *tc)
{
    size_t j, len = 10;
    binheap *minheap = binheap_new(&cmp_dbl, sizeof(double), MIN_HEAP);
    binheap *maxheap = binheap_new(&cmp_dbl, sizeof(double), MAX_HEAP);
    CuAssertSizeTEquals(tc, 0, binheap_size(minheap));
    CuAssertSizeTEquals(tc, 0, binheap_size(maxheap));
    
    double *rv;
    rv = NEW_ARRAY(double, len);
    for (size_t i=0; i<len; rv[i]=(double)i++);
    for (size_t i=len-1; i>0; i--) {
        j = rand()%(i+1);
        double tmp = rv[j];
        rv[j] = rv[i];
        rv[i] = tmp;
    }

    //for (size_t i =0; i<len; i++) {
    //    printf("rv[#%zu] = %f\n",i,rv[i]);
    //}
    
    for (size_t i =0; i<len; i++) {
        double d = rv[i];
        binheap_push(minheap, &d);
        binheap_push(maxheap, &d);
        CuAssertSizeTEquals(tc, i+1, binheap_size(minheap));
        CuAssertSizeTEquals(tc, i+1, binheap_size(maxheap));
    }

    
    for (size_t i =0; i<len; i++) {
        double d;
        binheap_pop(minheap, &d);
        //printf("minheap #%zu = %f\n",i, d);
        CuAssertSizeTEquals(tc, len-i-1, binheap_size(minheap));
        CuAssertDblEquals(tc, (double)i, d, 0.1);
    }

    for (size_t i =0; i<len; i++) {
        double d;
        binheap_pop(maxheap, &d);
        //printf("maxheap #%zu = %f\n",i,d);
        CuAssertSizeTEquals(tc, len-i-1, binheap_size(maxheap));
        CuAssertDblEquals(tc, (double)(len-i-1), d, 0.1);
    }

    binheap_free(minheap, true);
    binheap_free(maxheap, true);
}



int cmp_int(const void *p1, const void *p2) {
    int i1, i2;
    i1 = *(int *)p1;
    i2 = *(int *)p2;
    if (i1<i2) 
        return -1; 
    else if (i1==i2) 
        return 0; 
    else 
        return +1; 
}


void test_binheap_push_pop_int(CuTest *tc)
{
    size_t j, len = 10;
    binheap *minheap = binheap_new(&cmp_int, sizeof(int), MIN_HEAP);
    binheap *maxheap = binheap_new(&cmp_int, sizeof(int), MAX_HEAP);
    CuAssertSizeTEquals(tc, 0, binheap_size(minheap));
    CuAssertSizeTEquals(tc, 0, binheap_size(maxheap));
    
    int *rv;
    rv = NEW_ARRAY(int, len);
    for (size_t i=0; i<len; rv[i]=(int)i++);
    for (size_t i=len-1; i>0; i--) {
        j = rand()%(i+1);
        int tmp = rv[j];
        rv[j] = rv[i];
        rv[i] = tmp;
    }

    //for (size_t i =0; i<len; i++) {
    //    printf("rv[#%zu] = %d\n",i,rv[i]);
    //}
    
    for (size_t i =0; i<len; i++) {
        binheap_push_int(minheap, rv[i]);
        binheap_push_int(maxheap, rv[i]);
        CuAssertSizeTEquals(tc, i+1, binheap_size(minheap));
        CuAssertSizeTEquals(tc, i+1, binheap_size(maxheap));
    }

    
    for (size_t i =0; i<len; i++) {
        int d;
        d = binheap_pop_int(minheap);
        //printf("minheap #%zu = %d\n",i,d);
        CuAssertSizeTEquals(tc, len-i-1, binheap_size(minheap));
        CuAssertIntEquals(tc, (int)i, d);
    }

    for (size_t i =0; i<len; i++) {
        int d;
        d = binheap_pop_int(maxheap);
        //printf("maxheap #%zu = %d\n",i,d);
        CuAssertSizeTEquals(tc, len-i-1, binheap_size(maxheap));
        CuAssertIntEquals(tc, (int)(len-i-1), d);
    }

    binheap_free(minheap, false);
    binheap_free(maxheap, false);
}



CuSuite *binheap_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_binheap_push_pop);
    SUITE_ADD_TEST(suite, test_binheap_push_pop_int);
    return suite;
}