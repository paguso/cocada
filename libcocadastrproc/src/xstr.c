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

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "alphabet.h"
#include "arrays.h"
#include "bitbyte.h"
#include "errlog.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"
#include "xstr.h"



struct _xstr {
	void *buf;
	size_t len;
	size_t cap;
	size_t sizeof_char;
};


#define MIN_CAP 4
#define GROWBY 1.66f


static void check_and_resize_by(xstr *self, size_t n)
{
	size_t new_cap = MAX(MIN_CAP, self->cap);
	while (new_cap - self->len < n + 1) {
		new_cap *= GROWBY;
	}
	if (new_cap != self->cap) {
		self->buf = realloc(self->buf, new_cap * self->sizeof_char);
		memset( self->buf + (self->cap * self->sizeof_char), '\0',
		        (new_cap - self->cap) * self->sizeof_char );
		self->cap = new_cap;
	}
}


xstr *xstr_new(size_t sizeof_char)
{
	return xstr_new_with_capacity(sizeof_char, MIN_CAP);
}


xstr *xstr_new_with_capacity(size_t sizeof_char, size_t cap)
{
	ERROR_ASSERT(sizeof_char <= XCHAR_BYTES,
	             "Invalid xstr char size %zu. "
				 "This must be at most XCHAR_BYTES=%d. "
				 "See xchar documentation.\n",
	             sizeof_char, XCHAR_BYTES);
	xstr *ret = NEW(xstr);
	ret->sizeof_char = sizeof_char;
	ret->len = 0;
	ret->cap = MAX(MIN_CAP, cap);
	ret->buf = calloc(ret->cap, sizeof_char);
	return ret;
}


xstr *xstr_new_from_arr(void *src, size_t len, size_t sizeof_char)
{
	xstr *ret = NEW(xstr);
	ret->sizeof_char = sizeof_char;
	ret->len = len;
	ret->cap = len;
	ret->buf = realloc(src, len * sizeof_char);
	check_and_resize_by(ret, 0); // add at least one trailing \0
	return ret;
}


xstr *xstr_new_from_arr_cpy(const void *src, size_t len, size_t sizeof_char)
{
	xstr *ret = NEW(xstr);
	ret->sizeof_char = sizeof_char;
	ret->len = len;
	ret->cap = len;
	ret->buf = malloc(len * sizeof_char);
	memcpy(ret->buf, src, len * sizeof_char);
	check_and_resize_by(ret, 0);
	return ret;
}



void xstr_free(xstr *self)
{
	if (self==NULL) return;
	FREE(self->buf);
	FREE(self);
}


void xstr_to_string (const xstr *self, strbuf *dest)
{
	for (size_t i=0, l=xstr_len(self); i < l; i++) {
		if (i) strbuf_append_char(dest, '-');
		xchar_t c = xstr_get(self, i);
		size_t  n = c;
		size_t  ord = 1;
		for (; n>=10; n/=10) ord*=10;
		char d[2] = "\0\0";
		n = c;
		while (ord) {
			d[0] = '0' + (n/ord);
			n = n % ord;
			ord /= 10;
			strbuf_nappend(dest, d, strlen(d));
		}
	}
}


xchar_t xstr_get(const xstr *self, size_t pos)
{
	xchar_t ret = 0;
#if ENDIANNESS==LITTLE
	memcpy(&ret, self->buf + (pos * self->sizeof_char), self->sizeof_char);
#elif ENDIANNESS==BIG
	memcpy(&ret + (XCHAR_BYTES - self->sizeof_char),
	       self->buf + (pos * self->sizeof_char), self->sizeof_char);
#endif
	return ret;
}


void xstr_set(xstr *self, size_t pos, xchar_t val)
{
#if ENDIANNESS==LITTLE
	memcpy(self->buf + (pos * self->sizeof_char), &val, self->sizeof_char);
#elif ENDIANNESS==BIG
	memcpy(self->buf + (pos * self->sizeof_char),
	       &val + (XCHAR_BYTES - self->sizeof_char));
#endif
}


