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
        const uint64_t *coef = kwayrng_coefs(rng);
        FPRINT_ARR(stderr, coef, coef, %zu, 0, k, 10);       
        DEBUG("%zu-way independent sequence:\n", k);
        for (size_t i=0; i<kwayrng_maxval(rng); i++) {
            DEBUG("X[%zu] = %"PRIu64"\n", i,  kwayrng_next(rng));
        }
    }    
}

void test_uniform(CuTest *tc) {
    size_t *counts = NEW_ARR_0(size_t, 17);
    uint64_t *coefs = NEW_ARR(uint64_t, 3);
    for (size_t k0=0; k0<17; k0++) 
    {
        coefs[0] = k0;
    for (size_t k1=0; k1<17; k1++) 
    {
        coefs[1] = k1;
    for (size_t k2=0; k2<17; k2++) 
    {
        coefs[2] = k2;
        kwayrng *rng = kwayrng_new_with_coefs(3, coefs,  4);
        for (size_t x=0; x<17; x++) {
            uint64_t val = kwayrng_next(rng);
            counts[(size_t)val]++;
        }
    }    
    }    
    }    
    PRINT_ARR(counts, counts, %zu, 0, 17, 17);
}


CuSuite *kwayrng_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_kwayrng);
    SUITE_ADD_TEST(suite, test_uniform);
    return suite;
}


