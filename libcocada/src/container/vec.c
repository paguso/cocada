/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Founvtion; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Founvtion,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "arrays.h"
#include "bitbyte.h"
#include "iter.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"
#include "order.h"
#include "randutil.h"
#include "trait.h"
#include "vec.h"

const static usize MIN_CAPACITY = 4; // (!) MIN_CAPACITY > 1
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2
const static float  MIN_LOAD = 0.5;   // (!) GROW_BY*MIN_LOAD < 1

struct _Vec {
	void *data;
	usize typesize;
	usize len;
	usize capacity;
};


usize vec_memsize(Vec *self)
{
	return sizeof(struct _Vec) + (self->capacity * self->typesize);
}



Vec *vec_new(usize typesize)
{
	return vec_new_with_capacity(typesize, MIN_CAPACITY);
}


usize vec_sizeof()
{
	return sizeof(struct  _Vec);
}


Vec *vec_new_with_capacity(usize typesize, usize init_capacity)
{
	Vec *ret;
	ret = NEW(Vec);
	ret->typesize = typesize;
	ret->capacity = MAX(MIN_CAPACITY, init_capacity);
	ret->len = 0;
	ret->data = malloc((ret->capacity + 1) *
	                   ret->typesize); // +1 position used for swap
	return ret;
}



Vec *vec_new_from_arr(void *buf, usize len, usize typesize)
{
	Vec *ret = NEW(Vec);
	ret->typesize = typesize;
	ret->len = len;
	ret->data = buf;
	ret->capacity = ret->len;
	ret->data = realloc(ret->data, (ret->capacity + 1) * ret->typesize);
	return ret;
}


Vec *vec_new_from_arr_cpy(const void *buf, usize len, usize typesize)
{
	Vec *ret = NEW(Vec);
	ret->typesize = typesize;
	ret->len = len;
	ret->capacity = ret->len;
	ret->data = malloc((ret->capacity + 1) * ret->typesize);
	memcpy(ret->data, buf, ret->len * ret->typesize);
	return ret;
}


void vec_fit(Vec *v)
{
	v->capacity = v->len;
	v->data = realloc(v->data, (v->capacity + 1) * v->typesize);
}



static void _reusizeo(Vec *v, usize cap)
{
	v->capacity = MAX3(MIN_CAPACITY, v->len, cap);
	v->data = realloc(v->data, (v->capacity + 1) * v->typesize);
}


static void _check_and_resize(Vec *v)
{
	if (v->len == v->capacity) {
		_reusizeo(v, GROW_BY * v->capacity);
	}
	else if (v->capacity > MIN_CAPACITY && v->len < MIN_LOAD * v->capacity) {
		_reusizeo(v, v->len / MIN_LOAD);
	}
}


void vec_finalise(void *ptr, const Finaliser *fnr )
{
	Vec *v = (Vec *)ptr;
	if (finaliser_nchd(fnr)) {
		const Finaliser *chd_fr = finaliser_chd(fnr, 0);
		for (usize i = 0, l = vec_len(v); i < l; i++) {
			void *chd =  vec_get_mut(v, i);
			FINALISE(chd, chd_fr);
		}
	}
	FREE(v->data);
}


usize vec_len(const Vec *v)
{
	return v->len;
}


usize vec_typesize(const Vec *v)
{
	return v->typesize;
}


void vec_clear(Vec *v)
{
	v->len = 0;
}


const void *vec_as_array(Vec *v)
{
	return v->data;
}


void *vec_detach(Vec *v)
{
	vec_fit(v);
	void *data = realloc(v->data, v->len * v->typesize);
	FREE(v);
	return data;
}


const void *vec_get(const Vec *v, usize pos)
{
	return v->data + ( pos * v->typesize );
}


const void *vec_first(const Vec *v)
{
	return (v->len) ? vec_get(v, 0) : NULL;
}


const void *vec_last(const Vec *v)
{
	return (v->len) ? vec_get(v, v->len - 1) : NULL;
}


void *vec_get_mut(const Vec *v, usize pos)
{
	return v->data + ( pos * v->typesize );
}


void *vec_first_mut(const Vec *v)
{
	return (v->len) ? vec_get_mut(v, 0) : NULL;
}


void *vec_last_mut(const Vec *v)
{
	return (v->len) ? vec_get_mut(v, v->len - 1) : NULL;
}


void vec_get_cpy(const Vec *v, usize pos, void *dest)
{
	memcpy(dest, v->data + (pos * v->typesize), v->typesize);
}


void vec_set(Vec *v, usize pos, const void *src)
{
	_check_and_resize(v);
	memcpy(v->data + (pos * v->typesize), src, v->typesize);
}


