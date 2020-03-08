#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "alphabet.h"
#include "arrutil.h"
#include "new.h"
#include "order.h"
#include "minqueue.h"
#include "new.h"
#include "hashmap.h"
#include "vec.h"
#include "xstring.h"

#include "mmindex.h"

struct _mmindex {
    alphabet *ab;
    size_t nparam;
    size_t *w;
    size_t *k;
    hashmap **tbls;
    size_t nstr;
    vec *offs;
};


mmindex *mmindex_new(alphabet *ab, size_t n, size_t *w, size_t *k)
{
    mmindex *ret = NEW(mmindex);
    ret->ab = ab;
    ret->nparam = n;
    ret->w = w;
    ret->k = k;
    ret->tbls = NEW_ARR(hashmap*, ret->nparam);
    for (size_t i=0; i<ret->nparam; i++) {
        ret->tbls[i] = hashmap_new( sizeof(uint64_t), vec_sizeof(), 
                                    ident_hash_uint64_t, eq_uint64_t );
    }
    ret->nstr = 0;
    ret->offs = vec_new(sizeof(size_t));
    return ret;
}

void mmindex_dispose(void *ptr, const dtor *dt)
{
    mmindex *mm = (mmindex *)ptr;
    alphabet_free(mm->ab);
    FREE(mm->w);
    FREE(mm->k);
    dtor *hmdt = dtor_cons( dtor_cons( DTOR(hashmap), empty_dtor() ), DTOR(vec) );
    for (size_t i=0; i < mm->nparam; i++) {
        FINALISE(mm->tbls[i], hmdt);
        FREE(mm->tbls[i]);
    }
    FREE(mm->tbls);
    dtor_free(hmdt);
    FREE(mm->offs, vec);
}