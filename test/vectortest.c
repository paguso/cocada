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
#include "randutil.h"
#include "vec.h"


void test_vec_new(CuTest *tc)
{
    vec *v = vec_new_with_capacity(sizeof(int), 10);
    CuAssertSizeTEquals(tc, 0, vec_len(v));
    vec_free(v, true);
}


void test_vec_app(CuTest *tc)
{
    size_t len = 100;
    vec *v = vec_new(sizeof(short));
    CuAssertSizeTEquals(tc, 0, vec_len(v));

    for (size_t i =0; i<len; i++) {
        short d = i;
        vec_app(v, &d);
        CuAssertSizeTEquals(tc, i+1, vec_len(v));
    }

    for (size_t i =0; i<len; i++) {
        const short *d;
        d = vec_get(v, i);
        //printf("get da[%zu]=%d\n",i,*d);
        CuAssertIntEquals(tc, (int)i, *d);
    }

    vec_free(v, false);
}


void test_vec_get_cpy(CuTest *tc)
{
    size_t len = 100;
    vec *v = vec_new(sizeof(double));
    CuAssertSizeTEquals(tc, 0, vec_len(v));

    for (size_t i =0; i<len; i++) {
        double d = i;
        vec_app(v, &d);
        CuAssertSizeTEquals(tc, i+1, vec_len(v));
    }

    for (size_t i =0; i<len; i++) {
        double d;
        vec_get_cpy(v, i, &d);
        //printf("get da[%zu]=%f\n",i,d);
        CuAssertDblEquals(tc, (double)i, d, 0.2);
    }

    vec_free(v, false);
}



void test_vec_set(CuTest *tc)
{
    size_t len = 100;
    vec *v = vec_new(sizeof(int));
    CuAssertSizeTEquals(tc, 0, vec_len(v));

    for (int i =0; i<len; i++) {
        vec_app(v, &i);
        CuAssertSizeTEquals(tc, i+1, vec_len(v));
    }

    for (int i=1; i<len; i+=2) {
        int d = -i;
        vec_set(v, i, &d);
    }
    
    for (int i =0; i<len; i++) {
        const int *d ;
        d = vec_get(v, i);
        //printf("get da[%zu]=%d\n",i,*d);
        CuAssertIntEquals(tc, (i%2)?-i:i, *d);
    }

    vec_free(v, false);
}


void test_vec_ins(CuTest *tc)
{
    size_t len = 110;
    vec *v = vec_new(sizeof(double*));
    CuAssertSizeTEquals(tc, 0, vec_len(v));

    for (size_t i=1; i<len; i+=2) {
        double d = i;
        vec_app(v, &d);
    }
    for (size_t i=0; i<len; i+=2) {
        double d = i;
        vec_ins(v, i, &d);
    }
    for (size_t i =0; i<len; i++) {
        const double *d;
        d = vec_get(v, i);
        //printf("get da[%zu]=%f\n",i,*d);
        CuAssertDblEquals(tc, (double)i, *d, 0.2);
    }
    vec_free(v, false);
}



void test_vec_del(CuTest *tc)
{
    size_t len = 100;
    vec *v = vec_new(sizeof(double*));
    CuAssertSizeTEquals(tc, 0, vec_len(v));

    for (size_t i=0; i<len; i++) {
        double d = i;
        vec_app(v, &d);
    }
    for (size_t i=0; i<len/2; i++) {
        double d; 
        vec_del(v, i, &d);
        //printf(">deleted da[%zu]=%f\n",i,d);
        CuAssertDblEquals(tc, (double)2*i, d, 0.2);
    }
    for (size_t i =0; i<vec_len(v); i++) {
        const double *d;
        d = vec_get(v, i);
        //printf("get da[%zu]=%f\n",i,*d);
        CuAssertDblEquals(tc, (double)(2*i)+1.0, *d, 0.2);
    }
    vec_free(v, false);
}


void test_vec_swap(CuTest *tc)
{
    size_t len = 100;
    vec *v = vec_new(sizeof(double*));
    CuAssertSizeTEquals(tc, 0, vec_len(v));

    for (size_t i=0; i<len; i++) {
        double d = i;
        vec_app(v, &d);
    }
    for (size_t i=0; i<len/2; i++) {
        vec_swap(v, i, len-1-i);
    }
    for (size_t i =0; i<vec_len(v); i++) {
        const double *d;
        d = vec_get(v, i);
        //printf("get da[%zu]=%f\n",i,*d);
        CuAssertDblEquals(tc, (double)(len-1-i), *d, 0.2);
    }
    vec_free(v, false);
}



typedef struct _triple {
    uint values[3];
} triple;

