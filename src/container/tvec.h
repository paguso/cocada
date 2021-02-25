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

#ifndef TVEC_H
#define TVEC_H

#include <string.h>
#include <stdio.h>

#include "coretype.h"
#include "iter.h"
#include "new.h"
#include "errlog.h"
#include "order.h"
#include "memdbg.h"
#include "mathutil.h"
#include "arrays.h"

/**
 * @file tvec.h
 * @author Paulo Fonseca
 * @brief Generic type vector
 */


/*----------------------------------------------------------------------------*
 *                         GENERIC TYPE DECLARATION                           *
 *----------------------------------------------------------------------------*/

#define TVEC_DECL(TYPE, ...)                                                   \
	\
	typedef struct _tvec_##TYPE tvec_##TYPE;                                       \
	\
	tvec_##TYPE *tvec_##TYPE##_new();                                              \
	\
	tvec_##TYPE *tvec_##TYPE##_new_with_capacity(size_t init_capacity);            \
	\
	tvec_##TYPE *tvec_##TYPE##_new_from_arr(TYPE *buf, size_t len);                \
	\
	tvec_##TYPE *tvec_##TYPE##_new_from_arr_cpy(const TYPE *buf, size_t len);      \
	\
	void tvec_##TYPE##_fit(tvec_##TYPE *v);                                        \
	\
	void tvec_##TYPE##_finalise(void *ptr, const finaliser *fnr );                           \
	\
	size_t tvec_##TYPE##_len(const tvec_##TYPE *v);                                \
	\
	void tvec_##TYPE##_clear(tvec_##TYPE *v);                                      \
	\
	const TYPE *tvec_##TYPE##_as_array(tvec_##TYPE *v);                            \
	\
	TYPE *tvec_##TYPE##_detach(tvec_##TYPE *v);                                    \
	\
	TYPE tvec_##TYPE##_get(const tvec_##TYPE *v, size_t pos);                      \
	\
	TYPE tvec_##TYPE##_first(const tvec_##TYPE *v);                                \
	\
	TYPE tvec_##TYPE##_last(const tvec_##TYPE *v);                                 \
	\
	const TYPE *tvec_##TYPE##_get_ref(const tvec_##TYPE *v, size_t pos);           \
	\
	const TYPE *tvec_##TYPE##_first_ref(const tvec_##TYPE *v);                     \
	\
	const TYPE *tvec_##TYPE##_last_ref(const tvec_##TYPE *v);                      \
	\
	void tvec_##TYPE##_set(tvec_##TYPE *v, size_t pos, TYPE val);                  \
	\
	void tvec_##TYPE##_swap(tvec_##TYPE *v, size_t i, size_t j);                   \
	\
	void tvec_##TYPE##_push(tvec_##TYPE *v, TYPE val);                             \
	\
	void tvec_##TYPE##_push_cpy(tvec_##TYPE *v, TYPE *val);                        \
	\
	void tvec_##TYPE##_push_n(tvec_##TYPE *v, TYPE val, size_t n);                 \
	\
	void tvec_##TYPE##_ins(tvec_##TYPE *v, size_t pos, TYPE val);                  \
	\
	void tvec_##TYPE##_cat(tvec_##TYPE *dest, const tvec_##TYPE *src);             \
	\
	TYPE tvec_##TYPE##_pop(tvec_##TYPE *v, size_t pos);                            \
	\
	void tvec_##TYPE##_remv(tvec_##TYPE *v, size_t pos);                           \
	\
	void tvec_##TYPE##_clip(tvec_##TYPE *v, size_t from, size_t to);               \
	\
	void tvec_##TYPE##_rotate_left(tvec_##TYPE *v, size_t npos);                   \
	\
	size_t tvec_##TYPE##_find(tvec_##TYPE *v, const TYPE *val, eq_func eq);        \
	\
	void tvec_##TYPE##_qsort(tvec_##TYPE *v, cmp_func cmp);                        \
	\
	size_t tvec_##TYPE##_min(tvec_##TYPE *v, cmp_func cmp);                        \
	\
	size_t tvec_##TYPE##_max(tvec_##TYPE *v, cmp_func cmp);                        \
	\
	typedef struct _tvec_##TYPE##_iter tvec_##TYPE##_iter;                         \
	\
	tvec_##TYPE##_iter *tvec_##TYPE##_get_iter(tvec_##TYPE *v);                    \
	\
	DECL_TRAIT(tvec_##TYPE##_iter, iter)


XX_CORETYPES(TVEC_DECL)


#define TYPEMAP(T, F) tvec_##T *: tvec_##T##_##F,


#define GENFNAME(OBJ, TYPE, FNAME) \
	_Generic(OBJ,\
	         XX_CORETYPES(TYPEMAP, FNAME),\
	         default: __ERROR__)


#define tvec(TYPE) tvec_##TYPE

#define tvec_new(TYPE) tvec_##TYPE##_new()

/**
 * @brief Push a copy of @p val into @p v
 */
#define tvec_push(v, val) GENFNAME(v, tvec, push) (v, val)

#define tvec_get(self,  pos) GENFNAME(self, tvec, get) (self, pos)

#define tvec_get_iter(self)  GENFNAME(self, tvec, get_iter) (self)




// IMPLEMENTATION


#define TVEC_DEFAULT_CAP  16
#define TVEC_GROW_BY      1.6F
#define TVEC_MIN_LOAD     0.5F   // (!) GROW_BY*MIN_LOAD < 1                   

#define TVEC_IMPL(TYPE, ...)                                                   \
	\
	struct _tvec_##TYPE {                                                          \
		TYPE *data;                                                                \
		size_t len;                                                                \
		size_t capacity;                                                           \
	};                                                                             \
	\
	\
	tvec_##TYPE *tvec_##TYPE##_new()                                               \
	{                                                                              \
		return tvec_##TYPE##_new_with_capacity(TVEC_DEFAULT_CAP);                  \
	}                                                                              \
	\
	\
	tvec_##TYPE *tvec_##TYPE##_new_with_capacity(size_t init_capacity)             \
	{                                                                              \
		tvec_##TYPE *ret = NEW(tvec_##TYPE);                                       \
		ret->capacity = MAX(TVEC_DEFAULT_CAP, init_capacity);                      \
		ret->len = 0;                                                              \
		ret->data = malloc(ret->capacity * sizeof(TYPE));                          \
		return ret;                                                                \
	}                                                                              \
	\
	\
	\
	tvec_##TYPE *tvec_##TYPE##_new_from_arr(TYPE *buf, size_t len)                 \
	{                                                                              \
		tvec_##TYPE *ret = NEW(tvec_##TYPE);                                       \
		ret->len = len;                                                            \
		ret->data = buf;                                                           \
		ret->capacity = ret->len;                                                  \
		ret->data = realloc(ret->data, ret->capacity * sizeof(TYPE));              \
		return ret;                                                                \
	}                                                                              \
	\
	\
	tvec_##TYPE *tvec_##TYPE##_new_from_arr_cpy(const TYPE *buf, size_t len)       \
	{                                                                              \
		tvec_##TYPE *ret = NEW(tvec_##TYPE);                                       \
		ret->len = len;                                                            \
		ret->capacity = ret->len;                                                  \
		ret->data = malloc(ret->capacity * sizeof(TYPE));                          \
		memcpy(ret->data, buf, ret->len * sizeof(TYPE));                           \
		return ret;                                                                \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_fit(tvec_##TYPE *v)                                         \
	{                                                                              \
		v->capacity = v->len;                                                      \
		v->data = realloc(v->data, v->capacity * sizeof(TYPE));                    \
	}                                                                              \
	\
	\
	\
	static void _tvec_##TYPE##_resize_to(tvec_##TYPE *v, size_t cap)               \
	{                                                                              \
		v->capacity = MAX3(TVEC_DEFAULT_CAP, v->len, cap);                         \
		v->data = realloc(v->data, v->capacity * sizeof(TYPE));                    \
	}                                                                              \
	\
	\
	static void _tvec_##TYPE##_check_and_resize(tvec_##TYPE *v)                    \
	{                                                                              \
		if (v->len==v->capacity) {                                                 \
			_tvec_##TYPE##_resize_to(v, TVEC_GROW_BY * v->capacity);               \
		}                                                                          \
		else if (v->capacity > TVEC_DEFAULT_CAP &&                                 \
		         v->len < TVEC_MIN_LOAD * v->capacity) {                           \
			_tvec_##TYPE##_resize_to(v, v->len / TVEC_MIN_LOAD);                   \
		}                                                                          \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_finalise(void *ptr, const finaliser *fnr )                            \
	{                                                                              \
		tvec_##TYPE *v = (tvec_##TYPE *)ptr;                                       \
		if (finaliser_nchd(fnr)) {                                                       \
			const finaliser *chd_fr = finaliser_chd(fnr, 0);                                  \
			for (size_t i=0, l=tvec_##TYPE##_len(v); i<l; i++) {                   \
				void *chd =  (void *)tvec_##TYPE##_get_ref(v, i);                  \
				FINALISE(chd, chd_fr);                                             \
			}                                                                      \
		}                                                                          \
		FREE(v->data);                                                             \
	}                                                                              \
	\
	\
	size_t tvec_##TYPE##_len(const tvec_##TYPE *v)                                 \
	{                                                                              \
		return v->len;                                                             \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_clear(tvec_##TYPE *v)                                       \
	{                                                                              \
		v->len = 0;                                                                \
	}                                                                              \
	\
	\
	const TYPE *tvec_##TYPE##_as_array(tvec_##TYPE *v)                             \
	{                                                                              \
		return v->data;                                                            \
	}                                                                              \
	\
	\
	TYPE *tvec_##TYPE##_detach(tvec_##TYPE *v)                                     \
	{                                                                              \
		tvec_##TYPE##_fit(v);                                                      \
		TYPE *data = v->data;                                                      \
		FREE(v);                                                                   \
		return data;                                                               \
	}                                                                              \
	\
	\
	TYPE tvec_##TYPE##_get(const tvec_##TYPE *v, size_t pos)                       \
	{                                                                              \
		WARN_ASSERT( pos < v->len, "Invalid tvec index.n" );                       \
		return v->data[pos];                                                       \
	}                                                                              \
	\
	\
	TYPE tvec_##TYPE##_first(const tvec_##TYPE *v)                                 \
	{                                                                              \
		return tvec_##TYPE##_get(v, 0);                                            \
	}                                                                              \
	\
	\
	TYPE tvec_##TYPE##_last(const tvec_##TYPE *v)                                  \
	{                                                                              \
		return tvec_##TYPE##_get(v, v->len - 1);                                   \
	}                                                                              \
	\
	\
	const TYPE *tvec_##TYPE##_get_ref(const tvec_##TYPE *v, size_t pos)            \
	{                                                                              \
		WARN_ASSERT( pos < v->len, "Invalid tvec index.n" );                       \
		return &(v->data[pos]);                                                    \
	}                                                                              \
	\
	\
	const TYPE *tvec_##TYPE##_first_ref(const tvec_##TYPE *v)                      \
	{                                                                              \
		return tvec_##TYPE##_get_ref(v, 0);                                        \
	}                                                                              \
	\
	\
	const TYPE *tvec_##TYPE##_last_ref(const tvec_##TYPE *v)                       \
	{                                                                              \
		return tvec_##TYPE##_get_ref(v, v->len-1);                                 \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_set(tvec_##TYPE *v, size_t pos, TYPE val)                   \
	{                                                                              \
		ERROR_ASSERT(pos < v->len, "Invalid tvec index.n");                        \
		v->data[pos] = val;                                                        \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_swap(tvec_##TYPE *v, size_t i, size_t j)                    \
	{                                                                              \
		ERROR_ASSERT( i < v->len &&  j < v->len, "Invalid tvec index.n");          \
		if (i==j) return;                                                          \
		TYPE swp = v->data[i];                                                     \
		v->data[i] = v->data[j];                                                   \
		v->data[j] = swp;                                                          \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_push(tvec_##TYPE *v, TYPE val)                              \
	{                                                                              \
		_tvec_##TYPE##_check_and_resize(v);                                        \
		v->data[v->len++] = val;                                                   \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_push_cpy(tvec_##TYPE *v, TYPE *val)                         \
	{                                                                              \
		_tvec_##TYPE##_check_and_resize(v);                                        \
		v->data[v->len++] = *val;                                                  \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_push_n(tvec_##TYPE *v, TYPE val, size_t n)                  \
	{                                                                              \
		if (n == 0) return;                                                        \
		_tvec_##TYPE##_resize_to(v, v->len + n);                                   \
		for (size_t i=0; i<n; i++) {                                               \
			v->data[v->len++] = val;                                               \
		}                                                                          \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_ins(tvec_##TYPE *v, size_t pos, TYPE val)                   \
	{                                                                              \
		ERROR_ASSERT(pos < v->len, "Invalid tvec index.n");                        \
		_tvec_##TYPE##_check_and_resize(v);                                        \
		const size_t typesize = sizeof(TYPE);                                      \
		memmove( v->data+((pos+1)*typesize), v->data+(pos*typesize),               \
		         (v->len-pos)*typesize );                                          \
		v->data[pos] = val;                                                        \
		v->len++;                                                                  \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_cat(tvec_##TYPE *dest, const tvec_##TYPE *src)              \
	{                                                                              \
		_tvec_##TYPE##_resize_to(dest, dest->len + src->len);                      \
		const size_t typesize = sizeof(TYPE);                                      \
		memcpy(dest->data+(dest->len * typesize), src->data, src->len * typesize); \
		dest->len += src->len;                                                     \
	}                                                                              \
	\
	\
	TYPE tvec_##TYPE##_pop(tvec_##TYPE *v, size_t pos)                             \
	{                                                                              \
		ERROR_ASSERT(pos < v->len, "Invalid tvec index.n");                        \
		TYPE ret = v->data[pos];                                                   \
		const size_t typesize = sizeof(TYPE);                                      \
		memmove( v->data+(pos*typesize), v->data+((pos+1)*typesize),               \
		         (v->len-pos-1)*typesize );                                        \
		v->len--;                                                                  \
		_tvec_##TYPE##_check_and_resize(v);                                        \
		return ret;                                                                \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_remv(tvec_##TYPE *v, size_t pos)                            \
	{                                                                              \
		ERROR_ASSERT(pos < v->len, "Invalid tvec index.n");                        \
		const size_t typesize = sizeof(TYPE);                                      \
		memmove( v->data+(pos*typesize), v->data+((pos+1)*typesize),               \
		         (v->len-pos-1)*typesize );                                        \
		v->len--;                                                                  \
		_tvec_##TYPE##_check_and_resize(v);                                        \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_clip(tvec_##TYPE *v, size_t from, size_t to)                \
	{                                                                              \
		ERROR_ASSERT( from <= to && to <= v->len, "Invalid tvec index.n");         \
		const size_t typesize = sizeof(TYPE);                                      \
		memmove( v->data, v->data + (from * typesize), (to - from) * typesize );   \
		v->len = (to - from);                                                      \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_rotate_left(tvec_##TYPE *v, size_t npos)                    \
	{                                                                              \
		if (v->len == 0 || npos % v->len == 0) return;                             \
		npos = npos % v->len;                                                      \
		const size_t typesize = sizeof(TYPE);                                      \
		TYPE *buf =  NEW_ARR(TYPE, npos);                                          \
		memcpy(buf, v->data, npos * typesize);                                     \
		memmove(v->data, v->data + (npos * typesize), (v->len - npos) * typesize); \
		memcpy(v->data + ((v->len - npos) * typesize), buf, npos * typesize);      \
		FREE(buf);                                                                 \
	}                                                                              \
	\
	\
	size_t tvec_##TYPE##_find(tvec_##TYPE *v, const TYPE *val, eq_func eq)         \
	{                                                                              \
		size_t i, l;                                                               \
		for (i=0, l=tvec_##TYPE##_len(v);                                          \
		        i < l && !eq(val, &(v->data[i])); i++);                               \
		return i;                                                                  \
	}                                                                              \
	\
	\
	void tvec_##TYPE##_qsort(tvec_##TYPE *v, cmp_func cmp)                         \
	{                                                                              \
		qsort(v->data, v->len, sizeof(TYPE), cmp);                                 \
	}                                                                              \
	\
	\
	size_t tvec_##TYPE##_min(tvec_##TYPE *v, cmp_func cmp)                         \
	{                                                                              \
		size_t m = 0;                                                              \
		for (size_t i=1, l=v->len; i<l; ++i) {                                     \
			m = ( cmp(v->data + i, v->data + m) < 0 ) ? i : m;                     \
		}                                                                          \
		return m;                                                                  \
	}                                                                              \
	\
	\
	size_t tvec_##TYPE##_max(tvec_##TYPE *v, cmp_func cmp)                         \
	{                                                                              \
		size_t m = 0;                                                              \
		for (size_t i=1, l=v->len; i<l; ++i) {                                     \
			m = ( cmp(v->data + i, v->data + m) > 0 ) ? i : m;                     \
		}                                                                          \
		return m;                                                                  \
	}                                                                              \
	\
	\
	struct _tvec_##TYPE##_iter {                                                   \
		iter _t_iter;                                                              \
		tvec_##TYPE *src;                                                          \
		size_t index;                                                              \
	};                                                                             \
	\
	\
	static bool _tvec_##TYPE##_iter_has_next(iter *it)                             \
	{                                                                              \
		tvec_##TYPE##_iter *vit = (tvec_##TYPE##_iter *)it->impltor;               \
		return vit->index < tvec_##TYPE##_len(vit->src);                           \
	}                                                                              \
	\
	\
	static const void * _tvec_##TYPE##_iter_next(iter *it)                         \
	{                                                                              \
		tvec_##TYPE##_iter *vit = (tvec_##TYPE##_iter *)it->impltor;               \
		return tvec_##TYPE##_get_ref(vit->src, vit->index++);                      \
	}                                                                              \
	\
	\
	static iter_vt _tvec_##TYPE##_iter_vt =                                        \
	        {_tvec_##TYPE##_iter_has_next, _tvec_##TYPE##_iter_next};                  \
	\
	\
	tvec_##TYPE##_iter *tvec_##TYPE##_get_iter(tvec_##TYPE *v)                     \
	{                                                                              \
		tvec_##TYPE##_iter *ret = NEW(tvec_##TYPE##_iter);                         \
		ret->_t_iter.impltor = ret;                                                \
		ret->_t_iter.vt = &_tvec_##TYPE##_iter_vt;                                 \
		ret->src = v;                                                              \
		ret->index = 0;                                                            \
		return ret;                                                                \
	}                                                                              \
	\
	IMPL_TRAIT(tvec_##TYPE##_iter, iter)



#endif
