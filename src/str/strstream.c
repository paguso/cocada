/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 * 
 * Copyright (C) 2016  Paulo G S Fonseca
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "new.h"
#include "mathutil.h"
#include "strstream.h"
#include "xchar.h"
#include "xstring.h"


typedef enum {
    SSTR_STR   = 0,
    SSTR_FILE  = 1,
    SSTR_XSTR  = 2,
    SSTR_XFILE = 3,
} sstream_type;


struct _strstream {
    sstream_type type;
    union {
        FILE    *file;
        char    *str;
        xstring *xstr;
    } src;
    size_t bytes_per_char;
    size_t pos;
    size_t slen;
};


strstream *strstream_open_str(char *str, size_t slen)
{
    strstream *sst;
    sst = NEW(strstream);
    sst->type = SSTR_STR;
    sst->src.str = str;
    sst->pos = 0;
    sst->slen = slen;
    sst->bytes_per_char = sizeof(char);
    return sst;
}


strstream *strstream_open_xstr(xstring *xstr) 
{
    strstream *sst;
    sst = NEW(strstream);
    sst->type = SSTR_XSTR;
    sst->src.xstr = xstr;
    sst->pos = 0;
    sst->slen = xstr_len(xstr);
    sst->bytes_per_char = xstr_sizeof_char(xstr);
    return sst;

}

strstream *strstream_open_file(char *filename)
{
    strstream *sst;
    sst = NEW(strstream);
    sst->type = SSTR_FILE;
    sst->src.file = fopen(filename, "r");
    sst->pos = 0;
    sst->bytes_per_char = sizeof(char);
    return sst;
}


strstream *strstream_open_xfile(char *filename, size_t bytes_per_char)
{
    strstream *sst;
    sst = NEW(strstream);
    sst->type = SSTR_XFILE;
    sst->bytes_per_char = bytes_per_char;
    sst->src.file = fopen(filename, "rb");
    sst->pos = 0;
    return sst;
}


size_t strstream_sizeof_char(strstream *sst)
{
    return sst->bytes_per_char;
}


void strstream_reset(strstream *sst)
{
    switch (sst->type) {
    case SSTR_STR:
        sst->pos = 0;
        break;
    case SSTR_FILE:
        rewind(sst->src.file);
        break;
    case SSTR_XSTR:
        sst->pos = 0;
        break;
    case SSTR_XFILE:
        rewind(sst->src.file);
        break;
    }
}

bool strstream_end(strstream *sst)
{
    switch (sst->type) {
    case SSTR_STR:
        return sst->slen <= sst->pos;
        break;
    case SSTR_FILE:
        return feof(sst->src.file);
        break;
    case SSTR_XSTR:
        return sst->slen <= sst->pos;
        break;
    case SSTR_XFILE:
        return feof(sst->src.file);
        break;
    default:
        return true;
    }
}

xchar_t strstream_getc(strstream *sst)
{
    switch (sst->type) {
    case SSTR_STR:
        if (sst->pos>=sst->slen)
            return (xchar_t)EOF;
        else
            return (xchar_t)sst->src.str[sst->pos++];
        break;
    case SSTR_FILE:
        return fgetc(sst->src.file);
        break;
    case SSTR_XSTR:
        if (sst->pos>=xstr_len(sst->src.xstr))
            return XEOF;
        else
            return xstr_get(sst->src.xstr, sst->pos++);
        break;
    case SSTR_XFILE:
        ;
        xchar_t ret = 0;
        fread(&ret, sizeof(xchar_t),  1, sst->src.file);
        return ret;
        break;
    default:
        return '\0';
    }
}

size_t strstream_reads(strstream *sst, char *dest, size_t n)
{
    size_t nread;
    switch (sst->type) {
    case SSTR_STR:
        nread = MIN(n, (sst->pos<sst->slen)?(sst->slen-sst->pos):0);
        strncpy(dest, sst->src.str+sst->pos, nread);
        sst->pos += nread;
        //dest[nread] = '\0';
        return nread;
        break;
    case SSTR_FILE:
        return fread(dest, 1, n, sst->src.file);
        break;
    default:
        return 0;
    }
}

size_t strstream_readxs(strstream *sst, xstring *dest, size_t n)
{
    size_t nread;
    switch (sst->type) {
    case SSTR_XSTR:
        nread = MIN(n, (sst->pos<sst->slen)?(sst->slen-sst->pos):0);
        xstr_ncpy(dest, 0, sst->src.xstr, sst->pos, nread);
        sst->pos += nread;
        return nread;
        break;
    case SSTR_XFILE:;
        xchar_t c;
        size_t bpc = xstr_sizeof_char(dest);
        size_t nread=0;
        while (nread<n) {
            if (fread(&c, bpc, 1, sst->src.file) == bpc) 
                xstr_set(dest, nread++, c);
            else 
                break;
        }
        return nread;
        break;
    default:
        return 0;
    }
}


void strstream_close(strstream *sst)
{
    if (sst==NULL) return;
    switch (sst->type) {
    case SSTR_STR:
        break;
    case SSTR_FILE:
        fclose(sst->src.file);
        break;
    default:
        break;
    }
    FREE(sst);
}
