#include <stddef.h>
#include <string.h>


#include "mathutil.h"
#include "new.h"
#include "trait.h"
#include "xchar.h"
#include "xstr.h"
#include "xstrread.h"
#include "xstrreader.h"


struct _xstrreader
{
    xstrread _t_xstrread;
    void     *src;
    size_t   index;
    size_t   len;
};


static void _xstr_reset(xstrread *t) 
{
    ((xstrreader *)(t->impltor))->index = 0;
}


static xchar_wt _xstr_getc(xstrread *t)
{
    xstrreader *rdr = (xstrreader *) t->impltor;
    xstr *src = (xstr *) rdr->src;
    if (rdr->index < xstr_len(src))  {
        return xstr_get(src, rdr->index++);
    } else {
        return XEOF;
    }
}

size_t  _xstr_read(xstrread *t, xstr *dest, size_t n)
{
    xstrreader *rdr = (xstrreader *) t->impltor;
    xstr *src = (xstr *) rdr->src;
    size_t r = MIN(n, xstr_len(src) - rdr->index); 
    xstr_ncpy(dest, 0, src, rdr->index, r);
    return r;
}


size_t  _xstr_read_until(xstrread *t, xstr *dest, xchar_t delim)
{
    xstrreader *rdr = (xstrreader *) t->impltor;
    xstr *src = (xstr *) rdr->src;
    size_t i;
    for (i = rdr->index; i < xstr_len(src) && xstr_get(src, i) != delim; i++);
    size_t r = i - rdr->index;
    xstr_ncpy(src, 0, dest, rdr->index, r);
    rdr->index = i;
    return r;
}


static xstrread_vt _xstr_vt = { .reset = _xstr_reset, 
                                .getch = _xstr_getc,
                                .read = _xstr_read,
                                .read_until = _xstr_read_until };




static xchar_wt _str_getc(xstrread *t)
{
    xstrreader *rdr = (xstrreader *) t->impltor;
    char *src = (char *) rdr->src;
    if (rdr->index < rdr->len) {
        return src[rdr->index++];
    } else {
        return XEOF;
    }
}


size_t  _str_read(xstrread *t, xstr *dest, size_t n)
{
    xstrreader *rdr = (xstrreader *) t->impltor;
    char *src = (char *) rdr->src;
    size_t i, j, l;
    for (i = rdr->index, j = 0, l = MIN(xstr_len(dest), n); j < l && i < rdr->len ; i++, j++) {
        xstr_set(dest, j, src[i]);
    }
    for (; j < n && i < rdr->len ; i++, j++) {
        xstr_push(dest, src[i]);
    }
    rdr->index = i;
    return j;
}


size_t  _str_read_until(xstrread *t, xstr *dest, xchar_t delim)
{
    xstrreader *rdr = (xstrreader *) t->impltor;
    char *src = (char *) rdr->src;
    size_t i, j, l;
    for (i = rdr->index, j = 0, l = xstr_len(dest); j < l && i < rdr->len && (xchar_t) src[i] != delim; i++, j++) {
        xstr_set(dest, j, src[i]);
    }
    for (; i < rdr->len && (xchar_t) src[i] != delim; i++, j++) {
        xstr_push(dest, src[i]);
    }
    rdr->index = i;
    return j;
}


static xstrread_vt _str_vt = { .reset = _xstr_reset, 
                               .getch = _str_getc,
                               .read = _str_read,
                               .read_until = _str_read_until };


xstrreader *xstrreader_open(xstr *src)
{
    xstrreader *ret = NEW(xstrreader);
    ret->_t_xstrread.impltor = ret;
    ret->_t_xstrread.vt = &_xstr_vt;
    ret->src = src;
    ret->index = 0;
    return ret;
}


xstrreader *xstrreader_open_str(char *src, size_t len)
{
    xstrreader *ret = NEW(xstrreader);
    ret->_t_xstrread.impltor = ret;
    ret->_t_xstrread.vt = &_str_vt;
    ret->src = src;
    ret->len = strlen(src);
    ret->index = 0;
    return ret;
}


void xstrreader_close(xstrreader *self)
{
    FREE(self);
}


IMPL_TRAIT(xstrreader, xstrread)