#include <stddef.h>
#include <inttypes.h>

#include "CuTest.h"

#include "arrutil.h"
#include "errlog.h"
#include "kwayrng.h"


void test_kwayrng(CuTest *tc) 
{
    size_t nbits = 4;
    for (size_t k=0; k<10; k++) {
        kwayrng *rng = kwayrng_new(k, nbits);
        uint64_t *coef = kwayrng_coefs(rng);
        //PRINT_ARR(coef, "coef", "%"PRIu64, 0, k, 10);       
        for (size_t i=0; i<k; i++) {
            DEBUG("coef[%zu] = %"PRIu64"\n", i, coef[i]);
        }
        DEBUG("%zu-way independent sequence:\n", k);
        for (size_t i=0; i<kwayrng_maxval(rng); i++) {
            DEBUG("X[%zu] = %"PRIu64"\n", i,  kwayrng_next(rng));
        }
    }    
}


CuSuite *kwayrng_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_kwayrng);
    return suite;
}