static size_t triple_key(const void *tp, size_t d)
{
    return ((triple *)tp)->values[2-d%3];
}

static int triple_cmp(triple *t1, triple *t2)
{
    if (t1->values[0]<t2->values[0]) return -1;
    else if (t1->values[0]>t2->values[0]) return +1;
    else if (t1->values[1]<t2->values[1]) return -1;
    else if (t1->values[1]>t2->values[1]) return +1;
    else if (t1->values[2]<t2->values[2]) return -1;
    else if (t1->values[2]>t2->values[2]) return +1;
    else return 0;
}


void test_vec_radixsort(CuTest *tc)
{
    size_t max_key = 3;
    vec *v;
    v = vec_new(sizeof(triple));
    for (size_t d1=0; d1<max_key; d1++) {
        for (size_t d0=0; d0<max_key; d0++) {
            for (size_t d2=0; d2<max_key; d2++) {
                triple t;
                t.values[0] = d0;
                t.values[1] = d1;
                t.values[2] = d2;
                vec_app(v, &t);
            }
        }
    }
    
    /*
    printf("Before sort:\n");
    for (size_t i=0; i<vec_len(v); i++)
    {
        triple *t = (triple *)vec_get(v, i);
        printf("v[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
    }
    */

    vec_radixsort(v, &triple_key, 3, max_key);
    /*
    printf("After sort:\n");
    for (size_t i=0; i<vec_len(da); i++)
    {
        triple *t = (triple *)vec_get(da, i);
        printf("da[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
    }
    */
    for (size_t i=0; i<vec_len(v)-1; i++)
    {
        triple *p = (triple *)vec_get(v, i);
        triple *q = (triple *)vec_get(v, i+1);
        CuAssertIntEquals(tc, -1, triple_cmp(p, q));
    }
}


void test_vec_qsort(CuTest *tc)
{
    size_t max_key = 10000;
    triple *arr = NEW_ARRAY(triple, max_key);
    for (size_t i=0; i<max_key; i++) {
                triple t;
                t.values[0] = i;
                t.values[1] = i;
                t.values[2] = i;
                arr[i] = t;
    }
    shuffle_arr(arr, max_key, sizeof(triple));
    vec *v = vec_new(sizeof(triple));
    for (size_t i=0; i<max_key; i++){
        vec_app(v, &arr[i]); 
    }
    FREE(arr);

    /*
    printf("Before sort:\n");
    for (size_t i=0; i<vec_len(v); i++)
    {
        triple *t = (triple *)vec_get(v, i);
        printf("v[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
    }
    */
    vec_qsort(v, triple_cmp);
    /*
    printf("After sort:\n");
    for (size_t i=0; i<vec_len(v); i++)
    {
        triple *t = (triple *)vec_get(v, i);
        printf("v[%zu] = (%u, %u, %u)\n", i, t->values[0], t->values[1], t->values[2]);
    }
    */
    for (size_t i=0; i<vec_len(v)-1; i++)
    {
        triple *p = (triple *)vec_get(v, i);
        triple *q = (triple *)vec_get(v, i+1);
        CuAssertIntEquals(tc, -1, triple_cmp(p, q));
    }
}

typedef struct _vecobj {
    int i;
    double d;
} vobj;

void test_vec_free(CuTest *tc) 
{
    size_t n = 10;
    vec *v = vec_new(sizeof(vec *));
    for (size_t i=0; i<n; i++) {
        vec *c = vec_new(sizeof(vobj *));
        for (size_t j=0; j<i; j++) {
            vobj *e = NEW(vobj);
            e->i = (int)i;
            e->d = (double)i;
            vec_app(c, &e); 
        }
        CuAssertSizeTEquals(tc, i, vec_len(c));
        vec_app(v, &c);
    }
    CuAssertSizeTEquals(tc, n, vec_len(v));

    DESTROY_AND_CONSUME(v, dstr_cons(vec_dstr(), dstr_cons(vec_dstr(), raw_dstr())));

}

CuSuite *vec_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_vec_new);
    SUITE_ADD_TEST(suite, test_vec_app);
    SUITE_ADD_TEST(suite, test_vec_get_cpy);
    SUITE_ADD_TEST(suite, test_vec_set);
    SUITE_ADD_TEST(suite, test_vec_ins);
    SUITE_ADD_TEST(suite, test_vec_del);
    SUITE_ADD_TEST(suite, test_vec_swap);
    SUITE_ADD_TEST(suite, test_vec_radixsort);
    SUITE_ADD_TEST(suite, test_vec_qsort);
    SUITE_ADD_TEST(suite, test_vec_free);
    return suite;
}