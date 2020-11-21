#include "CuTest.h"
#include "avl.h"
#include "errlog.h"
#include "order.h"
#include "randutil.h"


void print_int(FILE *stream, const void *ptr) {
    fprintf(stream, "%d", *((int *)ptr));
}


void test_avl_push(CuTest *tc) 
{
    int half_univ = 100;
    avl *tree = avl_new(sizeof(int), cmp_int);
    for (int i = 0; i < half_univ; i++) {
        int val = half_univ + ((i % 2) ? i : -i);
        DEBUG("Insert %d\n", val);
        avl_push(tree, &val);
        DEBUG_ACTION(avl_print(tree, stderr, print_int));
    }
    DEBUG_ACTION(avl_print(tree, stderr, print_int));
    DEBUG("\n\n\n");

    DESTROY(tree, dtor_cons(DTOR(avl), ptr_dtor()));
}



CuSuite *avl_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_avl_push);
    return suite;
}