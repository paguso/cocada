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

#include "arrayutil.h"
#include "bitsandbytes.h"
#include "new.h"
#include "mathutil.h"
#include "order.h"
#include "randutil.h"
#include "vec.h"


const static size_t MIN_CAPACITY = 4; // (!) MIN_CAPACITY > 1
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2 
const static float  MIN_LOAD = 0.5;   // (!) GROW_BY*MIN_LOAD < 1

struct _vec {
    size_t typesize;
    size_t len;
    size_t capacity;
    void *data;
    void *swp;
};

vec *vec_new(size_t typesize)
{
    return vec_new_with_capacity(typesize, MIN_CAPACITY);
}

size_t vec_sizeof() {
    return sizeof(struct  _vec);
}

vec *vec_new_with_capacity(size_t typesize, size_t init_capacity)
{
    vec *ret;
    ret = NEW(vec);
    ret->typesize = typesize;
    ret->capacity = MAX(MIN_CAPACITY, init_capacity);
    ret->len = 0;
    ret->data = malloc(ret->capacity*ret->typesize);
    ret->swp = malloc(typesize);
    return ret;
}


static void check_and_resize(vec *v)
{
    if (v->len==v->capacity) { 
        v->capacity = MAX(GROW_BY*v->capacity, MIN_CAPACITY);
        v->data = realloc(v->data, v->capacity*v->typesize);
    }
    else if (v->capacity > MIN_CAPACITY && v->len < MIN_LOAD*v->capacity) {
        v->capacity = MAX(v->len/MIN_LOAD, MIN_CAPACITY);
        v->data = realloc(v->data, v->capacity*v->typesize);
    }
}
void vec_free(vec *v, bool free_elements)
{
    if (v==NULL) return;
    if (free_elements) {
        for (size_t i=0; i<v->len; i++) {
            FREE(((void **)v->data)[i]);
        }
    }
    FREE(v->data);
    FREE(v->swp);
    FREE(v);
}

void vec_dispose(void *ptr, const dtor *dt ) 
{
    vec *v = (vec *)ptr;
    if (dtor_nchd(dt)) {
        dtor *chd_dt = dtor_chd(dt, 0);
        for (size_t i=0, l=vec_len(v); i<l; i++) {
            void *chd = * (void **) vec_get(v, i);
            FINALISE(chd, chd_dt);
        }
    }
    FREE(v->data);
    FREE(v->swp);
}


size_t vec_len(const vec *v)
{
    return v->len;
}


size_t vec_typesize(const vec *v)
{
    return v->typesize;
}


void vec_clear(vec *v)
{
    v->len = 0;
}


static void vec_trim(vec *v)
{
    v->data = realloc(v->data, MAX(MIN_CAPACITY, v->len)*v->typesize);
}


void *vec_detach(vec *v)
{
    vec_trim(v);
    void *data = v->data;
    FREE(v);
    return data;
}


const void *vec_get(const vec *v, size_t pos)
{
    return v->data + ( pos * v->typesize );
}


void vec_get_cpy(const vec *v, size_t pos, void *dest)
{
    memcpy(dest, v->data+(pos*v->typesize), v->typesize);
}


void vec_set(vec *v, size_t pos, void *src)
{
    check_and_resize(v);
    memcpy(v->data+(pos*v->typesize), src, v->typesize);
}


void vec_swap(vec *v, size_t i, size_t j)
{
    if (i==j) return;
    memcpy(v->swp, v->data+(i*v->typesize), v->typesize);
    memcpy(v->data+(i*v->typesize), v->data+(j*v->typesize), v->typesize);
    memcpy(v->data+(j*v->typesize), v->swp, v->typesize);
}


void vec_push(vec *v, void *src)
{
    check_and_resize(v);
    memcpy(v->data+(v->len*v->typesize), src, v->typesize);
    v->len++;
}


void vec_ins(vec *v, size_t pos, void *src)
{
    check_and_resize(v);
    memmove( v->data+((pos+1)*v->typesize), v->data+(pos*v->typesize), 
            (v->len-pos)*v->typesize );
    memcpy( v->data+(pos*v->typesize), src, v->typesize);
    v->len++;
}


