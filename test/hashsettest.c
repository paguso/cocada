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
#include <stdint.h>

#include "CuTest.h"

#include "arrayutil.h"
#include "hashset.h"
#include "mathutil.h"

typedef uint32_t uint;

static size_t _key_fn(const void *elt) 
{
    return (size_t)(*((uint *)elt));
}

static bool _equals_fn(const void *elt1, const void *elt2) 
{
    return ((size_t)(*((uint *)elt1))==(size_t)(*((uint *)elt2)));
}

static int _uint_cmp_fn(const void *elt1, const void *elt2) 
{
    return ((*((uint *)elt1))-(*((uint *)elt2)));
}

void test_hashset_indel(CuTest *tc)
{
    size_t n=105;
    hashset *set;
    uint elt, *elts;
    hashset_iter *iterator;
    set = hashset_new(sizeof(uint), &_key_fn, &_equals_fn);
    
    CuAssertSizeTEquals(tc, 0, hashset_size(set));
    //CuAssertTrue(tc, !hashset_contains(set, NULL));

    // insert originals
    for (uint i=0; i<n; i++) {
        CuAssertSizeTEquals(tc, i, hashset_size(set));
        elt = (uint)(10*i);
        CuAssertTrue(tc, !hashset_contains(set, &elt));
        hashset_add(set, &elt);
        CuAssertTrue(tc, hashset_contains(set, &elt));
    }

    // insert duplicates
    for (uint i=0; i<n; i++) {
        elt = (uint)(10*i);
        CuAssertTrue(tc, hashset_contains(set, &elt));
        hashset_add(set, &elt);
        CuAssertTrue(tc, hashset_contains(set, &elt));
        CuAssertSizeTEquals(tc, n, hashset_size(set));
    }
    
    // delete half the elements
    for (uint i=0; i<n/2; i++) {
        CuAssertSizeTEquals(tc, n-i, hashset_size(set));
        elt  = (uint)(10*i);
        CuAssertTrue(tc, hashset_contains(set, &elt));
        hashset_remove(set, &elt);
        CuAssertTrue(tc, !hashset_contains(set, &elt));
    }
    
    // delete nonexistent elements
    for (uint i=0; i<n/2; i++) {
        elt = (uint)(10*i);
        CuAssertTrue(tc, !hashset_contains(set, &elt));
        hashset_remove(set, &elt);
        CuAssertTrue(tc, !hashset_contains(set, &elt));
        CuAssertSizeTEquals(tc, n-(n/2), hashset_size(set));
    }

    // add them back
    for (uint i=0; i<n; i++) {
        CuAssertSizeTEquals(tc, MIN((n-(n/2)+i), n), hashset_size(set));
        elt = (uint)(10*i);
        hashset_add(set, &elt);
        CuAssertTrue(tc, hashset_contains(set, &elt));
    }
    CuAssertSizeTEquals(tc, n, hashset_size(set));
    
    elts = NEW_ARRAY(uint, n);
    iterator = hashset_get_iter(set);
    for( size_t i=0; hashset_iter_has_next(iterator); i++) {
        uint *e = (uint *)hashset_iter_next(iterator);
        elts[i] = *e;
        //printf("element[%zu] = %u\n",i,*e);
    }
    qsort(elts, n, sizeof(uint), &_uint_cmp_fn);
    for( size_t i=0; i<n; i++){
        CuAssertSizeTEquals(tc, i*10, (size_t)elts[i]);
        //printf("element[%zu] = %u\n",i, elts[i]);
    }

    hashset_iter_free(iterator);
    hashset_free(set, false);
    free(elts);
}


CuSuite *hashset_get_test_suite() 
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_hashset_indel);
    return suite;
}