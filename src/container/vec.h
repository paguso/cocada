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
 */

#ifndef VECTOR_H
#define VECTOR_H

#include "coretype.h"
#include "iter.h"
#include "new.h"
#include "order.h"
#include "trait.h"

/**
 * @file vec.h
 * @brief Vector, a.k.a dynamic array.
 * @author Paulo Fonseca
 *
 * A vector (a.k.a. dynamic array) is a linear dynamic
 * collection of elements of the same type and fixed size.
 * It contains the usual access/insert/deletion operations for
 * individual elements at arbitrary positions, plus other
 * convenience functions.
 *
 * It is implemented as a heap allocated array with a given limited
 * capacity, which gets reallocated on demand.
 *
 * This is a **flat** vector (see ::new.h module documentation),
 * meaning the values are directly copied into the buffer, as
 * opposed to storing only references to elements located elsewhere.
 */


/**
 * @brief Vector type (opaque).
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
 * @brief Transforms raw byte array into a vector.
 *        The buffer @p buf is **moved into** the vector and becomes
 *        its internal buffer.
 *        To create an vector from a **copy** of a raw buffer, which is
 *        not moved, see #vec_new_from_arr_cpy
 * @param buf (**move**) The buffer containing the vector data.
 * @param len The lenght of the vector.
 * @param typesize The size in bytes of each vector element.
 * @warning
 * - The size of @p buf must be at least (@p len * @p typesize) bytes
 * - The pointer @p buf **must not be used directly (read or
 *   modified) after this function call**.
 * - Since it becomes the internal buffer,  @p buf  **must** be heap
 *   allocated. In particular, no constant arrays of string literals
 *   should be used.
 * @see vec_new_from_arr_cpy
 */
vec *vec_new_from_arr(void *buf, size_t len, size_t typesize);


/**
 * @brief Creates a vector from a copy of a raw buffer.
 *        This of course implies copying the data from the buffer
 *        to the vector.
 *        To turn @p buf into a dynamic array without duplicating its
 *        values see #vec_new_from_arr.
 * @param buf (**no transfer**) The buffer containing the vector data.
 * @param len The lenght of the vector.
 * @param typesize The  size in bytes of each vector element.
 * @see vec_new_from_arr
 */
vec *vec_new_from_arr_cpy(const void *buf, size_t len, size_t typesize);


/**
 * @brief Returns the type size of the actual implementation in bytes.
 */
size_t vec_sizeof();


/**
 * @brief Finaliser
 * @see new.h
 */
void vec_destroy(void *v, const dtor *dt);


/**
 * @brief Returns the # of elements logically stored.
 */
size_t vec_len(const vec *v);


/**
 * @brief Returns the individual size of stored elements (in bytes).
 */
size_t vec_typesize(const vec *v);


/**
 * @brief Resets the vector **without** erasing its contents.
 *
 * @warning This **DOES NOT** finalise the objects stored in the
 * vector. If this vector contains references to **owned** objects,
 * this might cause memory leaks.
 */
void vec_clear(vec *v);


/**
 * @brief Returns a reference to the internal buffer array.
 * @warning Directly modifying the returned array may result
 * in undefined behaviour. Use for read-only access.
 */
const void *vec_as_array(vec *v);


/**
 * @brief Fits the vector to its actual size, i.e. deallocates
 *        unused internal memory.
 */
void vec_fit(vec *v);


/**
 * @brief Detaches and returns the trimmed internal byte array.
 *        The size of the returned array in bytes will be
 *        vec_typesize(@p v) * vec_len(@p v);
 * @see vec_fit
 * @warning After this operation, the vector object is destroyed.
 */
void *vec_detach(vec *v);


/**
 * @brief Returns (the internal reference to) the element at position @p pos.
 */
const void *vec_get(const vec *v, size_t pos);


/**
 * @brief Returns (the internal reference to) the first element.
 * If none exists, return NULL.
 */
const void *vec_first(const vec *v);


/**
 * @brief Returns (the internal reference to) the last element.
 * If none exists, returns NULL.
 */
const void *vec_last(const vec *v);


/**
 * @brief Returns a mutable (non-const) reference to the element at position @p pos.
 */
void *vec_get_mut(const vec *v, size_t pos);


/**
 * @brief Returns a mutable (non-const) reference to the first element.
 * If none exists, return NULL.
 */
void *vec_first_mut(const vec *v);


/**
 * @brief Returns a mutable (non-const) reference to the last element.
 * If none exists, return NULL.
 */
void *vec_last_mut(const vec *v);


/**
 * @brief Copies the element at position @p pos into the location
 *        pointed to by @p dest
 */
void  vec_get_cpy(const vec *v, size_t pos, void *dest);


/**
 * @brief Sets (overwrites) the element at position @p pos to a copy
 *        of the value pointed to by @p src.
 */
void  vec_set(vec *v, size_t pos, const void *src);


/**
 * @brief Swaps elements at positions @p i and @p j
 */
void vec_swap(vec *v, size_t i, size_t j);


/**
 * @brief Appends a copy of the value pointed to by @p src.
 */
void vec_push(vec *v, const void *src);


/**
 * @brief Appends @p n copies of the value pointed to by @p src to the vector.
 */
void vec_push_n(vec *v, const void *src, size_t n);


/**
 * @brief Inserts a copy of the element pointed to by @p src
 *        at position @p pos.
 */
void vec_ins(vec *v, size_t pos, const void *src);


/**
 * @brief Concatenates a copy of the contents of @p src to at the end of @p dest,
 *        leaving @p src unchanged.
 * @warning the vectors are assumed to be of the same type. No check is performed.
 */
