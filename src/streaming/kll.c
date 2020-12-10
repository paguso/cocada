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
#include <math.h>
#include <stdio.h>

#include "kll.h"
#include "mathutil.h"
#include "new.h"
#include "order.h"
#include "randutil.h"
#include "vec.h"

struct __kllsumm {
	double eps;
	double c;
	double k;
	vec *buffs;
	size_t typesize;
	cmp_func cmp;
};


kllsumm *kll_new(size_t typesize, cmp_func cmp, double eps, double c)
{
	assert (eps > 0);
	assert (0.5 < c && c < 1.0);
	kllsumm *ret = NEW(kllsumm);
	ret->typesize = typesize;
	ret->cmp = cmp;
	ret->eps = eps;
	ret->c = c;
	ret->k = (1.0/eps) * sqrt(log10(1.0/eps));
	ret->buffs = vec_new(sizeof(vec *));
    vec *buf = vec_new(typesize);
	vec_push_rawptr(ret->buffs, buf);
	return ret;
}


static inline size_t _len(kllsumm *self)
{
	return vec_len(self->buffs);
}


static inline size_t _cap(kllsumm *self, size_t i)
{
	size_t ret = self->k * pow(self->c, _len(self) - 1 - i);
	return MAX(2, (size_t)ret);
}


static void _compress(kllsumm *self)
{
	for (size_t i = 0; i < vec_len(self->buffs); i++) {
		size_t cap = _cap(self, i);
		vec *buf = (vec *)vec_get_rawptr(self->buffs, i);
		vec_qsort(buf, self->cmp);
		if (vec_len(buf) > cap) {
			vec *nxtbuf;
			if (i + 1 < _len(self)) {
				nxtbuf = (vec *)vec_get_rawptr(self->buffs, i + 1);
			} else {
				nxtbuf = vec_new(self->typesize);
				vec_push_rawptr(self->buffs, nxtbuf);
			}
			int coin = IS_EVEN(rand_next());
			size_t start = coin ? 0 : 1;
			for (size_t j = start, l = vec_len(buf); j < l; j += 2) {
				vec_push(nxtbuf, vec_get(buf, j));
			}
			vec_clear(buf);
			vec_fit(buf);
		}
	}

}


void kll_upd(kllsumm *self, void *val)
{
	vec_push(vec_get_rawptr(self->buffs, 0), val);
	_compress(self);
}


static size_t _rank(vec *buf, void *val, cmp_func cmp)
{
	if (vec_len(buf)==0 || cmp(vec_first(buf), val) >= 0) {
		return 0;
	} else if (cmp(vec_last(buf), val) < 0) {
		return vec_len(buf);
	} else { // return first position >= val
		size_t l = 0, r = vec_len(buf) - 1;
		while (r - l > 1) { // invariant: l < rank <= r
			size_t m = (r + l) / 2;
			if ( cmp(vec_get(buf, m), val) < 0 ) {
				l = m;
			} else {
				r = m;
			}
		}
		return r;
	}
}


size_t kll_rank(kllsumm *self, void *val)
{
	uint64_t ret = 0,  pow = 1;
	for (size_t i=0, l=_len(self); i<l; i++) {
		vec *buf = vec_get_mut(self->buffs, i);
		//vec_qsort(buf, self->cmp);
		uint64_t rk = _rank(buf, val, self->cmp);
		ret += (rk * pow);
		pow *= 2;
	}
	return ret;
}


void kll_print(kllsumm *self, FILE *stream, void (*print_val)(FILE *, const void *))
{
    fprintf(stream, "KLL @%p:\n", self);
    fprintf(stream, "\teps = %f\n", self->eps);
    fprintf(stream, "\tc = %f\n", self->c);
    fprintf(stream, "\tk = %f\n", self->k);
	for (size_t i = 0; i < _len(self); i++) {
		fprintf(stream, "\tB[%zu] = (", i);
		bool comma = false;
		//FOREACH_IN_ITER(val, void, vec_get_iter((vec*)vec_get(self->buffs, i))) {
		vec *buf = (vec *)vec_get_rawptr(self->buffs, i);
        for (size_t j=0; j<vec_len(buf); j++) {
            const void *val = vec_get(buf, j);
        	fprintf(stream, "%s", (comma)?", ":"");
			print_val(stream, val);
			comma = true;
		}
		fprintf(stream, ") cap=%zu\n", _cap(self, i));
	}
}