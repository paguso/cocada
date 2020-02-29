#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "alphabet.h"
#include "bitsandbytes.h"
#include "new.h"
#include "mathutil.h"
#include "xstring.h"

static const size_t MIN_CAP = 4;
static const float  GROW_BY = 1.5f;


struct _xstring {
    size_t  len;
    size_t  cap;
    size_t  bytes_per_char;
    void   *str;
};

xstring *xstring_new(size_t bytes_per_char)
{
    return xstring_new_with_capacity(MIN_CAP, bytes_per_char);
}


xstring *xstring_new_with_capacity(size_t cap, size_t bytes_per_char)
{
    assert(bytes_per_char<=XCHAR_BYTESIZE);
    xstring *ret = NEW(xstring);
    ret->bytes_per_char = MAX(1, bytes_per_char);
    ret->len = 0;
    ret->cap = MAX(MIN_CAP, cap) + 1;
    ret->str = calloc(cap, bytes_per_char);
    return ret;
}


xstring *xstring_new_with_len(size_t len, size_t bytes_per_char)
{
    xstring *ret = xstring_new_with_capacity(len, bytes_per_char);
    ret->len = len;
    return ret;
}


void xstring_free(xstring *xs)
{
    if (xs==NULL) return;
    FREE(xs->str);
    FREE(xs);
}


void xstr_print(xstring *xs)
{
    printf("xstring@%p {\n", xs);
    printf("  len : %zu\n", xs->len);
    printf("  bytes_per_char: %zu\n", xs->bytes_per_char);
    printf("  str: ");
    for (size_t i=0; i < xs->len; i++) {
        printf("%zu%s", (size_t)xstr_get(xs, i), (i<xs->len-1)?"-":"");
    }
    printf("} # end of xstring@%p\n", xs);
}


void xstr_to_string (xstring *xs, dynstr *dest) 
{
    for (size_t i=0; i < xs->len; i++) {
        if (i) dstr_append(dest, "-");
        xchar_t c = xstr_get(xs, i);
        size_t  n = c;
        size_t  ord = 1;
        for (; n>=10; n/=10) ord*=10;
        char d[2] = "\0\0";
        n = c;
        while (ord) {
            d[0] = '0' + (n/ord);
            n = n % ord;
            ord /= 10;
            dstr_append(dest, d);
        }
    }
}


inline xchar_t xstr_get(xstring *xs, size_t pos)
{
    xchar_t ret=0x0;
    memcpy(&ret, xs->str+(pos*xs->bytes_per_char), xs->bytes_per_char);
#if ENDIANNESS==BIG
    xchar_flip_bytes(&ret);
#endif
    return ret;
}


inline void xstr_set(xstring *xs, size_t pos, xchar_t val)
{
#if ENDIANNESS==BIG
    xchar_flip_bytes(&val);
#endif
    memcpy(xs->str+(pos*xs->bytes_per_char), &val, xs->bytes_per_char);
}


void xstr_nset(xstring *xs, size_t n, xchar_t val)
{
    if (val==0) 
        memset( xs->str, 0, n*xs->bytes_per_char );
    else 
        for (int i=0; i<n; i++) 
            xstr_set(xs, i, val);
    xs->len = MAX(n, xs->len);
}


size_t xstr_len(xstring *xs)
{
    return xs->len;
}


size_t xstr_sizeof_char(xstring *xs)
{
    return xs->bytes_per_char;
}


static void check_and_grow_by(xstring *xs, size_t n)
{
    if ( (xs->cap - xs->len) >= n+1 ) return;
    while ( (xs->cap - xs->len) < n+1)
        xs->cap *= GROW_BY;
    xs->str = realloc(xs->str, xs->cap*xs->bytes_per_char);
    memset( xs->str + (xs->len * xs->bytes_per_char),  0, 
            (xs->cap - xs->len) * xs->bytes_per_char );
}


void xstr_push(xstring *xs, xchar_t c) 
{
    check_and_grow_by(xs, 1);
    xstr_set(xs, (xs->len)++, c);
}


void xstr_cat(xstring *dest, xstring *src)
{
    assert(dest->bytes_per_char == src->bytes_per_char);
    check_and_grow_by(dest, src->len);
    memcpy( dest->str+(dest->len*dest->bytes_per_char), 
            src->str, src->len*dest->bytes_per_char );
}


void xstr_cpy(xstring *dest, xstring *src)
{
    xstr_ncpy(dest, 0, src, 0, src->len);
}


void xstr_ncpy( xstring *dest, size_t from_dest, xstring *src, size_t from_src, 
                size_t nxchars )
{
    assert(dest->bytes_per_char == src->bytes_per_char);
    size_t deltalen = from_dest + nxchars - dest->len;
    check_and_grow_by(dest, deltalen);
    memcpy( dest->str+(from_dest*dest->bytes_per_char),
            src->str+(from_src*src->bytes_per_char), 
            nxchars*dest->bytes_per_char );
    dest->len += deltalen; 
}


void xstr_trim(xstring *xs) 
{
    xs->cap = MAX(MIN_CAP, xs->len) + 1;
    xs->str = realloc(xs->str, xs->cap * xs->bytes_per_char);
    memset( xs->str + (xs->len * xs->bytes_per_char), 0x0, 
            (xs->cap - xs->len) * xs->bytes_per_char );
}


void xstr_clip(xstring *xs, size_t from, size_t to)
{
    size_t n = (from < xs->len && from < to) ? MIN(to, xs->len) - from : 0;
    if (n) 
        memmove( xs->str, xs->str + (from * xs->bytes_per_char), 
                 n * xs->bytes_per_char );
    xs->len = n;
}


void xstr_clear(xstring *xs) 
{
    xs->len = 0;
}


void *xstr_detach(xstring *xs)
{
    xstr_trim(xs);
    void *ret = xs->str;
    FREE(xs);
    return ret;
}


int xstr_ncmp(xstring *this, xstring *other, size_t n) {
    size_t m = MIN3(n, this->len, other->len);
    intmax_t cmp;
    for (size_t i=0; i<m;  i++) {
        cmp = (intmax_t)xstr_get(this, i) - (intmax_t)xstr_get(other, i);
        if (cmp) return cmp/abs(cmp);
    }
    if (n <= MIN(this->len, other->len)) return 0;
    else {
        if (this->len == other->len) return 0;
        else if (this->len < other->len) return -1;
        else return +1;
    }
}


int xstr_cmp(xstring *this, xstring *other) {
    return xstr_ncmp(this, other, MIN(this->len, other->len));
}
