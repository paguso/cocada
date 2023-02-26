#include "CuTest.h"
#include "avlordmap.h"
#include "errlog.h"
#include "memdbg.h"
#include "order.h" 
#include "randutil.h"

void avlordmap_test_set(CuTest *tc)
{
    memdbg_reset();
    int n = 10000;
    int range = 10*n;
    bool *set = calloc(range, sizeof(bool));
    avlordmap *map = avlordmap_new(sizeof(int), sizeof(int *), cmp_int);
    while (n) 
    {
        int key = rand_range_int(0, range);
        if (!set[key]) {
            set[key] = true;
            n--;
            int *val = malloc(sizeof(int));
            *val = key;
            avlordmap_set(map, &key, &val);
        }
    }
    for (int key = 0; key < range; key++) {
        bool has = avlordmap_contains(map, &key);
        CuAssert(tc, "Wrong map result", has == set[key]);
        if (has) {
            int **val = avlordmap_get(map, &key);
            CuAssertIntEquals(tc, key, **val);
        }
    }
    free(set);
    DEBUG("AVLordmap size=%zu\n", memdbg_total());

    avl_iter *it = avlordmap_get_iter(map);
    FOREACH_IN_ITER(entry, void, it) {
        int *key = (int*) avlordmap_key(map, entry);
        int *val = *((int**) avlordmap_val(map, entry));
        CuAssertIntEquals(tc, *key, *val);
    }
    avl_iter_free(it);

    DESTROY(map, finaliser_cons(finaliser_cons(FNR(avlordmap), finaliser_new_empty()), finaliser_new_ptr()));
    memdbg_print_stats(stdout, true);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
}


CuSuite *avlordmap_get_test_suite()
{
    CuSuite *ret = CuSuiteNew();
    SUITE_ADD_TEST(ret, avlordmap_test_set);
    return ret;
}
