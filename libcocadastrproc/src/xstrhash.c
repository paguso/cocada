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

struct _xstrHash {
	Alphabet *ab;
	size_t max_exp;
	uint64 *pow;
};


static void _initpow(xstrHash *self)
{
	uint64 base = alphabet_size(self->ab);
	size_t e = 0;
	uint64 p = 1;
	while ( (uint64)(base * p) > p ) {
		e++;
		p *= base;
	}
	self->max_exp = e;
	self->pow = ARR_NEW(uint64, e);
	p = 1;
	for (size_t i = 0; i < e; i++) {
		self->pow[i] = p;
		p *= base;
	}
}

static inline uint64 _pow(const xstrHash *self, size_t exp)
{
	uint64 ret = 1;
	while (exp > self->max_exp) {
		ret *= self->pow[self->max_exp];
		exp -= self->max_exp;
	}
	return ret * self->pow[exp];
}


xstrHash *xstrhash_new(Alphabet *ab)
{
	xstrHash *ret = NEW(xstrHash);
	ret->ab = ab;
	_initpow(ret);
	return ret;
}


void xstrhash_finalise(void *ptr, const Finaliser *fnr)
{
	xstrHash *self = (xstrHash *)ptr;
	DESTROY_FLAT(self->ab, alphabet);
}


uint64 xstrhash_lex(const xstrHash *self, const xstr *s)
{
	return xstrhash_lex_sub(self, s, 0, xstr_len(s));
}


uint64 xstrhash_lex_sub(const xstrHash *self, const xstr *s, size_t from,
                        size_t to)
{
	uint64 hash = 0;
	for (size_t i = from; i < to; i++) {
		hash *= alphabet_size(self->ab);
		hash += alphabet_rank(self->ab, xstr_get(s, i));
	}
	return hash;
}


uint64 xstrhash_roll_lex(const xstrHash *self, const xstr *s, uint64 hash,
                         xchar c)
{
	hash -= _pow(self, xstr_len(s) - 1) * alphabet_rank(self->ab, xstr_get(s, 0));
	hash += alphabet_rank(self->ab, c);
	return hash;
}


uint64 xstrhash_roll_lex_sub(const xstrHash *self, const xstr *s, size_t from,
                             size_t to,  uint64 hash, xchar c)
{
	hash -= _pow(self, to - from - 1) * alphabet_rank(self->ab, xstr_get(s, from));
	hash *= alphabet_size(self->ab);
	hash += alphabet_rank(self->ab, c);
	return hash;
}
