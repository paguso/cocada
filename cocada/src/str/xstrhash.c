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
#include <stdint.h>

#include "alphabet.h"
#include "arrays.h"
#include "math.h"
#include "new.h"
#include "xstr.h"
#include "xstrhash.h"

struct _xstrhash {
	alphabet *ab;
	size_t max_exp;
	uint64_t *pow;
};


static void _initpow(xstrhash *self)
{
	uint64_t base = ab_size(self->ab);
	size_t e = 0;
	uint64_t p = 1;
	while ( (uint64_t)(base*p) > p ) {
		e++;
		p *= base;
	}
	self->max_exp = e;
	self->pow = ARR_NEW(uint64_t, e);
	p = 1;
	for (size_t i=0; i<e; i++) {
		self->pow[i] = p;
		p *= base;
	}
}

static inline uint64_t _pow(const xstrhash *self, size_t exp)
{
	uint64_t ret = 1;
	while (exp > self->max_exp) {
		ret *= self->pow[self->max_exp];
		exp -= self->max_exp;
	}
	return ret * self->pow[exp];
}


xstrhash *xstrhash_new(alphabet *ab)
{
	xstrhash *ret = NEW(xstrhash);
	ret->ab = ab;
	_initpow(ret);
	return ret;
}


void xstrhash_finalise(void *ptr, const finaliser *fnr)
{
	xstrhash *self = (xstrhash *)ptr;
	DESTROY_FLAT(self->ab, alphabet);
}


uint64_t xstrhash_lex(const xstrhash *self, const xstr *s)
{
	return xstrhash_lex_sub(self, s, 0, xstr_len(s));
}


uint64_t xstrhash_lex_sub(const xstrhash *self, const xstr *s, size_t from,
                          size_t to)
{
	uint64_t hash = 0;
	for (size_t i=from; i < to; i++) {
		hash *= ab_size(self->ab);
		hash += ab_rank(self->ab, xstr_get(s, i));
	}
	return hash;
}


uint64_t xstrhash_roll_lex(const xstrhash *self, const xstr *s, uint64_t hash,
                           xchar_t c)
{
	hash -= _pow(self, xstr_len(s)-1) * ab_rank(self->ab, xstr_get(s, 0));
	hash += ab_rank(self->ab, c);
	return hash;
}


uint64_t xstrhash_roll_lex_sub(const xstrhash *self, const xstr *s, size_t from,
                               size_t to,  uint64_t hash, xchar_t c)
{
	hash -= _pow(self, to - from - 1) * ab_rank(self->ab, xstr_get(s, from));
	hash *= ab_size(self->ab);
	hash += ab_rank(self->ab, c);
	return hash;
}