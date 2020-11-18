#include <stdio.h>


#include "CuTest.h"

#include "qdigest.h"
#include "randutil.h"


void test_qdigest_upd(CuTest *tc) 
{
    size_t range = 1 << 10;
    size_t nupd = 4 * range;
    double err = 0.2;
    qdigest *sketch = qdigest_new(range, err);
    for (size_t i=0; i<nupd; i++) {
        size_t val = rand_range_size_t(0, range);
        size_t qty = rand_range_size_t(1, 10);
        qdigest_upd(sketch, val, qty);
        //printf("\n\nInsert #%zu val=%zu qty=%zu:\n\n", i, val, qty);
        //qdigest_print(sketch, stdout);

    }
}



CuSuite *qdigest_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_qdigest_upd);
    return suite;
}