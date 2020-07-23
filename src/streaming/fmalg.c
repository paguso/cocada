#include <inttypes.h>

#include "bitbyte.h"
#include "errlog.h"
#include "fmalg.h"
#include "kwayrng.h"
#include "mathutil.h"
#include "new.h"


struct _fmalg {
    kwayrng *rng;
    uint64_t maxval;
    byte_t maxlsb;
};


fmalg *fmalg_init(uint64_t maxval)
{
    fmalg *ret = NEW(fmalg);
    ret->maxval = maxval;
    uint64_t p2ceil = pow2ceil_uint64_t(maxval);            
    ret->rng = kwayrng_new(2, uint64_lobit(p2ceil));
    ret->maxlsb = 0;
    return ret;
}


void fmalg_free(fmalg *fm)
{
    FREE(fm);
}


void fmalg_reset(fmalg *fm)
{
    fm->maxlsb = 0;
}


void fmalg_process(fmalg *fm, uint64_t val)
{
    WARN_ASSERT(val<=fm->maxval, "Ignoring invalid FM value %"PRIu64\
                ". Max allowed value is %"PRIu64"", val, fm->maxval);
    const uint64_t hashval = kwayrng_val(fm->rng, val); 
    const byte_t lsb = uint64_lobit( hashval );
    fm->maxlsb = MAX(lsb, fm->maxlsb);
}


uint64_t fmalg_query(fmalg *fm)
{
    return (1 << fm->maxlsb);
}