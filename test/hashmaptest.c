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
#include "hash.h"
#include "hashmap.h"

bool eq_uint32_t(const void *a, const void *b) {
    return (uint32_t)(*((uint32_t *)a)) == (uint32_t)(*((uint32_t *)b));
}


void test_hashmap_int(CuTest *tc) 
{
    hashmap *hmap = hashmap_new(sizeof(uint32_t), sizeof(uint32_t), ident_hash_uint32_t, eq_uint32_t);

    size_t n = 1000000;
    for (int i=0; i<n; i++) {
        //printf("Adding [%d,%d] to hashmap\n", i, i);
        hashmap_set(hmap, &i, &i);
    }
    CuAssertSizeTEquals(tc, n, hashmap_size(hmap));
    
    for (int i=0; i<n; i++) {
        CuAssert(tc, "map should contain key", hashmap_has_key(hmap, &i));
        CuAssertIntEquals(tc, i, *((int *)hashmap_get(hmap, &i)));
    }
    CuAssertSizeTEquals(tc, n, hashmap_size(hmap));

    for (int i=0; i<n; i+=7) {
        CuAssert(tc, "map should contain key", hashmap_has_key(hmap, &i));
        hashmap_unset(hmap, &i);
        CuAssert(tc, "map should NOT contain key", !hashmap_has_key(hmap, &i));
        n--;
    }
    CuAssertSizeTEquals(tc, n, hashmap_size(hmap));
    hashmap_free(hmap, false, false);
}




typedef struct {
    uint64_t k1;
    uint64_t k2;
    char *k3;
} object;


uint64_t hash_bin_str(const void *key) {
    char *s = ((char **)key)[0];
    size_t n = strlen(s);
    uint64_t h = 0;
    uint64_t pow = 2;
    for (size_t i=0; i<n; i++) {
        h = (pow * h) +  (s[i]-'0');
    }
    return h;
}

bool bin_str_eq(const void *a, const void *b) {
    char *sa = ((char **)a)[0];
    char *sb = ((char **)b)[0];
    return strcmp(sa, sb)==0;
}

void test_hashmap_obj(CuTest *tc) 
{
    hashmap *hmap = hashmap_new(sizeof(char *), sizeof(object), hash_bin_str, bin_str_eq);

    uint64_t n = 10000;
    uint64_t mink = 1; mink <<= 32;
    uint64_t maxk = mink + n;
    for (uint64_t i=mink; i<maxk; i++) {
        char *k = cstr_new(64);
        uint_to_cstr(k, i, 'b');
        //printf("%zu => adding %s to hashmap\n", i-mink, k);
        object v = {i, i+1, k};
        hashmap_set(hmap, &k, &v);
    }
    CuAssertSizeTEquals(tc, n, hashmap_size(hmap));
    
    for (uint64_t i=mink; i<maxk; i++) {
        char *k = cstr_new(64);
        uint_to_cstr(k, i, 'b');
        CuAssert(tc, "map should contain key", hashmap_has_key(hmap, &k));
        object *v = (object *)hashmap_get(hmap, &k);
        CuAssert(tc, "wrong k1 value", i == v->k1);
        CuAssert(tc, "wrong k2 value", i+1 == v->k2);
        CuAssert(tc, "wrong k3 value", strcmp(k, v->k3)==0);
        FREE(k);
    }
    CuAssertSizeTEquals(tc, n, hashmap_size(hmap));

    for (uint64_t i=mink; i<maxk; i++) {
        char *k = cstr_new(64);
        uint_to_cstr(k, i, 'b');
        CuAssert(tc, "map should contain key", hashmap_has_key(hmap, &k));
        hashmap_unset(hmap, &k);
        CuAssert(tc, "map should NOT contain key", !hashmap_has_key(hmap, &k));
        n--;
        FREE(k);
    }
    CuAssertSizeTEquals(tc, n, hashmap_size(hmap));
    hashmap_free(hmap, true, false);
}



CuSuite *hashmap_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_hashmap_int);
    SUITE_ADD_TEST(suite, test_hashmap_obj);
    return suite;
}
