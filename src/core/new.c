#include <stdlib.h>
#include <stdio.h>

#include "new.h"

void consume_dstr(dstr *d) {
    if (d==NULL) return;
    for (size_t i=0; i<d->nchd; i++) {
        consume_dstr(((dstr **)d->chd_dsts)[i]);
    }
    FREE(d->chd_dsts);
    FREE(d);
}

size_t dstr_nchd(dstr *dst) 
{
    return dst->nchd;
}

dstr *dstr_chd(dstr *par, size_t index)
{
    return ((dstr **)par->chd_dsts)[index];
}


dstr *dstr_cons(dstr *par, dstr *chd)
{
    par->chd_dsts = (dstr **) realloc(par->chd_dsts, par->nchd+1 * sizeof(dstr *));
    par->chd_dsts[par->nchd] =  chd;
    par->nchd++;
    return par;
}


static void _empty_free(void *ptr, dstr *chain ) {}


dstr *empty_dstr() {
    dstr *ret = NEW(dstr);
    ret->freer = _empty_free;
    ret->nchd = 0;
    return ret;
} 


static void _raw_free(void *ptr, dstr *chain ) 
{
    FREE(ptr);
}


dstr *raw_dstr() {
    dstr *ret = NEW(dstr);
    ret->freer = _raw_free;
    ret->nchd = 0;
    return ret;
}
