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
#include "reader.h"
#include "strreader.h"

struct _StrReader {
	Reader _t_Reader;
	char *src;
	usize len;
	usize index;
};


static void _strreader_reset(Reader *t)
{
	((StrReader *)(t->impltor))->index = 0;
}


static int _strreader_getc(Reader *t)
{
	StrReader *rdr = (StrReader *) t->impltor;
	if (rdr->index < rdr->len) {
		return rdr->src[rdr->index++];
	}
	else {
		return EOF;
	}
}


static int _strreader_ungetc(Reader *t)
{
	StrReader *rdr = (StrReader *) t->impltor;
	if (0 < rdr->index && rdr->index <= rdr->len) {
		rdr->index--;
		return 1;
	}
	else {
		return 0;
	}
}


static usize _strreader_read_str(Reader *t, char *dest, usize n)
{
	StrReader *rdr = (StrReader *) t->impltor;
	usize r = MIN(n, rdr->len - rdr->index);
	strncpy(dest, rdr->src + rdr->index, r);
	rdr->index += r;
	return r;
}


static usize _strreader_read_str_until(Reader *t, char *dest, char delim)
{
	StrReader *rdr = (StrReader *) t->impltor;
	usize i, j;
	for (i = rdr->index, j = 0; i < rdr->len && rdr->src[i] != delim; i++) {
		dest[j++] = rdr->src[i];
	}
	dest[j] = '\0';
	rdr->index = i;
	return j;
}


static Reader_vt _strreader_vt = { .reset = _strreader_reset,
                                   .getc = _strreader_getc,
                                   .ungetc = _strreader_ungetc,
                                   .read_str = _strreader_read_str,
                                   .read_str_until = _strreader_read_str_until
                                 };


StrReader *strreader_new(char *src, usize len)
{
	StrReader *ret = NEW(StrReader);
	ret->_t_Reader.impltor = ret;
	ret->_t_Reader.vt = &_strreader_vt;
	ret->src = src;
	ret->len = len;
	ret->index = 0;
	return ret;
}


void strreader_free(StrReader *rdr)
{
	FREE(rdr);
}


IMPL_TRAIT(StrReader, Reader)
