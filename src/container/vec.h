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

#ifndef VECTOR_H
#define VECTOR_H

#include "bitsandbytes.h"
#include "new.h"
#include "order.h"


/**
 * @file vector.h
 * @author Paulo Fonseca
 *
 * @brief Dynamic array ADT (a.k.a. Vector)
 */


/**
 * Vector type
 */
typedef struct _vec vec;

/**
 * @brief Vector constructor.
 * @param typesize The size of the elements to be stored (in bytes).
 */
vec *vec_new(size_t typesize);


/**
 * @brief Vector constructor.
 * @param typesize The size of the elements to be stored (in bytes).
 * @param init_capacity The initial capacity (in # of elements).
 */
vec *vec_new_with_capacity(size_t typesize, size_t init_capacity);


/**
 * @brief returns the type size of the actual implementation in bytes.
 */
size_t vec_sizeof();


/**
 * @brief Destructor.
 * @param free_elements Indicates whether referenced objects should be freed.
 *        To be used whenever the array stores pointers to dynamically allocated
 *        memory.
 */
void vec_free(vec *v, bool free_elements);


/**
 * @brief Finaliser
 * @see new.h
 */
void vec_dispose(void *v, dtor *dst );


/**
 * @brief Returns the # of elements logically stored.
 */
size_t vec_len(const vec *v);


/**
 * @brief Returns the individual size of stored elements (in bytes). 
 */
size_t vec_typesize(const vec *v);


/**
 * @brief Removes all elements.
 */
void vec_clear(vec *v);


/**
 * @brief Detaches and returns the current internal array 
 * 
 * @see vec_trim
 * @warning After this operation, the vector is destroyed.
 */
void *vec_detach(vec *v);


/**
 * @brief Returns (the internal reference to) the element at position @p pos.
 */
const void *vec_get(const vec *v, size_t pos);


/**
 * @brief Copies the element at position @p pos into the location 
 *        pointed to by @p dest 
 */
void  vec_get_cpy(const vec *v, size_t pos, void *dest);


/**
 * @brief Sets (overwrites) the element at position @p pos to a copy
 *        of the value pointed to by @p src.
 */
void  vec_set(vec *v, size_t pos, void *src);


/**
 * @brief Swaps elements at positions @p i and @p j
 */
void vec_swap(vec *v, size_t i, size_t j);


/**
 * @brief Appends a copy of the value pointed to by @p src.
 * The number of copied bytes is given by the array typesize.
 */
void vec_push(vec *v, void *src);


/**
 * @brief Inserts a copy of the element pointed to by @p src 
 *        at position @p pos.
 */
void vec_ins(vec *v, size_t pos, void *src);


/**
 * @brief Removes the element at position @p pos from the vector, 
 * copying its value to the position pointed to by @p dest.
 * @warning Does not check if @dest is valid
 */
void vec_pop(vec *v, size_t pos, void *dest);


/**
 * @brief Deletes the element at position @p pos from the vector.
 * The value/reference is lost.
 */
void vec_del(vec *v, size_t pos);


/**
 * @brief In-place sorting of vector elements using the Quicksort algorithm.
 */
void vec_qsort(vec *v, cmp_func cmp); 


/**
 * @brief Radix sort on a generic dynamic array.
 *
 * We assume that each element of the array can be associated to a
 * numeric key vector of key_size positions K = (K[key_size-1],...,K[0]),
 * where each key position K[j] assumes one of max_key integer values in
 * the range 0 <= K[j]< max_key.
 * Then this function performs a radix sort on the elements of the array based
 * on their key vectors, with K[0] being the least significant position, and
 * K[key_size-1] the most significant. That is the elements end up sorted by
 * K[key_size-1], then K[key_size-2], and so forth, downto K[0].
 *
 * @param v The vector to sort
 * @param key_fn A pointer to a function that computes K[j] from a given element
 * @param key_size The size of the key vector
 * @param max_key The noninclusive maximum value for each key position
 */
void vec_radixsort(vec *v, size_t (*key_fn)(const void *, size_t),
                    size_t key_size, size_t max_key);


#define VEC_NEW_DECL( TYPE ) \
   TYPE vec_new_##TYPE();

#define VEC_GET_DECL( TYPE ) \
   TYPE vec_get_##TYPE(vec *v, size_t pos);


#define VEC_SET_DECL( TYPE ) \
   void vec_set_##TYPE(vec *v, size_t pos, TYPE val);


#define VEC_APP_DECL( TYPE ) \
   void vec_app_##TYPE(vec *v, TYPE val);


#define VEC_INS_DECL( TYPE ) \
   void vec_ins_##TYPE(vec *v, size_t pos, TYPE val);


#define VEC_DEL_DECL( TYPE ) \
   TYPE vec_del_##TYPE(vec *v, size_t pos); 


#define VEC_ALL_DECL( TYPE )\
VEC_NEW_DECL(TYPE)\
VEC_GET_DECL(TYPE)\
VEC_SET_DECL(TYPE)\
VEC_APP_DECL(TYPE)\
VEC_INS_DECL(TYPE)\
VEC_DEL_DECL(TYPE)


VEC_ALL_DECL(int)
VEC_ALL_DECL(size_t)
VEC_ALL_DECL(byte_t)

                    
#endif
