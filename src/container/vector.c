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
#include <stdio.h>
#include <string.h>

#include "arrayutil.h"
#include "bitsandbytes.h"
#include "cocadautil.h"
#include "vector.h"


const static size_t MIN_CAPACITY = 4; // (!) MIN_CAPACITY > 1
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2 
const static float  MIN_LOAD = 0.5;   // (!) GROW_BY*MIN_LOAD < 1

typedef struct _vector {
    size_t typesize;
    size_t len;
    size_t capacity;
    void *data;
    void *swp;
}
vector;


vector *vec_new(size_t typesize)
{
    return vec_new_with_capacity(typesize, MIN_CAPACITY);
}


vector *vec_new_with_capacity(size_t typesize, size_t init_capacity)
{
    vector *ret;
    ret = NEW(vector);
    ret->typesize = typesize;
    ret->capacity = MAX(MIN_CAPACITY, init_capacity);
    ret->len = 0;
    ret->data = malloc(ret->capacity*ret->typesize);
    ret->swp = malloc(typesize);
    return ret;
}


static void check_and_resize(vector *v)
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


void vec_free(vector *v, bool free_elements)
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


size_t vec_len(vector *v)
{
    return v->len;
}


size_t vec_typesize(vector *v)
{
    return v->typesize;
}


void vec_clear(vector *v)
{
    v->len = 0;
}


static void vec_trim(vector *v)
{
    v->data = realloc(v->data, MAX(MIN_CAPACITY, v->len)*v->typesize);
}


void *vec_detach(vector *v)
{
    vec_trim(v);
    void *data = v->data;
    FREE(v);
    return data;
}


const void *vec_get(vector *v, size_t pos)
{
    return v->data + ( pos * v->typesize );
}


void vec_get_cpy(vector *v, size_t pos, void *dest)
{
    memcpy(dest, v->data+(pos*v->typesize), v->typesize);
}


void vec_set(vector *v, size_t pos, void *src)
{
    check_and_resize(v);
    memcpy(v->data+(pos*v->typesize), src, v->typesize);
}


void vec_swap(vector *v, size_t i, size_t j)
{
    if (i==j) return;
    //byte_t swp[v->typesize];
    memcpy(v->swp, v->data+(i*v->typesize), v->typesize);
    memcpy(v->data+(i*v->typesize), v->data+(j*v->typesize), v->typesize);
    memcpy(v->data+(j*v->typesize), v->swp, v->typesize);
}


/*
void vec_app(dynarray *v, void *val)
{
    check_and_resize(v);
    ((void **)v->data)[v->len++] = val;
}
*/


void vec_app(vector *v, void *src)
{
    check_and_resize(v);
    memcpy(v->data+(v->len*v->typesize), src, v->typesize);
    v->len++;
}


/*
void vec_ins(dynarray *v, size_t pos, void *val)
{
    check_and_resize(v);
    for (size_t i=v->len; i>pos; i--) {
        ((void **)v->data)[i] = ((void **)v->data)[i-1];
    }
    ((void **)v->data)[pos] = val;
    v->len++;
}
*/


void vec_ins(vector *v, size_t pos, void *src)
{
    check_and_resize(v);
    memmove( v->data+((pos+1)*v->typesize), v->data+(pos*v->typesize), 
            (v->len-pos)*v->typesize );
    memcpy( v->data+(pos*v->typesize), src, v->typesize);
    v->len++;
}


void vec_del(vector *v, size_t pos, void *dest)
{
    if (dest!=NULL) 
        vec_get_cpy(v, pos, dest);
    memmove( v->data+(pos*v->typesize), v->data+((pos+1)*v->typesize),
             (v->len-pos-1)*v->typesize );
    v->len--;
    check_and_resize(v);
}


void vec_radixsort(vector *v, size_t (*key_fn)(const void *, size_t),
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


#define VEC_GET( TYPE ) \
   TYPE vec_get_##TYPE(vector *v, size_t pos)\
    { return *((TYPE *)vec_get(v, pos)); } 


#define VEC_SET( TYPE ) \
   void vec_set_##TYPE(vector *v, size_t pos, TYPE val)\
    { vec_set(v, pos, &val); } 


#define VEC_APP( TYPE ) \
   void vec_app_##TYPE(vector *v, TYPE val)\
    { vec_app(v, &val); } 


#define VEC_INS( TYPE ) \
   void vec_ins_##TYPE(vector *v, size_t pos, TYPE val)\
    { vec_ins(v, pos, &val); } 


#define VEC_DEL( TYPE ) \
   TYPE vec_del_##TYPE(vector *v, size_t pos)\
    {  TYPE r; vec_del(v, pos, &r); return r; } 


#define VEC_OPS( TYPE )\
VEC_GET(TYPE)\
VEC_SET(TYPE)\
VEC_APP(TYPE)\
VEC_INS(TYPE)\
VEC_DEL(TYPE)


VEC_OPS(int)
VEC_OPS(size_t)
VEC_OPS(byte_t)

