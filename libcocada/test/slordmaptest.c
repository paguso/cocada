#include "CuTest.h"
#include "errlog.h"
#include "memdbg.h"
#include "order.h" 
#include "randutil.h"
#include "slordmap.h"

void slordmap_test_set(CuTest *tc)
{
    memdbg_reset();
    int n = 100;
    int range = 10*n;
    bool *set = calloc(range, sizeof(bool));
    slordmap *map = slordmap_new(sizeof(int), sizeof(int *), cmp_int);
    while (n) 
    {
        int key = rand_range_int(0, range);
        if (!set[key]) {
            set[key] = true;
            n--;
            int *val = malloc(sizeof(int));
            *val = key;
            slordmap_set(map, &key, &val);
        }
    }
    for (int key = 0; key < range; key++) {
        bool has = slordmap_contains(map, &key);
        CuAssert(tc, "Wrong map result", has == set[key]);
        if (has) {
            int **val = slordmap_get(map, &key);
            CuAssertIntEquals(tc, key, **val);
        }
    }
    free(set);

    skiplist_iter *it = slordmap_get_iter(map);
    FOREACH_IN_ITER(entry, void, it) {
        int *key = (int*) slordmap_key(map, entry);
        int *val = *((int**) slordmap_val(map, entry));
        CuAssertIntEquals(tc, *key, *val);
    }

    DESTROY(map, finaliser_cons(finaliser_cons(FNR(slordmap), finaliser_new_empty()), finaliser_new_ptr()));
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());

}


CuSuite *slordmap_get_test_suite()
{
    CuSuite *ret = CuSuiteNew();
    SUITE_ADD_TEST(ret, slordmap_test_set);
    return ret;
}
