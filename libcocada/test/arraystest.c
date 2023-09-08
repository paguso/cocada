#include "CuTest.h"

#include "arrays.h"
#include "memdbg.h"

void test_matrix(CuTest *tc) {
    memdbg_reset();

    for (int m = 0; m < 100; m++) {
        printf("m = %d\n", m);
        for (int n = 0; n < 100; n++) {
            printf("n = %d\n", n);
            NEW_MATRIX(mat, int, m, n);
                    
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < n; j++) {
                    mat[i][j] = i * j;
                }
            }
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < n; j++)
                    CuAssertIntEquals(tc, i * j, mat[i][j]);
            }

            FREE_MATRIX(mat);
        }
    }

    CuAssert(tc, "Memory leak", memdbg_is_empty());
}



CuSuite *arrays_get_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_matrix);
    return suite;
}