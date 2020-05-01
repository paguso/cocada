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
#include "arrutil.h"
#include "bitbyte.h"
#include "new.h"
#include "mathutil.h"
#include "vec.h"
#include "xstr.h"


struct _xstr {
	vec *buf;
};


xstr *xstr_new(size_t sizeof_char)
{
	assert(sizeof_char<=XCHAR_BYTESIZE);
	xstr *ret = NEW(xstr);
	ret->buf = vec_new(sizeof_char);
	return ret;
}


xstr *xstr_new_with_capacity(size_t sizeof_char, size_t cap)
{
	assert(sizeof_char<=XCHAR_BYTESIZE);
	xstr *ret = NEW(xstr);
	ret->buf = vec_new_with_capacity(sizeof_char, cap);
	return ret;
}


xstr *xstr_new_from_arr(void *src, size_t len, size_t sizeof_char)
{
	xstr *ret = NEW(xstr);
	ret->buf = vec_new_from_arr(src, len, sizeof_char);
	return ret;
}


xstr *xstr_new_from_arr_cpy(const void *src, size_t len, size_t sizeof_char)
{
	xstr *ret = NEW(xstr);
	ret->buf = vec_new_from_arr_cpy(src, len, sizeof_char);
	return ret;
}


void xstr_free(xstr *xs)
{
	if (xs==NULL) return;
	FREE(xs->buf);
	FREE(xs);
}


void xstr_print(const xstr *xs)
{
	printf("xstr@%p {\n", xs);
	printf("  len : %zu\n", vec_len(xs->buf));
	printf("  sizeof_char: %zu\n", vec_typesize(xs->buf));
	printf("  str: ");
	for (size_t i=0, l=vec_len(xs->buf); i < l; i++) {
		printf(XCHAR_FMT"%s", xstr_get(xs, i), (i < l-1) ? "-" : "");
	}
	printf("} # end of xstr@%p\n", xs);
}


void xstr_to_string (const xstr *xs, strbuf *dest)
{
	for (size_t i=0, l=xstr_len(xs); i < l; i++) {
		if (i) strbuf_append(dest, "-");
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
			strbuf_append(dest, d);
		}
	}
}


xchar_t xstr_get(const xstr *xs, size_t pos)
{
	xchar_t ret = 0;
#if ENDIANNESS==LITTLE
	memcpy(&ret, vec_get(xs->buf, pos), xstr_sizeof_char(xs));
#elif ENDIANNESS==BIG
	memcpy((&ret + (XCHAR_BYTESIZE - xstr_sizeof_char(xs)), vec_get(xs->buf, pos), xstr_sizeof_char(xs));
#endif
	return ret;
}


void xstr_set(xstr *xs, size_t pos, xchar_t val)
{
#if ENDIANNESS==LITTLE
	vec_set(xs->buf, pos, &val);
#elif ENDIANNESS==BIG
	vec_set(xs->buf, pos, &val + (XCHAR_BYTESIZE - xstr_sizeof_char(xs)));
#endif
}


void xstr_nset(xstr *xs, size_t n, xchar_t val)
{
	size_t l = MIN(vec_len(xs->buf), n);
	for (size_t i=0; i<l; i++) {
		xstr_set(xs, i, val);
	}
	if (n > l) {
		xstr_push_n(xs, val, n-l);
	}
}


size_t xstr_len(const xstr *xs)
{
	return vec_len(xs->buf);
}


size_t xstr_sizeof_char(const xstr *xs)
{
	return vec_typesize(xs->buf);
}


void xstr_push(xstr *xs, xchar_t c)
{
#if ENDIANNESS==LITTLE
	vec_push(xs->buf, &c);
#elif ENDIANNESS==BIG
	vec_push(xs->buf, &c + (XCHAR_BYTESIZE - xstr_sizeof_char(xs)));
#endif
}


void xstr_push_n(xstr *xs, xchar_t c, size_t n)
{
#if ENDIANNESS==LITTLE
	vec_push_n(xs->buf, &c, n);
#elif ENDIANNESS==BIG
	vec_push_n(xs->buf, &c + (XCHAR_BYTESIZE - xstr_sizeof_char(xs)), n);
#endif
}


void xstr_cat(xstr *dest, const xstr *src)
{
	vec_cat(dest->buf, src->buf);
}


void xstr_cpy(xstr *dest, const xstr *src)
{
	xstr_ncpy(dest, 0, src, 0, xstr_len(src));
}


void xstr_ncpy( xstr *dest, size_t from_dest, const xstr *src,
                size_t from_src, size_t n )
{
	for (size_t i=0; i < n; i++ ) {
		xstr_set(dest, from_dest + i, xstr_get(src, from_src + i));
	}
}


void xstr_fit(xstr *xs)
{
	vec_fit(xs->buf);
}


void xstr_clip(xstr *xs, size_t from, size_t to)
{
	vec_clip(xs->buf, from, to);
}


void xstr_rot_left(xstr *xs, size_t npos)
{
	vec_rotate_left(xs->buf, npos);
}


void xstr_clear(xstr *xs)
{
	vec_clear(xs->buf);
}


void *xstr_detach(xstr *xs)
{
	void *ret = vec_detach(xs->buf);
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
