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


static void check_and_resize_by(xstr *xs, size_t n)
{
	size_t new_cap = MAX(MIN_CAP, xs->cap);
	while (new_cap - xs->len < n + 1) {
		new_cap *= GROWBY;
	}
	if (new_cap != xs->cap) {
		xs->buf = realloc(xs->buf, new_cap * xs->sizeof_char);
		memset( xs->buf + (xs->cap * xs->sizeof_char), '\0',
		        (new_cap - xs->cap) * xs->sizeof_char );
		xs->cap = new_cap;
	}
}


xstr *xstr_new(size_t sizeof_char)
{
	return xstr_new_with_capacity(sizeof_char, MIN_CAP);
}


xstr *xstr_new_with_capacity(size_t sizeof_char, size_t cap)
{
	assert(sizeof_char<=XCHAR_BYTESIZE);
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



void xstr_free(xstr *xs)
{
	if (xs==NULL) return;
	FREE(xs->buf);
	FREE(xs);
}


void xstr_print(FILE *stream, const xstr *xs)
{
	fprintf(stream, "xstr@%p {\n", xs);
	fprintf(stream, "  len : %zu\n", xs->len );
	fprintf(stream, "  cap : %zu\n", xs->cap );
	fprintf(stream, "  sizeof_char: %zu\n", xs->sizeof_char );
	fprintf(stream, "  str: ");
	for (size_t i=0, l=xs->len; i < l; i++) {
		fprintf(stream, XCHAR_FMT"%s", xstr_get(xs, i), (i < l-1) ? "-" : "");
	}
	fprintf(stream, "}\n");
}


void xstr_to_string (const xstr *xs, strbuf *dest)
{
	for (size_t i=0, l=xstr_len(xs); i < l; i++) {
		if (i) strbuf_append_char(dest, '-');
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
			strbuf_append(dest, d, strlen(d));
		}
	}
}


xchar_t xstr_get(const xstr *xs, size_t pos)
{
	xchar_t ret = 0;
#if ENDIANNESS==LITTLE
	memcpy(&ret, xs->buf + (pos * xs->sizeof_char), xs->sizeof_char);
#elif ENDIANNESS==BIG
	memcpy(&ret + (XCHAR_BYTESIZE - xs->sizeof_char),
	       xs->buf + (pos * xs->sizeof_char), xs->sizeof_char);
#endif
	return ret;
}


void xstr_set(xstr *xs, size_t pos, xchar_t val)
{
#if ENDIANNESS==LITTLE
	memcpy(xs->buf + (pos * xs->sizeof_char), &val, xs->sizeof_char);
#elif ENDIANNESS==BIG
	memcpy(xs->buf + (pos * xs->sizeof_char),
	       &val + (XCHAR_BYTESIZE - xs->sizeof_char));
#endif
}


void xstr_nset(xstr *xs, size_t n, xchar_t val)
{
	size_t l = MIN(xstr_len(xs), n);
	for (size_t i=0; i<l; i++) {
		xstr_set(xs, i, val);
	}
	if (n > l) {
		xstr_push_n(xs, val, n-l);
	}
}


inline size_t xstr_len(const xstr *xs)
{
	return xs->len;
}


inline size_t xstr_sizeof_char(const xstr *xs)
{
	return xs->sizeof_char;
}


void xstr_push(xstr *xs, xchar_t c)
{
	xstr_push_n(xs, c, 1);
}


void xstr_push_n(xstr *xs, xchar_t c, size_t n)
{
	check_and_resize_by(xs, n);
	void *last = xs->buf + (xs->len * xs->sizeof_char);
	for (size_t i=0; i<n; i++, last += xs->sizeof_char) {
#if ENDIANNESS==LITTLE
		memcpy(last, &c, xs->sizeof_char);
#elif ENDIANNESS==BIG
		memcpy(last, &c + (XCHAR_BYTESIZE - xs->sizeof_char), xs->sizeof_char);
#endif
	}
	xs->len += n;
}


void xstr_cat(xstr *dest, const xstr *src)
{
	check_and_resize_by(dest, src->len);
	memcpy(	dest->buf + (dest->len * dest->sizeof_char), src->buf,
	        src->len * src->sizeof_char );
	dest->len += src->len;
}


void xstr_cpy(xstr *dest, const xstr *src)
{
	xstr_ncpy(dest, 0, src, 0, xstr_len(src));
}


void xstr_ncpy( xstr *dest, size_t from_dest, const xstr *src,
                size_t from_src, size_t n )
{
	size_t l = MIN(n, xstr_len(dest)-from_dest);
	for (size_t i=0; i < l; i++ ) {
		xstr_set(dest, from_dest + i, xstr_get(src, from_src + i));
	}
	for (size_t i=l; i<n; i++) {
		xstr_push(dest, xstr_get(src, from_src + i));
	}
}


void xstr_fit(xstr *xs)
{
	xs->cap = MAX(xs->len + 1, MIN_CAP);
	xs->buf = realloc(xs->buf, xs->cap * xs->sizeof_char);
}


void xstr_clip(xstr *xs, size_t from, size_t to)
{
	memmove( xs->buf, xs->buf + (from * xs->sizeof_char),
	         (to - from) * xs->sizeof_char );
	memset( xs->buf + ((to - from) * xs->sizeof_char), '\0',
	        (xs->len - (to - from)) * xs->sizeof_char );
	xs->len = to - from;
}


void xstr_rot_left(xstr *xs, size_t npos)
{
	size_t h = npos % xs->len;
	size_t t = xs->len - h;
	if (h) {
		void *tmp = malloc(h * xs->sizeof_char);
		memcpy(tmp, xs->buf, h * xs->sizeof_char);
		memcpy(xs->buf, xs->buf + (h * xs->sizeof_char), t * xs->sizeof_char);
		memcpy(xs->buf + (t * xs->sizeof_char), tmp, h * xs->sizeof_char);
		FREE(tmp);
	}
}


void xstr_clear(xstr *xs)
{
	memset(xs->buf, '\0', xs->len * xs->sizeof_char);
	xs->len = 0;
}


void *xstr_detach(xstr *xs)
{
	xstr_fit(xs);
	void *ret = xs->buf;
	FREE(xs);
	return ret;
}


int xstr_ncmp(const xstr *this, const xstr *other, size_t n)
{
	size_t lt = xstr_len(this);
	size_t lo = xstr_len(other);
	size_t m = MIN3(n, lt, lo);
	intmax_t cmp;
	for (size_t i=0; i<m;  i++) {
		cmp = (intmax_t)xstr_get(this, i) - (intmax_t)xstr_get(other, i);
		if (cmp) return cmp/abs(cmp);
	}
	if (n <= MIN(lt, lo)) return 0;
	else {
		if (lt == lo) return 0;
		else if (lt < lo) return -1;
		else return +1;
	}
}


int xstr_cmp(const xstr *this, const xstr *other)
{
	return xstr_ncmp(this, other, MAX(xstr_len(this), xstr_len(other)));
}
