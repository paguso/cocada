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

#ifndef BINHEAP_H
#define BINHEAP_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "new.h"
#include "order.h"

/**
 * @file binheap.h
 * @author Paulo Fonseca
 *
 * @brief Binary heap.
 *
 * This generic binary heap maintains a dynamic collection of items
 * over an ordered set, with insertion and extraction in O(log(n))
 * worst-case time.
 *
 * The binary heap can be seen as a priority queue, with priority
 * given by a comparison function
 * ```
 * int (*cmp_func) (const void *left, const void *right);
 * ```
 * such that
 * - if the function returns a positive value, then `right`
 * has greater priority than `left` (`right` "dominates" `left`)
 * - if the function returns a negative value, then `left`
 * has greater priority than `right` (`left` "dominates" `right`)
 * - if the function returns zero, then `left` and `right` have
 * the same priority (neither `left` "dominates" `right` or the
 * opposite)
 *
 * A pop() operation always removes and returns an element which
 * is not dominated by any other element currently in the heap,
 * that is, no other element has greater priority (notice that
 * this works even with partial order relations by having
 * `cmp_func` returning zero when called on uncomparable elements).
 * If several elements respect this condition than any of them
 * can be returned.
 *
 */


/**
 * @brief Binary heap type.
 */
typedef struct _binheap binheap;


/**
 * @brief Creates a new empty binary heap.
 * @param typesize The size (in bytes) of the individual elements.
 * @param mode The mode of the heap
 */
binheap *binheap_new(cmp_func cmp, size_t typesize);


/**
 * @brief Finaliser
 * @see new.h
 */
void binheap_dtor(void *ptr, const dtor *dt);


/**
 * @brief Returns the number of elements stored in the heap.
 */
size_t binheap_size(const binheap *heap);


/**
 * @brief Stores a new element in the heap.
 */
void binheap_push(binheap *heap, const void *elt);


/**
 * @brief Removes an element with maximum priority (or
 *        at least one such that no other element has
 *        greater priority) and copies it to @p dest.
 */
void binheap_pop(binheap *heap, void *dest);



#define BINHEAP_PUSH_DECL( TYPE )\
	void binheap_push_##TYPE(binheap *heap, TYPE val);


#define BINHEAP_POP_DECL( TYPE )\
	TYPE binheap_pop_##TYPE(binheap *heap);


#define BINHEAP_ALL_DECL( TYPE, ... )\
	BINHEAP_PUSH_DECL(TYPE)\
	BINHEAP_POP_DECL(TYPE)

XX_CORETYPES(BINHEAP_ALL_DECL)



#endif