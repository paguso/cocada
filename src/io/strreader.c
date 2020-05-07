#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "mathutil.h"
#include "new.h"
#include "strread.h"
#include "strreader.h"

struct _strreader {
    strread _t_strread;
    char *src;
    size_t len;
    size_t index;
};


static void _strreader_reset(strread *t) 
{
    ((strreader *)(t->impltor))->index = 0;
}


static int _strreader_getc(strread *t)
{
    strreader *rdr = (strreader *) t->impltor;
    if (rdr->index < rdr->len) {
        return rdr->src[rdr->index++];
    } else {
        return EOF;
    }
}

size_t  _strreader_read_str(strread *t, char *dest, size_t n)
{
    strreader *rdr = (strreader *) t->impltor;
    size_t r = MIN(n, rdr->len - rdr->index); 
    strncpy(dest, rdr->src + rdr->index, r);
    return r;
}


size_t  _strreader_read_str_until(strread *t, char *dest, char delim)
{
    strreader *rdr = (strreader *) t->impltor;
    size_t i, j;
    for (i = rdr->index, j = 0; i < rdr->len && rdr->src[i] != delim; i++) {
        dest[j++] = rdr->src[i];
    }
    dest[j] = '\0';
    rdr->index = i;
    return j;
}


static strread_vt _strreader_vt = { .reset = _strreader_reset, 
                                    .getc = _strreader_getc,
                                    .read_str = _strreader_read_str,
                                    .read_str_until = _strreader_read_str_until };


strreader *strreader_open(char *src, size_t len)
{
    strreader *ret = NEW(strreader);
    ret->_t_strread.impltor = ret;
    ret->_t_strread.vt = &_strreader_vt;
    ret->src = src;
    ret->len = len;
    ret->index = 0;
    return ret;
}


void strreader_close(strreader *rdr)
{
    FREE(rdr);
}


IMPL_TRAIT(strreader, strread)
