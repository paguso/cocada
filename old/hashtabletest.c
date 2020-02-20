#include <stdio.h>
#include <stdlib.h>

#include "CuTest.h"

#include "hashtable.h"

typedef struct {
    int i;
    short s;
} int2short;

static size_t hash_int2short(void *i2s)
{
    return (size_t)(((int2short*)i2s)->i);
}

static bool eq_int2short(void *a, void *b)
{
    return (((int2short*)a)->i)==(((int2short*)b)->i);
}

void test_hashtable_add(CuTest *tc) 
{
    hashtable *ht = hashtable_new(sizeof(int2short), &hash_int2short, &eq_int2short);
    size_t n = 10000;
    int2short i2s;
    for (int i=0; i<n; i++) {
        i2s.i = i;
        i2s.s = (short)i;
        hashtable_add(ht, &i2s);
        //printf("added (%d,%d)\n", i2s.i, i2s.s);
    }
    for (int i=0; i<n; i++) {
        i2s.i = i;
        i2s.s = 0;
        CuAssert(tc, "should contain", hashtable_contains(ht, &i2s));
        CuAssertIntEquals(tc, 0, i2s.s);
        hashtable_get_cpy(ht, &i2s, &i2s);
        CuAssertIntEquals(tc, (short)i, i2s.s);
    }
    hashtable_free(ht, false);
}


void test_hashtable_del(CuTest *tc) 
{
    hashtable *ht = hashtable_new(sizeof(int2short), &hash_int2short, &eq_int2short);
    size_t n = 10000;
    int2short i2s;
    for (int i=0; i<n; i++) {
        i2s.i = i;
        i2s.s = (short)i;
        hashtable_add(ht, &i2s);
        //printf("added (%d,%d)\n", i2s.i, i2s.s);
    }
    for (int i=0; i<n; i+=2) {
        i2s.i = i;
        i2s.s = 0;
        hashtable_del(ht, &i2s, false);
    }
    for (int i=0; i<n; i++) {
        i2s.i = i;
        i2s.s = 0;
        bool has = hashtable_contains(ht, &i2s);
        CuAssertIntEquals(tc, i%2, has);
        if (hashtable_get_cpy(ht, &i2s, &i2s))
            CuAssertIntEquals(tc, i, i2s.s);
    }
    hashtable_free(ht, false);
}


void test_hashtable_get(CuTest *tc) 
{
    hashtable *ht = hashtable_new(sizeof(int2short), &hash_int2short, &eq_int2short);
    size_t n = 10000;
    int2short i2s;
    for (int i=0; i<n; i++) {
        i2s.i = i;
        i2s.s = (short)i;
        hashtable_add(ht, &i2s);
        //printf("added (%d,%d)\n", i2s.i, i2s.s);
    }
    for (int i=0; i<n; i++) {
        CuAssert(tc, "should contain", hashtable_contains(ht, &i));
        CuAssertIntEquals(tc, i, ((int2short*)hashtable_get(ht, &i))->i);
    }
    hashtable_free(ht, false);
}


CuSuite *hashtable_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_hashtable_add);
    SUITE_ADD_TEST(suite, test_hashtable_del);
    SUITE_ADD_TEST(suite, test_hashtable_get);
    return suite;
} 