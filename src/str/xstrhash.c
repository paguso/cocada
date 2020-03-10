#include <stddef.h>
#include <stdint.h>

#include "alphabet.h"
#include "arrutil.h"
#include "math.h"
#include "new.h"
#include "xstring.h"
#include "xstrhash.h"

struct _xstrhash {
    alphabet *ab;
    size_t max_exp;
    uint64_t *pow;
};


static void _initpow(xstrhash *self)
{
    uint64_t base = ab_size(self->ab);
    size_t e = 0;
    uint64_t p = 1;
    while ( (uint64_t)(base*p) > p ) {
        e++; 
        p *= base;
    }
    self->max_exp = e;
    self->pow = NEW_ARR(uint64_t, e);
    p = 1;
    for (size_t i=0; i<e; i++) {
        self->pow[i] = p;
        p *= base;
    }
}

static uint64_t _pow(const xstrhash *self, size_t exp) {
    uint64_t ret = 1;
    while (exp > self->max_exp) {
        ret *= self->pow[self->max_exp];
        exp -= self->max_exp;
    }
    return ret * self->pow[exp];
}


uint64_t xstrhash_new(const alphabet *ab)
{
    xstrhash *ret = NEW(xstrhash);
    ret->ab = alphabet_clone(ab);
    _initpow(ret);
    return ret;
}

void xstrhash_dispose(void *ptr, const dtor *dt)
{
    xstrhash *self = (xstrhash *)ptr;
    FREE(self->ab, alphabet);
}

uint64_t xstrhash_lex(const xstrhash *self, const xstring *s)
{
    uint64_t hash = 0;
    for (size_t i=0; i<xstr_len(s); i++) {
        hash *= ab_size(self->ab);
        hash += ab_rank(self->ab, xstr_get(s, i));
    }
    return hash;
}

uint64_t xstrhash_roll_lex(const xstrhash *self, const xstring *s, uint64_t hash, xchar_t c)
{
    hash -= _pow(self, xstr_len(s)-1) * ab_rank(self->ab, xstr_get(s, 0));
    hash += ab_rank(self->ab, c);
    return hash;
}