void xstr_nset(xstr *self, size_t n, xchar_t val)
{
	size_t l = MIN(xstr_len(self), n);
	for (size_t i=0; i<l; i++) {
		xstr_set(self, i, val);
	}
	if (n > l) {
		xstr_push_n(self, val, n-l);
	}
}


inline size_t xstr_len(const xstr *self)
{
	return self->len;
}


inline size_t xstr_sizeof_char(const xstr *self)
{
	return self->sizeof_char;
}


const byte_t *xstr_as_bytes(const xstr *self)
{
	return (const byte_t *)(self->buf);
}


size_t xstr_nbytes(const xstr *self)
{
	return self->len * self->sizeof_char;
}


void xstr_push(xstr *self, xchar_t c)
{
	xstr_push_n(self, c, 1);
}


void xstr_push_n(xstr *self, xchar_t c, size_t n)
{
	check_and_resize_by(self, n);
	void *last = self->buf + (self->len * self->sizeof_char);
	for (size_t i=0; i<n; i++, last += self->sizeof_char) {
#if ENDIANNESS==LITTLE
		memcpy(last, &c, self->sizeof_char);
#elif ENDIANNESS==BIG
		memcpy(last, &c + (XCHAR_BYTES - self->sizeof_char), self->sizeof_char);
#endif
	}
	self->len += n;
}


void xstr_cat(xstr *self, const xstr *src)
{
	check_and_resize_by(self, src->len);
	memcpy(	self->buf + (self->len * self->sizeof_char), src->buf,
	        src->len * src->sizeof_char );
	self->len += src->len;
}


void xstr_cpy(xstr *self, const xstr *src)
{
	xstr_ncpy(self, 0, src, 0, xstr_len(src));
}


void xstr_ncpy( xstr *self, size_t from_dest, const xstr *src,
                size_t from_src, size_t n )
{
	size_t l = MIN(n, xstr_len(self)-from_dest);
	for (size_t i=0; i < l; i++ ) {
		xstr_set(self, from_dest + i, xstr_get(src, from_src + i));
	}
	for (size_t i=l; i<n; i++) {
		xstr_push(self, xstr_get(src, from_src + i));
	}
}


void xstr_fit(xstr *self)
{
	self->cap = MAX(self->len + 1, MIN_CAP);
	self->buf = realloc(self->buf, self->cap * self->sizeof_char);
}


void xstr_clip(xstr *self, size_t from, size_t to)
{
	memmove( self->buf, self->buf + (from * self->sizeof_char),
	         (to - from) * self->sizeof_char );
	memset( self->buf + ((to - from) * self->sizeof_char), '\0',
	        (self->len - (to - from)) * self->sizeof_char );
	self->len = to - from;
}


void xstr_rot_left(xstr *self, size_t npos)
{
	size_t h = npos % self->len;
	size_t t = self->len - h;
	if (h) {
		void *tmp = malloc(h * self->sizeof_char);
		memcpy(tmp, self->buf, h * self->sizeof_char);
		memcpy(self->buf, self->buf + (h * self->sizeof_char), t * self->sizeof_char);
		memcpy(self->buf + (t * self->sizeof_char), tmp, h * self->sizeof_char);
		FREE(tmp);
	}
}


void xstr_clear(xstr *self)
{
	memset(self->buf, '\0', self->len * self->sizeof_char);
	self->len = 0;
}


void *xstr_detach(xstr *self)
{
	xstr_fit(self);
	void *ret = self->buf;
	FREE(self);
	return ret;
}


int xstr_ncmp(const xstr *self, const xstr *other, size_t n)
{
	size_t lt = xstr_len(self);
	size_t lo = xstr_len(other);
	size_t m = MIN3(n, lt, lo);
	intmax_t cmp;
	for (size_t i=0; i<m;  i++) {
		cmp = (intmax_t)xstr_get(self, i) - (intmax_t)xstr_get(other, i);
		if (cmp) return cmp/abs(cmp);
	}
	if (n <= MIN(lt, lo)) return 0;
	else {
		if (lt == lo) return 0;
		else if (lt < lo) return -1;
		else return +1;
	}
}


int xstr_cmp(const xstr *self, const xstr *other)
{
	return xstr_ncmp(self, other, MAX(xstr_len(self), xstr_len(other)));
}
