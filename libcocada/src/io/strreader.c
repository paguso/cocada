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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "mathutil.h"
#include "new.h"
#include "strbuf.h"
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


static int _strreader_peek(strread *t)
{
	strreader *rdr = (strreader *) t->impltor;
	if (rdr->index < rdr->len) {
		return rdr->src[rdr->index];
	} else {
		return EOF;
	}
}

static bool _strreader_eof(strread *t)
{
	strreader *rdr = (strreader *) t->impltor;
	return  (rdr->index >= rdr->len);
}


static size_t _strreader_read_str(strread *t, char *dest, size_t n)
{
	strreader *rdr = (strreader *) t->impltor;
	size_t r = MIN(n, rdr->len - rdr->index);
	strncpy(dest, rdr->src + rdr->index, r);
	rdr->index += r;
	return r;
}


static size_t _strreader_read_strbuf(strread *t, strbuf *dest)
{
	strreader *rdr = (strreader *) t->impltor;
	size_t ret =  rdr->len - rdr->index;
	strbuf_nappend(dest, rdr->src + rdr->index, ret);
	rdr->index += ret;
	return ret;
}


static size_t _strreader_read_strbuf_until(strread *t, strbuf *dest, char delim)
{
	strreader *rdr = (strreader *) t->impltor;
	size_t r, ret;
	for (r = rdr->index; r < rdr->len && rdr->src[r] != delim; r++);
	ret = r - rdr->index;
	strbuf_nappend(dest, rdr->src + rdr->index, ret);
	rdr->index = r;
	return ret;
}


static size_t _strreader_read_str_until(strread *t, char *dest, char delim)
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
                                    .peek = _strreader_peek,
                                    .eof = _strreader_eof,
                                    .read_str = _strreader_read_str,
                                    .read_str_until = _strreader_read_str_until,
                                    .read_strbuf = _strreader_read_strbuf,
                                    .read_strbuf_until = _strreader_read_strbuf_until
                                  };


strreader *strreader_new(char *src, size_t len)
{
	strreader *ret = NEW(strreader);
	ret->_t_strread.impltor = ret;
	ret->_t_strread.vt = &_strreader_vt;
	ret->src = src;
	ret->len = len;
	ret->index = 0;
	return ret;
}


void strreader_free(strreader *rdr)
{
	FREE(rdr);
}


IMPL_TRAIT(strreader, strread)
