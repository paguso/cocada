#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "strread.h"
#include "strfileread.h"
#include "xchar.h"
#include "xstring.h"


struct _strfileread {
    strread _t_strread;
    FILE *src;
    size_t pos;
    size_t sizeof_char;
};


IMPL_TRAIT(strfileread, strread)



static void _reset(void *self)
{
    rewind( ((strfileread *)self)->src );
}


static xchar_t _getc(void *self)
{
    return fgetc( ((strfileread *)self)->src );
}


static size_t _read_str(void *self, char *dest, size_t n)
{
	return fread(dest, ((strfileread *)self)->sizeof_char, n, ((strfileread *)self)->src);
}


static size_t _read_str_until(void *self, char *dest, char delim)
{
    FILE *src = ((strfileread *)self)->src;
    size_t nread;
    char c;
    for (nread=0; !feof(src); nread++) {
        if ( (c=fgetc(src)) == delim ) {
            fseek(src, -1, SEEK_CUR);
            break;
        }
        dest[nread] = c;
    }
    return nread;
}

/*
static size_t _read_xstr(void *self, xstring *xstr, size_t n)
{
    xchar_t c;
	size_t bpc = xstr_sizeof_char(dest);
	size_t nread=0;
	while (nread < n) {
		if (fread(&c, bpc, 1, sst->src.file) == bpc)
			xstr_set(dest, nread++, c);
        else
			break;
	}
	return nread;
}


static size_t _read_xstr_until(void *self, xstring *xstr, xchar_t delim)
{
    return 0;
}


static void  _close(void *self)
{

}
*/

static size_t _sizeof_char(void *self)
{
    return ((strfileread*)self)->sizeof_char;
}


static strread_vt _strread_vt  = 
{
    .getc = _getc,
    .read_str = _read_str,
    .read_str_until = _read_str_until,
    .reset = _reset,
    .sizeof_char = _sizeof_char
};


strfileread *strfileread_open(char *filename)
{
   	strfileread *ret = NEW(strfileread);
    ret->_t_strread.impltor = ret;
    ret->_t_strread.vtbl = &_strread_vt;
	ret->src = fopen(filename, "r");
    ret->pos = 0;
	ret->sizeof_char = sizeof(char);
	return ret;
}
