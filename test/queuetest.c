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

#include "queue.h"


void test_queue_push_pop(CuTest *tc)
{
    size_t len = 100;
    queue *q = queue_new(sizeof(int));
    CuAssertSizeTEquals(tc, 0, queue_len(q));
    CuAssertTrue(tc, queue_empty(q));
    
    for (size_t rounds = 0; rounds<10; rounds++) {
        for (size_t k=1; k<=10; k++) {
            for (size_t i=1; i<=(len/3); i++) {
                int d = (int)(k*i);
                //printf("pushing %d\n", d);
                queue_push(q, &d);
                CuAssertSizeTEquals(tc, ((k-1)*(len/3)+i), queue_len(q));
            }
        }
    
        for (size_t k=1; k<=10; k++) {
            for (size_t i=1; i<=(len/3); i++) {
                int d;
                queue_pop(q, &d);
                //printf("popping %d\n", d);
                CuAssertIntEquals(tc, (size_t)(k*i), d);
            }
        }
    
        CuAssertSizeTEquals(tc, 0, queue_len(q));
        CuAssertTrue(tc, queue_empty(q));
    
    }
}


void test_queue_push_pop_int(CuTest *tc)
{
    size_t len = 100;
    queue *q = queue_new(sizeof(int));
    CuAssertSizeTEquals(tc, 0, queue_len(q));
    
    for (size_t rounds = 0; rounds<1; rounds++) {
        for (size_t k=1; k<=10; k++) {
            for (size_t i=1; i<=(len/3); i++) {
                int d = (int)(k*i);
                //printf("pushing int %d\n", d);
                queue_push_int(q, d);
                CuAssertSizeTEquals(tc, ((k-1)*(len/3)+i), queue_len(q));
            }
        }
    
        for (size_t k=1; k<=10; k++) {
            for (size_t i=1; i<=(len/3); i++) {
                int d = queue_pop_int(q);
                //printf("popping int %d\n", d);
                CuAssertIntEquals(tc, (size_t)(k*i), d);
            }
        }
    
        CuAssertSizeTEquals(tc, 0, queue_len(q));
    
    }
}



CuSuite *queue_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_queue_push_pop);
    SUITE_ADD_TEST(suite, test_queue_push_pop_int);
    return suite;
}