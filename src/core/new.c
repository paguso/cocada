#include <stdlib.h>
#include <stdio.h>

#include "new.h"


dtor *dtor_new_with_func( dstr_func df ) {
    dtor *dt = NEW(dtor);
    dt->df = df;
    dt->nchd = 0;
    dt->chd = calloc(1, sizeof(dtor *));
    return dt;
}


void dtor_free(dtor *d) {
    if (d==NULL) return;
    for (size_t i=0; i<d->nchd; i++) {
        dtor_free(((dtor **)d->chd)[i]);
    }
    FREE(d->chd);
    FREE(d);
}

size_t dtor_nchd(dtor *dst) 
{
    return dst->nchd;
}

dtor *dtor_chd(dtor *par, size_t index)
{
    return ((dtor **)par->chd)[index];
}


dtor *dtor_cons(dtor *par, dtor *chd)
{
    par->chd = (dtor **) realloc(par->chd, par->nchd+1 * sizeof(dtor *));
    par->chd[par->nchd] =  chd;
    par->nchd++;
    return par;
}


static void _empty_free(void *ptr, dtor *dt ) {
}


dtor *empty_dtor() {
    dtor *ret = NEW(dtor);
    ret->df = _empty_free;
    ret->nchd = 0;
    return ret;
} 


static void _raw_free(void *ptr, dtor *dt ) 
{
    FREE(ptr);
}


dtor *ptr_dtor() {
    dtor *ret = NEW(dtor);
    ret->df = _raw_free;
    ret->nchd = 0;
    return ret;
}
