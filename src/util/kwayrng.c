#include <stddef.h>
#include <stdint.h>

#include "arrutil.h"
#include "coretype.h"
#include "errlog.h"
#include "kwayrng.h"
#include "mathutil.h"
#include "new.h"
#include "randutil.h"


struct  _kwayrng
{
    size_t k;
    uint64_t maxval;
    uint64_t *coefs;    
    uint64_t count;
};


kwayrng *kwayrng_new(size_t k, size_t nbits)
{
    ERROR_ASSERT(nbits<64, "Maximum number of random bits is 63");
    kwayrng *ret = NEW(kwayrng);
    ret->k = k;
    ret->maxval = prime_succ((1 << nbits) - 1 );
    ret->coefs = NEW_ARR(uint64_t, k);
    for (size_t i=0; i<k; i++) {
        ret->coefs[i] = rand_range_uint64_t(1, ret->maxval);
    }
    ret->count = 0;
    return ret;
}


kwayrng *kwayrng_new_with_coefs(size_t k, uint64_t *coefs, size_t nbits)
{
    ERROR_ASSERT(nbits<64, "Maximum number of random bits is 63");
    kwayrng *ret = NEW(kwayrng);
    ret->k = k;
    ret->maxval = prime_succ((1 << nbits) - 1 );
    ret->coefs = NEW_ARR(uint64_t, k);
    for (size_t i=0; i<k; i++) {
        ret->coefs[i] = coefs[i] % ret->maxval;
    }
    ret->count = 0;
    return ret;
}




size_t kwayrng_k(kwayrng *rng)
{
    return rng->k;
}


const uint64_t *kwayrng_coefs(kwayrng *rng)
{
    return rng->coefs;
}


void kwayrng_reset(kwayrng *rng) {
    rng->count = 0;
}


uint64_t kwayrng_maxval(kwayrng *rng) {
    return rng->maxval;
}


uint64_t kwayrng_next(kwayrng *rng) {
    uint64_t ret = 0;
    for (uint64_t i=0, pow=1; i < rng->k; i++) {
        ret = mod_sum(ret, mod_mult(rng->coefs[i], pow, rng->maxval), rng->maxval);
        pow = mod_mult(pow, rng->count, rng->maxval);
    }
    rng->count = (rng->count + 1) % rng->maxval;
    return ret;
}

