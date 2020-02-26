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

#include "deque.h"

typedef struct _dequeobj {
    int i;
    double f;
    char c;
} dequeobj;


void test_deque_push_pop(CuTest *tc)
{
    size_t n = 10000;
    deque *q = deque_new(sizeof(dequeobj));
    CuAssertSizeTEquals(tc, 0, deque_len(q));
    CuAssertTrue(tc, deque_empty(q));

    for (int i=0; i<n; i++) {
        dequeobj o = {i, (double)i, (char)i};
        deque_push_back(q, &o); 
    }
    CuAssertSizeTEquals(tc, n, deque_len(q));       

    dequeobj o;
    for (int i=0; i<n/2; i++) {
        deque_pop_front(q, &o);
        CuAssertIntEquals(tc, i, o.i);
        CuAssertDblEquals(tc, (double)i, o.f, 0.1);
        CuAssertCharEquals(tc, (char)i, o.c);
        deque_push_back(q, &o);
    }
    CuAssertSizeTEquals(tc, n, deque_len(q));       

    for (int i=n/2-1; i>=0; i--) {
        deque_pop_back(q, &o);
        CuAssertIntEquals(tc, i, o.i);
        CuAssertDblEquals(tc, (double)(i), o.f, 0.1);
        CuAssertCharEquals(tc, (char)(i), o.c);
    }
    CuAssertSizeTEquals(tc, n-(n/2), deque_len(q));       
    
    for (int i=0; i<n/4; i++) {
        if (i%2==0) { 
            deque_pop_back(q, &o);
            CuAssertIntEquals(tc, n-1-(i/2), o.i);
            CuAssertDblEquals(tc, (double)(n-1-(i/2)), o.f, 0.1);
            CuAssertCharEquals(tc, (char)(n-1-(i/2)), o.c);
        }
        else {
            deque_pop_front(q, &o);
            CuAssertIntEquals(tc, (n/2)+((i-1)/2), o.i);
            CuAssertDblEquals(tc, (double)((n/2)+((i-1)/2)), o.f, 0.1);
            CuAssertCharEquals(tc, (char)((n/2)+((i-1)/2)), o.c);
        }
    }
    CuAssertSizeTEquals(tc, n-(n/2)-(n/4), deque_len(q));       
}


void test_deque_push_pop_int(CuTest *tc)
{
    size_t n = 10000;
    deque *q = deque_new_int();
    CuAssertSizeTEquals(tc, 0, deque_len(q));
    CuAssertTrue(tc, deque_empty(q));

    for (int i=0; i<n; i++) {
        deque_push_back_int(q, i); 
    }
    CuAssertSizeTEquals(tc, n, deque_len(q));       

    int o;
    for (int i=0; i<n/2; i++) {
        o = deque_pop_front_int(q);
        CuAssertIntEquals(tc, i, o);
        deque_push_back_int(q, o);
    }
    CuAssertSizeTEquals(tc, n, deque_len(q));       

    for (int i=n/2-1; i>=0; i--) {
        o = deque_pop_back_int(q);
        CuAssertIntEquals(tc, i, o);
    }
    CuAssertSizeTEquals(tc, n-(n/2), deque_len(q));       
    
    for (int i=0; i<n/4; i++) {
        if (i%2==0) { 
            o = deque_pop_back_int(q);
            CuAssertIntEquals(tc, n-1-(i/2), o);
        }
        else {
            o = deque_pop_front_int(q);
            CuAssertIntEquals(tc, (n/2)+((i-1)/2), o);
        }
    }
    CuAssertSizeTEquals(tc, n-(n/2)-(n/4), deque_len(q));       
}



CuSuite *deque_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_deque_push_pop);
    SUITE_ADD_TEST(suite, test_deque_push_pop_int);
    return suite;
}