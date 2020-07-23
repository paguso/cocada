#include <stddef.h>

#include "CuTest.h"

#include "bitvec.h"
#include "errlog.h"
#include "fmalg.h"
#include "randutil.h"


void test_fmalg(CuTest *tc) 
{
    uint64_t maxval = (uint64_t)1<<16;
    bitvec *ticks = bitvec_new_with_capacity(maxval);
    fmalg *fm  = fmalg_init(maxval);
    uint64_t val=0, true_count=0;
    for (size_t i=0; i<100000; i++) {
        val = rand_range_uint64_t(0, maxval);
        if ( bitvec_get_bit(ticks, val) == 0 ) {
            true_count++;
            bitvec_set_bit(ticks, val, 1);
        }
        fmalg_process(fm, val);
        if (i%10 == 0) {
            uint64_t f0 = fmalg_query(fm);
            DEBUG("FM estimate = %"PRIu64" true count = %"PRIu64" error = %f\n", f0, true_count, (double)(f0-true_count)/true_count);
        }

    }
}


CuSuite *fmalg_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_fmalg);
    return suite;
}