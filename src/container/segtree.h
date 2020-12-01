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

#ifndef SEGTREE_H
#define SEGTREE_H

#include "coretype.h"
#include "mathutil.h"

/**
 * @file segtree.h
 * @brief Segment tree
 * @author Paulo Fonseca
 * 
 * A Segment Tree hierarchically partitions an array `T` of size `n` into a 
 * collection of intervals (or segments), allowing for so-called range 
 * queries `Q` to be answered over arbitrary intervals `T[l,r)=(T[l]..T[r-1])` . 
 * 
 * The query `Q(T[l,r))` must be so that :
 * 1. `Q(T[l,l)) = c` fo some constant `c`
 * 2. `Q(T[l,l+1)) = T[l]` , and
 * 3. `Q(T[l,r)) = merge( Q(T[l,m)), Q(T[m,r)) )` for any `0 <= l <= m <= r < n`.
 * 
 * In particular, property 3 implies that we can answer the query over an 
 * interval by combining (merging) partial results on any collection of
 * adjacent segments partitioning the original range.
 * 
 * ## Examples
 * Typical examples include
 * - Sum segtree: Q(T[l,r)) = T[l]+...+T[r-1], with Q(T[l,l))=0
 * - Min segtree: Q(T[l,r)) = min(T[l],...,T[r-1]), with Q(T[l,l)) = +Infinity
 * - Max segtree: Q(T[l,r)) = max(T[l],...,T[r-1]), with Q(T[l,l)) = -Infinity
 * 
 * 
 */


/**
 * @brief Segment tree opaque type.
 */
typedef struct __segtree segtree;


/**
 * @brief Merge function type. 
 * 
 * The merge function cobines the results of the queries on two adjacent 
 * intervals `[l,m)` and `[m,r)`, pointed by @p left and @p right,
 * and stores the result into the address pointed at by @p dest.
 */
typedef void (*merge_func)(const void *left, const void *right, void *dest);


#define SEGTREE_MERGE_DECL(TYPE,...)\
void segtree_merge_sum_##TYPE(const void *left, const void *right, void *dest);\
void segtree_merge_min_##TYPE(const void *left, const void *right, void *dest);\
void segtree_merge_max_##TYPE(const void *left, const void *right, void *dest);

XX_PRIMITIVES(SEGTREE_MERGE_DECL)

/**
 * @brief Generic constructor
 * @param range The size of the array, that is the posisions range from 0 to @p range-1
 * @param typesize The size of the stored elements in bytes
 * @param merge The merge function
 * @param init_val The default value returned by the query over an empty interval.
 * This value is also used for initialising the array elements.
 */ 
segtree *segtree_new(size_t range, size_t typesize, merge_func merge, const void *init_val);


/**
 * @brief Flat destructor.
 */
void segtree_free(segtree *self);


/**
 * @brief Updates an individual position
 */
void segtree_upd(segtree *self, size_t pos, const void *val);


/**
 * @brief Queries an individual position
 */
const void *segtree_qry(segtree *self, size_t pos);


/**
 * @brief Range query over `T[left, right)`.
 */
void segtree_range_qry(segtree *self, size_t left, size_t right, void *dest);


#define SEGTREE_OPS_DECL(TYPE, ...)\
void segtree_upd_##TYPE(segtree *self, size_t pos, TYPE val);\
\
TYPE segtree_qry_##TYPE(segtree *self, size_t pos);\
\
TYPE segtree_range_qry_##TYPE(segtree *self, size_t left, size_t right);\


XX_CORETYPES(SEGTREE_OPS_DECL)

#endif