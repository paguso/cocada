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
#include <stddef.h>

#include "errlog.h"
#include "kll.h"
#include "mathutil.h"
#include "new.h"
#include "order.h"
#include "randutil.h"
#include "vec.h"


struct __kllsumm {
	double err;
	double c;
	double k;
	vec *coins;
	vec *buffs;
	size_t typesize;
	cmp_func cmp;
	double k_const;
	size_t npts;
	size_t cap;
	finaliser *chd_fr;
};



kllsumm *kll_new(size_t typesize, cmp_func cmp, double err)
{
	return kll_new_own(typesize, cmp, err, finaliser_new_empty());
}


kllsumm *kll_new_own(size_t typesize, cmp_func cmp, double err,
                     finaliser *chd_fr)
{
	size_t cap = (size_t) ceil( (1.0/(1.0-KLL_DEFAULT_C)) *
	                            ( KLL_MIN_K_BIG_OH_CONST * (1.0/err) * sqrt(log(1.0/err)) ) );
	return kll_new_own_with_cap(typesize, cmp, err, cap, chd_fr);
}


kllsumm *kll_new_with_cap(size_t typesize, cmp_func cmp, double err,
                          size_t capacity)
{
	return kll_new_own_with_cap(typesize, cmp, err, capacity,
	                            finaliser_new_empty());
}



kllsumm *kll_new_own_with_cap(size_t typesize, cmp_func cmp, double err,
                              size_t capacity, finaliser *chd_fr)
{
	assert (err > 0);
	kllsumm *ret = NEW(kllsumm);
	ret->typesize = typesize;
	ret->cmp = cmp;
	ret->buffs = vec_new(sizeof(vec *));
	ret->coins = vec_new(sizeof(byte_t));
	vec *buf = vec_new(typesize);
	vec_push_rawptr(ret->buffs, buf);
	vec_push_byte_t(ret->coins, 0);
	ret->npts = 0;
	// adjust parameters
	double cap = (double) MAX(KLL_DEFAULT_CAP, capacity);
	double k = KLL_MIN_K_BIG_OH_CONST * (1.0/err) * sqrt(log(1.0/err));
	ERROR_ASSERT( cap >= 2 * k,
	              "KLL: Insufficient capacity (%zu) to ensure the intended error (%f).\n"
	              "Minimum required capacity is %zu.\n", (size_t)cap, err,  (size_t)(2 * k) );
	double c = KLL_DEFAULT_C;
	if ( (cap * (1-c)) < k ) {
		c = 1 - (k / cap);
	}
	k = cap * (1-c);
	ret->cap = cap;
	ret->err = err;
	ret->k = k;
	ret->k_const = ret->k / ((1.0/err) * sqrt(log(1.0/err)));
	ret->c = c;
	ret->chd_fr = chd_fr;
	return ret;
}


void kll_finalise(void *ptr, const finaliser *fnr)
{
	kllsumm *self = (kllsumm *)ptr;
	DESTROY_FLAT(self->coins, vec);
	if (finaliser_nchd(fnr)) {
		DESTROY(self->buffs, finaliser_cons(FNR(vec),
		                                    finaliser_cons(finaliser_new_ptr(),
		                                            finaliser_cons(FNR(vec), finaliser_chd(fnr, 0)))));
	}
	else {
		DESTROY(self->buffs, finaliser_cons(FNR(vec), FNR(vec)));
	}
}


static inline size_t _nlevels(kllsumm *self)
{
	return vec_len(self->buffs);
}


static inline size_t _cap(kllsumm *self, size_t i)
{
	size_t ret = self->k * pow(self->c, _nlevels(self) - 1 - i);
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
			if (i + 1 < _nlevels(self)) {
				nxtbuf = (vec *)vec_get_rawptr(self->buffs, i + 1);
			}
			else {
				nxtbuf = vec_new(self->typesize);
				vec_push_rawptr(self->buffs, nxtbuf);
				vec_push_byte_t(self->coins, 0);
			}
			byte_t coin =  vec_get_byte_t(self->coins, i);
			if (coin == 0) {
				coin = rand_next() % 2;
				vec_set_byte_t(self->coins, i, coin + 1);
			}
			else {
				vec_set_byte_t(self->coins, i, 0);
			}
			size_t j = coin;
			size_t l = vec_len(buf);
			for (; j < l; j += 2) {
				vec_push(nxtbuf, vec_get(buf, j));
			}
			self->npts -= ( l - ((j - coin) / 2) );
			if (self->chd_fr) {
				for (j = 1 - coin; j < l; j += 2) {
					FINALISE(vec_get_mut(buf, j), self->chd_fr);
				}
			}
			vec_clear(buf);
			vec_fit(buf);
		}
	}
}


void kll_upd(kllsumm *self, void *val)
{
	vec_push(vec_get_rawptr(self->buffs, 0), val);
	self->npts++;
	_compress(self);
}


static size_t _rank(vec *buf, void *val, cmp_func cmp)
{
	if (vec_len(buf)==0 || cmp(vec_first(buf), val) >= 0) {
		return 0;
	}
	else if (cmp(vec_last(buf), val) < 0) {
		return vec_len(buf);
	}
	else {   // return first position >= val
		size_t l = 0, r = vec_len(buf) - 1;
		while (r - l > 1) { // invariant: l < rank <= r
			size_t m = (r + l) / 2;
			if ( cmp(vec_get(buf, m), val) < 0 ) {
				l = m;
			}
			else {
				r = m;
			}
		}
		return r;
	}
}


size_t kll_rank(kllsumm *self, void *val)
{
	uint64_t ret = 0,  pow = 1;
	for (size_t i=0, l=_nlevels(self); i<l; i++) {
		vec *buf = (vec *)vec_get_rawptr(self->buffs, i);
		//vec_qsort(buf, self->cmp);
		uint64_t rk = _rank(buf, val, self->cmp);
		ret += (rk * pow);
		pow *= 2;
	}
	return ret;
}


void kll_print(kllsumm *self, FILE *stream, void (*print_val)(FILE *,
               const void *))
{
	fprintf(stream, "KLL @%p:\n", self);
	fprintf(stream, "\terr = %f\n", self->err);
	fprintf(stream, "\tc = %f\n", self->c);
	fprintf(stream, "\tk = %f\n", self->k);
	fprintf(stream, "\tnpts = %zu\n", self->npts);
	fprintf(stream, "\tcap = %zu\n", self->cap);
	fprintf(stream, "\tk_const = %f\n", self->k_const);

	for (size_t i = 0; i < _nlevels(self); i++) {
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
		fprintf(stream, ") len = %zu (cap=%zu)\n", vec_len(buf), _cap(self, i));
	}
}