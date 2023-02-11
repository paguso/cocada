#include "CuTest.h"
#include "errlog.h"
#include "memdbg.h"
#include "new.h"
#include "order.h"
#include "skiplist.h"


int cmp_ptr_to_int(const void *left, const void *right) {
    return cmp_int(*(int**)left, *(int**)right);
}

void skiplist_ins_test(CuTest *tc) {
    memdbg_reset();
    //insert owned objects
    skiplist *sl = skiplist_new(sizeof(int), cmp_int);
    skiplist_iter *it;
    for (int key = 10; key < 100; key +=10) {
        skiplist_ins(sl, &key);    
    }
    for (int key = 5; key < 100; key +=10) {
        skiplist_ins(sl, &key);    
    }
    size_t len = skiplist_len(sl);
    for (int key = 5; key < 100; key +=10) { 
        // duplicate insertions should do nothing
        skiplist_ins(sl, &key);    
    }
    CuAssertSizeTEquals(tc, len, skiplist_len(sl));
    it = skiplist_get_iter(sl);
    FOREACH_IN_ITER(key, int *, skiplist_iter_as_iter(it)) {
        DEBUG("key = %d\n", *key);
    }
    FREE(it);
    DESTROY(sl, finaliser_cons(FNR(skiplist), finaliser_new_empty()));
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
    //insert non-owned objects
    sl = skiplist_new(sizeof(int*), cmp_ptr_to_int);
    int keys[10] = {8, 9, 2, 3, 1, 4, 0, 7, 5, 6 };
    for (int *key = keys; key < keys + 10; key++) {
        skiplist_ins(sl, &key);    
    }
    len = skiplist_len(sl);
    for (int *key = keys; key < keys + 10; key++) { 
        // duplicate insertions should do nothing
        skiplist_ins(sl, &key);    
    }
    CuAssertSizeTEquals(tc, len, skiplist_len(sl));
    it = skiplist_get_iter(sl);
    FOREACH_IN_ITER(key, int **, skiplist_iter_as_iter(it)) {
        DEBUG("key = %d\n", **key);
    }
    FREE(it);
    DESTROY_FLAT(sl, skiplist);
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
}


void skiplist_del_test(CuTest *tc) {
    memdbg_reset();
    //insert owned objects
    skiplist *sl = skiplist_new(sizeof(int), cmp_int);
    for (int key = 10; key < 100; key +=10) {
        int *keymem = malloc(sizeof(int));
        *keymem = key;
        skiplist_ins(sl, keymem);    
    }
    for (int key = 10; key < 100; key +=20) {
        int *del_key = (int*) skiplist_del(sl, &key);
        CuAssertIntEquals(tc, key, *del_key);
    }
    skiplist_iter *it = skiplist_get_iter(sl);
    FOREACH_IN_ITER(key, int *, skiplist_iter_as_iter(it)) {
        DEBUG("key = %d\n", *key);
    }
    FREE(it);
    DESTROY(sl, finaliser_cons(FNR(skiplist), finaliser_new_ptr()));
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
    //insert non-owned objects
    sl = skiplist_new(sizeof(int), cmp_int);
    int *keys = calloc(200, sizeof(int));
    int n = 0;
    for (int key = 10; key < 100; key +=10) {
        keys[n] = key;
        int *keymem = &keys[n++];
        skiplist_ins(sl, keymem);    
    }
    for (int key = 5; key < 100; key +=10) {
        keys[n] = key;
        int *keymem = &keys[n++];
        skiplist_ins(sl, keymem);    
    }
    it = skiplist_get_iter(sl);
    FOREACH_IN_ITER(key, int *, skiplist_iter_as_iter(it)) {
        DEBUG("key = %d\n", *key);
    }
    FREE(it);
    DESTROY_FLAT(sl, skiplist);
    FREE(keys);
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
}


CuSuite *skiplist_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, skiplist_ins_test);
    //SUITE_ADD_TEST(suite, skiplist_del_test);
    return suite;
}