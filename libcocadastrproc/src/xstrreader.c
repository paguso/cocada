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
#include <string.h>

#include "cstrutil.h"
#include "mathutil.h"
#include "new.h"
#include "strread.h"
#include "trait.h"
#include "xchar.h"
#include "xstr.h"
#include "xstrread.h"
#include "xstrreader.h"


struct _xstrreader {
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
	}
	else {
		return XEOF;
	}
}

size_t  _xstr_read(xstrread *t, xstr *dest, size_t n)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	xstr *src = (xstr *) rdr->src;
	size_t r = MIN(n, xstr_len(src) - rdr->index);
	if (dest != NULL) {
		xstr_ncpy(dest, 0, src, rdr->index, r);
	}
	rdr->index += r;
	return r;
}


size_t  _xstr_read_until(xstrread *t, xstr *dest, xchar_t delim)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	xstr *src = (xstr *) rdr->src;
	size_t i;
	for (i = rdr->index; i < xstr_len(src) && xstr_get(src, i) != delim; i++);
	size_t r = i - rdr->index;
	if (dest != NULL) {
		xstr_ncpy(src, 0, dest, rdr->index, r);
	}
	rdr->index = i;
	return r;
}


static xstrread_vt _xstr_vt = { .reset = _xstr_reset,
                                .getch = _xstr_getc,
                                .read = _xstr_read,
                                .read_until = _xstr_read_until
                              };


xstrreader *xstrreader_open(xstr *src)
{
	xstrreader *ret = NEW(xstrreader);
	ret->_t_xstrread.impltor = ret;
	ret->_t_xstrread.vt = &_xstr_vt;
	ret->src = src;
	ret->index = 0;
	return ret;
}



static xchar_wt _str_getc(xstrread *t)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	char *src = (char *) rdr->src;
	if (rdr->index < rdr->len) {
		return src[rdr->index++];
	}
	else {
		return XEOF;
	}
}


size_t  _str_read(xstrread *t, xstr *dest, size_t n)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	char *src = (char *) rdr->src;
	size_t i, j, l;
	for (i = rdr->index, j = 0, l = MIN(xstr_len(dest), n); j < l
	        && i < rdr->len ; i++, j++) {
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
	for (i = rdr->index, j = 0, l = xstr_len(dest); j < l && i < rdr->len
	        && (xchar_t) src[i] != delim; i++, j++) {
		xstr_set(dest, j, src[i]);
	}
	for (; i < rdr->len && (xchar_t) src[i] != delim; i++, j++) {
		xstr_push(dest, src[i]);
	}
	while (i < l && src[i] == delim) {
		i--;
	}
	rdr->index = i;
	return j;
}


static xstrread_vt _str_vt = { .reset = _xstr_reset,
                               .getch = _str_getc,
                               .read = _str_read,
                               .read_until = _str_read_until
                             };



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


static void _strread_reset(xstrread *t)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	strread *sr = (strread *) rdr->src;
	strread_reset(sr);
}

static xchar_wt _strread_getc(xstrread *t)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	strread *sr = (strread *) rdr->src;
	return (xchar_wt) strread_getc(sr);
}


size_t  _strread_read(xstrread *t, xstr *dest, size_t n)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	strread *sr = (strread *) rdr->src;
	char *s = cstr_new(n);
	size_t r = strread_read_str(sr, s, n);
	xstr *xs = xstr_new_from_arr(s, r, sizeof(char));
	if (dest != NULL) {
		xstr_ncpy(dest, 0, xs, 0, r);
	}
	xstr_free(xs);
	return r;
}

size_t  _strread_read_until(xstrread *t, xstr *dest, xchar_t delim)
{
	xstrreader *rdr = (xstrreader *) t->impltor;
	strread *sr = (strread *) rdr->src;
	size_t r = 0;
	if (dest != NULL) {
		int c = strread_getc(sr);
		while (c != EOF && c != delim && r < xstr_len(dest)) {
			xstr_set(dest, r, c);
			r++;
			c = strread_getc(sr);
		}
		while (c != EOF && c != delim) {
			xstr_push(dest, c);
			r++;
			c = strread_getc(sr);
		}
		if (c == delim) {
			strread_ungetc(sr);
		}
	}
	else {
		int c = strread_getc(sr);
		while (c != EOF && c != delim) {
			r++;
			c = strread_getc(sr);
		}
		if (c == delim) {
			strread_ungetc(sr);
		}
	}
	return r;
}


static xstrread_vt _strread_vt = { .reset = _strread_reset,
                                   .getch = _strread_getc,
                                   .read = _strread_read,
                                   .read_until = _strread_read_until
                                 };


xstrreader *xstrreader_open_strread(strread *src)
{
	xstrreader *ret = NEW(xstrreader);
	ret->_t_xstrread.impltor = ret;
	ret->_t_xstrread.vt = &_strread_vt;
	ret->src = src;
	ret->index = 0;
	ret->len = 0;
	return ret;
}


void xstrreader_close(xstrreader *self)
{
	FREE(self);
}


IMPL_TRAIT(xstrreader, xstrread)