void vec_swap(Vec *v, usize i, usize j)
{
	void *swp = v->data + (v->capacity * v->typesize);
	if (i == j) return;
	memcpy(swp, v->data + (i * v->typesize), v->typesize);
	memcpy(v->data + (i * v->typesize), v->data + (j * v->typesize), v->typesize);
	memcpy(v->data + (j * v->typesize), swp, v->typesize);
}


void vec_push(Vec *v, const void *src)
{
	_check_and_resize(v);
	memcpy(v->data + (v->len * v->typesize), src, v->typesize);
	v->len++;
}


void vec_push_n(Vec *v, const void *src, usize n)
{
	if (n == 0) return;
	_reusizeo(v, v->len + n);
	void *begin = v->data + (v->len * v->typesize);
	void *end = begin;
	memcpy(begin, src, v->typesize);
	end += v->typesize;
	usize k;
	for (k = 1; 2 * k <= n; k *= 2 ) {
		memcpy(end, begin, k * v->typesize);
		end += (k * v->typesize);
	}
	memcpy(end, begin, (n - k) * v->typesize);
	v->len += n;
}


void vec_ins(Vec *v, usize pos, const void *src)
{
	_check_and_resize(v);
	pos = MIN(pos, v->len);
	memmove( v->data + ((pos + 1) * v->typesize), v->data + (pos * v->typesize),
	         (v->len - pos) * v->typesize );
	memcpy(v->data + (pos * v->typesize), src, v->typesize);
	v->len++;
}


void vec_cat(Vec *dest, const Vec *src)
{
	_reusizeo(dest, dest->len + src->len);
	memcpy(dest->data + (dest->len * dest->typesize), src->data,
	       src->len * src->typesize);
	dest->len += src->len;
}


void vec_pop(Vec *v, usize pos, void *dest)
{
	vec_get_cpy(v, pos, dest);
	memmove( v->data + (pos * v->typesize), v->data + ((pos + 1) * v->typesize),
	         (v->len - pos - 1) * v->typesize );
	v->len--;
	_check_and_resize(v);
}


void vec_del(Vec *v, usize pos)
{
	memmove( v->data + (pos * v->typesize), v->data + ((pos + 1) * v->typesize),
	         (v->len - pos - 1) * v->typesize );
	v->len--;
	_check_and_resize(v);
}


void vec_clip(Vec *v, usize from, usize to)
{
	memmove( v->data, v->data + (from * v->typesize), (to - from) * v->typesize );
	v->len = (to - from);
}


void vec_reverse(Vec *v)
{
	usize l = 0, r = v->len - 1;
	while (l < r) {
		vec_swap(v, l++, r--);
	}
}


void vec_rotate_left(Vec *v, usize npos)
{
	if (v->len == 0 || npos % v->len == 0) return;
	npos = npos % v->len;
	void *buf = (void *) ( ARR_NEW(byte, npos * v->typesize ) );
	memcpy(buf, v->data, npos * v->typesize);
	memmove(v->data, v->data + (npos * v->typesize), (v->len - npos) * v->typesize);
	memcpy(v->data + ((v->len - npos) * v->typesize), buf, npos * v->typesize);
	FREE(buf);
}


void vec_rotate_right(Vec *v, usize npos)
{
	vec_rotate_left(v, v->len - (npos % v->len));
}



usize vec_find(const Vec *v, const void *val, EqFunc eq)
{
	usize i, l;
	for (i = 0, l = vec_len(v); i < l && !eq(val, vec_get(v, i)); i++);
	return i;
}


// returns the first position i s.t. val <= v[i] if any; else vec_len(v)
static usize _first_geq_bsearch(const Vec *v, const void *val, CmpFunc cmp)
{
	if (vec_len(v) == 0 ) {
		return 0;
	}
	else if (cmp(val, vec_first(v)) <= 0) {
		return 0;
	}
	else if (cmp(vec_last(v), val) < 0) {
		return vec_len(v);
	}
	else {
		usize l = 0, r = vec_len(v) - 1;
		while (r - l > 1) { // l < ans <= r
			usize m = (l + r) / 2;
			if (cmp(vec_get(v, m), val) < 0) {
				l = m;
			}
			else {
				r = m;
			}
		}
		return r;
	}
}


usize vec_bsearch(const Vec *v, const void *val, CmpFunc cmp)
{
	usize fgeq = _first_geq_bsearch(v, val, cmp);
	if ( ( fgeq < vec_len(v) ) && (cmp(vec_get(v, fgeq), val) == 0) ) {
		return fgeq;
	}
	else {
		return vec_len(v);
	}
}



