#include "CuTest.h"

#include "errlog.h"
#include "segtree.h"


void sum_int(const void *left, const void *val, void *dest)
{
    int *res = (int *) dest;
    *res = *((int *)left) + *((int *)right);
}


void test_segtree_upd(CuTest *tc)
{
    int zero = 0;
    segtree *tree = segtree_new(range, sizeof(int), sum_int, &zero);
}



CuSuite *segtree_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_segtree_upd);
    return suite;
}