void vec_cat(vec *dest, const vec *src);


/**
 * @brief Removes the element at position @p pos from the vector,
 * copying its value to the position pointed to by @p dest.
 * @warning @p dest should be a valid address with enough space. No check is performed.
 */
void vec_pop(vec *v, size_t pos, void *dest);


/**
 * @brief Deletes the element at position @p pos from the vector.
 * The value/reference is lost.
 */
void vec_del(vec *v, size_t pos);


/**
 * @brief Clips the vector to @p v[@p from..@p to-1].
 * @warning Requires 0<=from<=to<=vec_len(@p v). No checks performed.
 */
void vec_clip(vec *v, size_t from, size_t to);


/**
 * @brief Rotates the vector contents npos positions to the left.
 *        If @p v has length `n`, then @p v[@p npos + i % n] becomes @p v[i],
 *        for i=0..n-1. If @p npos > `n`, this is the same as rotating
 *        @npos % `n` positions.
 *        Example: `vec_rotate_left(v=[a,b,c,d,e,f,g], 3)` => `v[d,e,f,g,a,b,c]`.
 */
void vec_rotate_left(vec *v, size_t npos);


/**
 * @brief Returns the position of the first element that is equal to
 *        @p val according to the equality function @p eq.
 *        If no element satisfies the condition, returns vec_len(v)
 * @note That is a linear search that performs O(n) comparisons
 */
size_t vec_find(vec *v, void *val, eq_func eq);


/**
 * @brief Performs a binary search for @p val in @p v
 * @returns The first position of @p val in @p v if it exists, else
 * returns the length of @p v.
 * @param cmp Comparison function
 * @see order.h
 */
size_t vec_bsearch(vec *v, void *val, cmp_func cmp);

/**
 * @brief Returns the position of the minimum element according to
 *        the order @p cmp. If the vector is empty, returns 0.
 */
size_t vec_min(vec *v, cmp_func cmp);


/**
 * @brief Returns the position of the minimum element according to
 *        the order @p cmp. If the vector is empty, returns 0.
 */
size_t vec_max(vec *v, cmp_func cmp);


/**
 * @brief In-place sorting of vector elements using the Quicksort algorithm.
 *
 * The @p cmp function should compare elements of the type stored in the vector,
 * so it receives pointers to locations containing such values.
 *
 * ## Example
 *
 * ```C
 * typedef struct {
 *    int first;
 *    int second;
 * } pair;
 *
 * int pair_cmp(const void *lp, const void *rp) {
 *    pair *left = (pair *)lp;
 *    pair *right = (pair *)lp;
 *    if (left.first < right.first) return -1;
 *    else if (left.first > right.first) return +1;
 *    else if (left.second < right.second) return -1;
 *    else if (left.second > right.second) return +1;
 *    else return 0;
 * }
 *
 * // ...
 *
 * vec *pairs = vec_new(sizeof(pair));
 *
 * pair p = {.first=1, .second=2};
 * vec_push(pairs, &p);
 *
 * // other insertions ...
 *
 * vec_qsort(pairs, pair_cmp);
 * ```
 *
 *
 * @see order.h
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
	/** @brief Creates a new TYPE vector @see coretype.h */ \
	TYPE vec_new_##TYPE();

#define VEC_GET_DECL( TYPE ) \
	/** @brief Returns TYPE copy of the element at position @p pos @see coretype.h */ \
	TYPE vec_get_##TYPE(const vec *v, size_t pos);

#define VEC_FIRST_DECL( TYPE ) \
	/** @brief Returns TYPE copy of the first element @see coretype.h */ \
	TYPE vec_first_##TYPE(const vec *v);

#define VEC_LAST_DECL( TYPE ) \
	/** @brief Returns TYPE copy of the last element @see coretype.h */ \
	TYPE vec_last_##TYPE(const vec *v);

#define VEC_SET_DECL( TYPE ) \
	/** @brief Sets (overwrites) the element at position @p pos to be a TYPE copy of @p val @see coretype.h */ \
	void vec_set_##TYPE(vec *v, size_t pos, TYPE val);

#define VEC_PUSH_DECL( TYPE ) \
	/** @brief Appends a TYPE copy of @p val @see coretype.h */ \
	void vec_push_##TYPE(vec *v, TYPE val);

#define VEC_INS_DECL( TYPE ) \
	/** @brief Inserts a TYPE copy of @p val at position @p pos  @see coretype.h */ \
	void vec_ins_##TYPE(vec *v, size_t pos, TYPE val);

#define VEC_POP_DECL( TYPE ) \
	/** @brief Removes and returns a TYPE copy of the element at position @p pos  @see coretype.h */ \
	TYPE vec_pop_##TYPE(vec *v, size_t pos);


#define TYPED_VEC_DECL( TYPE , ...) \
	VEC_NEW_DECL(TYPE) \
	VEC_GET_DECL(TYPE) \
	VEC_FIRST_DECL(TYPE) \
	VEC_LAST_DECL(TYPE) \
	VEC_SET_DECL(TYPE) \
	VEC_PUSH_DECL(TYPE) \
	VEC_INS_DECL(TYPE) \
	VEC_POP_DECL(TYPE)

XX_CORETYPES(TYPED_VEC_DECL)


/**
 * @brief Vector iterator type (opaque). Implements the ::iter trait.
 * @see iter.h
 */
typedef struct _vec_iter vec_iter;

vec_iter *vec_get_iter(vec *self);

DECL_TRAIT(vec_iter, iter)


#endif