void vec_pop(vec *v, size_t pos, void *dest)
{
    vec_get_cpy(v, pos, dest);
    memmove( v->data+(pos*v->typesize), v->data+((pos+1)*v->typesize),
             (v->len-pos-1)*v->typesize );
    v->len--;
    check_and_resize(v);
}


void vec_del(vec *v, size_t pos)
{
    memmove( v->data+(pos*v->typesize), v->data+((pos+1)*v->typesize),
             (v->len-pos-1)*v->typesize );
    v->len--;
    check_and_resize(v);
}


static size_t _part(vec *v, size_t l, size_t r, cmp_func cmp)
{
    size_t p = rand_range_size_t(l, r);
    vec_swap(v, l, p);
    size_t i = l;
    size_t j = r-1;
    void *pv = v->data + (l * v->typesize);
    void *pi = v->data + (i * v->typesize);
    void *pj = v->data + (j * v->typesize);
    while ( i < j ) {
        while ( i < r && cmp(pi, pv) <= 0 ) {
            i++;
            pi += (v->typesize);
        }
        while ( cmp(pj, pv) > 0 ) {
            j--;
            pj -= (v->typesize);
        }
        if ( i < j ) {
            vec_swap(v, i, j);
        }
    }
    vec_swap(v, l, j);
    return j;
}


static void _qsort(vec *v, size_t l, size_t r, cmp_func cmp)
{
    if (l < r) {
        size_t p = _part(v, l, r, cmp);
        _qsort(v, l, p, cmp);
        _qsort(v, p+1, r, cmp);
    }
}


void vec_qsort(vec *v, cmp_func cmp)
{
    _qsort(v, 0, vec_len(v), cmp);
}



void vec_radixsort(vec *v, size_t (*key_fn)(const void *, size_t),
                   size_t key_size, size_t max_key)
{
    size_t n = vec_len(v);
    void *vcpy = malloc(n*(v->typesize));
    size_t *count = NEW_ARRAY(size_t, max_key);
    size_t i, k;
    for (size_t d=0; d<key_size; d++) {
        memset(count, 0, max_key*sizeof(size_t));
        for (i=0; i<n; i++) {
            k = key_fn(vec_get(v, i), d);
            count[k]++;
        }
        for (k=1; k<max_key; k++) 
            count[k] += count[k-1];
        for (i=n; i>0; i--) {
            k = key_fn(vec_get(v, i-1), d);
            count[k]--;
            vec_get_cpy(v, i-1, vcpy+(count[k]*v->typesize));
        }
        memcpy(v->data, vcpy, n*(v->typesize));
    }
    FREE(vcpy);
    FREE(count);
}


#define VEC_NEW_IMPL( TYPE ) \
   vec *vec_new_##TYPE()\
   vecurn vec_new(sizeof(TYPE)); } 


#define VEC_GET_IMPL( TYPE ) \
   TYPE vec_get_##TYPE(vec *v, size_t pos)\
    { return *((TYPE *)vec_get(v, pos)); } 


#define VEC_SET_IMPL( TYPE ) \
   void vec_set_##TYPE(vec *v, size_t pos, TYPE val)\
    { vec_set(v, pos, &val); } 


#define VEC_APP_IMPL( TYPE ) \
   void vec_app_##TYPE(vec *v, TYPE val)\
    { vec_push(v, &val); }


#define VEC_INS_IMPL( TYPE ) \
   void vec_ins_##TYPE(vec *v, size_t pos, TYPE val)\
    { vec_ins(v, pos, &val);} 


#define VEC_DEL_IMPL( TYPE ) \
   TYPE vec_del_##TYPE(vec *v, size_t pos)\
    {  TYPE r; vec_pop(v, pos, &r); return r; } 


#define VEC_ALL_IMPL( TYPE )\
VEC_GET_IMPL(TYPE)\
VEC_SET_IMPL(TYPE)\
VEC_APP_IMPL(TYPE)\
VEC_INS_IMPL(TYPE)\
VEC_DEL_IMPL(TYPE)


VEC_ALL_IMPL(int)
VEC_ALL_IMPL(size_t)
VEC_ALL_IMPL(byte_t)

