#include "CuTest.h"
#include "errlog.h"
#include "memdbg.h"
#include "new.h"
#include "order.h"
#include "skiplist.h"


void skiplist_ins_test(CuTest *tc) {
    memdbg_reset();
    skiplist *sl = skiplist_new(sizeof(int), cmp_int);
    for (int key = 10; key < 100; key +=10) {
        int *keymem = malloc(sizeof(int));
        *keymem = key;
        skiplist_ins(sl, keymem);    
    }
    for (int key = 5; key < 100; key +=10) {
        int *keymem = malloc(sizeof(int));
        *keymem = key;
        skiplist_ins(sl, keymem);    
    }
    DESTROY(sl, finaliser_cons(FNR(skiplist), finaliser_new_ptr()));
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
}


CuSuite *skiplist_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, skiplist_ins_test);
    return suite;
}