void vec_qsort(Vec *v, CmpFunc cmp)
{
	qsort(v->data, v->len, v->typesize, cmp);
	//_qsort(v, 0, vec_len(v), cmp);
}


usize vec_min(const Vec *v, CmpFunc cmp)
{
	usize m = 0;
	for (usize i = 1, l = vec_len(v); i < l; ++i) {
		m = ( cmp(vec_get(v, i), vec_get(v, m)) < 0 ) ? i : m;
	}
	return m;
}


usize vec_max(const Vec *v, CmpFunc cmp)
{
	usize m = 0;
	for (usize i = 1, l = vec_len(v); i < l; ++i) {
		m = ( cmp(vec_get(v, i), vec_get(v, m)) > 0 ) ? i : m;
	}
	return m;
}


void vec_radixsort(Vec *v, usize (*key_fn)(const void *, usize),
                   usize key_size, usize max_key)
{
	usize n = vec_len(v);
	void *vcpy = malloc(n * (v->typesize));
	usize *count = ARR_NEW(usize, max_key);
	usize i, k;
	for (usize d = 0; d < key_size; d++) {
		memset(count, 0, max_key * sizeof(usize));
		for (i = 0; i < n; i++) {
			k = key_fn(vec_get(v, i), d);
			count[k]++;
		}
		for (k = 1; k < max_key; k++)
			count[k] += count[k - 1];
		for (i = n; i > 0; i--) {
			k = key_fn(vec_get(v, i - 1), d);
			count[k]--;
			vec_get_cpy(v, i - 1, vcpy + (count[k]*v->typesize));
		}
		memcpy(v->data, vcpy, n * (v->typesize));
	}
	FREE(vcpy);
	FREE(count);
}


#define VEC_NEW_IMPL( TYPE ) \
	Vec *vec_new_##TYPE() \
	{ return vec_new(sizeof(TYPE)); }


#define VEC_GET_IMPL( TYPE ) \
	TYPE vec_get_##TYPE(const Vec *v, usize pos)\
	{ return ((TYPE *)v->data)[pos]; }


#define VEC_FIRST_IMPL( TYPE ) \
	TYPE vec_first_##TYPE(const Vec *v)\
	{ return ((TYPE *)v->data)[0]; }


#define VEC_LAST_IMPL( TYPE ) \
	TYPE vec_last_##TYPE(const Vec *v)\
	{ return ((TYPE *)v->data)[v->len - 1]; }



#define VEC_SET_IMPL( TYPE ) \
	void vec_set_##TYPE(Vec *v, usize pos, TYPE val)\
	{\
		((TYPE *)v->data)[pos] = val;\
	}


#define VEC_PUSH_IMPL( TYPE ) \
	void vec_push_##TYPE(Vec *v, TYPE val)\
	{\
		_check_and_resize(v);\
		((TYPE *)v->data)[v->len++] = val;\
	}


#define VEC_INS_IMPL( TYPE ) \
	void vec_ins_##TYPE(Vec *v, usize pos, TYPE val)\
	{\
		vec_ins(v, pos, &val);\
	}


#define VEC_POP_IMPL( TYPE ) \
	TYPE vec_pop_##TYPE(Vec *v, usize pos)\
	{\
		TYPE r;\
		vec_pop(v, pos, &r);\
		return r;\
	}


#define TYPED_VEC_IMPL( TYPE , ...)\
	VEC_NEW_IMPL(TYPE) \
	VEC_GET_IMPL(TYPE)\
	VEC_FIRST_IMPL(TYPE)\
	VEC_LAST_IMPL(TYPE)\
	VEC_SET_IMPL(TYPE)\
	VEC_PUSH_IMPL(TYPE)\
	VEC_INS_IMPL(TYPE)\
	VEC_POP_IMPL(TYPE)


XX_CORETYPES(TYPED_VEC_IMPL)



struct _VecIter {
	Iter _t_Iter;
	const Vec *src;
	usize index;
};


static bool _vec_iter_has_next(Iter *it)
{
	VecIter *vit = (VecIter *)it->impltor;
	return vit->index < vec_len(vit->src);
}


static const void *_vec_iter_next(Iter *it)
{
	VecIter *vit = (VecIter *)it->impltor;
	return vec_get(vit->src, vit->index++);
}


static Iter_vt _vec_iter_vt = {_vec_iter_has_next, _vec_iter_next};


VecIter *vec_get_iter(const Vec *v)
{
	VecIter *ret = NEW(VecIter);
	ret->_t_Iter.impltor = ret;
	ret->_t_Iter.vt = &_vec_iter_vt;
	ret->src = v;
	ret->index = 0;
	return ret;
}

IMPL_TRAIT(VecIter, Iter)
