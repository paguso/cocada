#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "CuTest.h"

#include "minqueue.h"
#include "deque.h"
#include "randutil.h"

typedef struct _minqobj {
    int i;
    double f;
    char c;
} minqobj;

int minqobj_cmp(const void *first, const void *second)
{
    minqobj *f = (minqobj *)first;
    minqobj *s = (minqobj *)second;
    if (f->i < s->i) return -1;
    else if (f->i > s->i) return +1;
    else if (f->f < s->f) return -1;
    else if (f->f > s->f) return +1;
    else if (f->c < s->c) return -1;
    else if (f->c > s->c) return +1;
    else return 0;
}


void test_minqueue_push_pop(CuTest *tc) 
{
    minqueue *q = minqueue_new(sizeof(minqobj), minqobj_cmp);
    deque *v = deque_new(sizeof(minqobj));

    size_t n = 1000;
    for (int i=0; i<n; i++) {
        int r = rand_range_int(0, n);
        minqobj o = {(int)r, (double)r, (char)r};
        minqueue_push(q, &o);
        deque_push_back(v, &o);
    }
    CuAssertSizeTEquals(tc, minqueue_len(q), n);
    CuAssertSizeTEquals(tc, deque_len(v), n);

    minqobj *min;
    for (int i=0; i<n/2; i++) {
        min = minqueue_min(q);
        for (int j=0; j<deque_len(v); j++) {
            CuAssert(tc, "Min is not minimmal", minqobj_cmp(min, deque_get(v, j)) <= 0);
        }
        minqueue_pop(q, NULL);
        deque_pop_front(v, NULL);
    }
    CuAssertSizeTEquals(tc, minqueue_len(q), n-(n/2));
    
    
    for (int i=0; i<n/2; i++) {
        int r = rand_range_int(0, n);
        minqobj o = {(int)r, (double)r, (char)r};
        minqueue_push(q, &o);
        deque_push_back(v, &o);
    }
    CuAssertSizeTEquals(tc, minqueue_len(q), n);
    CuAssertSizeTEquals(tc, deque_len(v), n);

    for (int i=0; i<n/2; i++) {
        min = minqueue_min(q);
        for (int j=0; j<deque_len(v); j++) {
            CuAssert(tc, "Min is not minimmal", minqobj_cmp(min, deque_get(v, j)) <= 0);
        }
        minqueue_pop(q, NULL);
        deque_pop_front(v, NULL);
    }
    CuAssertSizeTEquals(tc, minqueue_len(q), n-(n/2));

    min = minqueue_min(q);
    size_t nmin = 0;
    minqueue_iter *iter = minqueue_all_min(q);
    for (int j=0; j<deque_len(v); j++) {
        if (minqobj_cmp(min, deque_get(v, j))==0) {
            nmin++;
        }
    }
    CuAssert(tc, "must have at least one min", nmin>0);
    while (minqueue_iter_has_next(iter)) {
        CuAssert(tc, "iterator returns not min", minqobj_cmp(min, minqueue_iter_next(iter))==0);
        nmin--;        
    }
    CuAssertSizeTEquals(tc, 0, nmin);

}


CuSuite *minqueue_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_minqueue_push_pop);
    return suite;
}