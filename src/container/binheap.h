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

#include "new.h"
#include "order.h"

/**
 * @file binheap.h
 * @author Paulo Fonseca
 *
 * @brief Binary heap.
 *
 * This generic binary heap maintains a dynamic collection of items
 * over an ordered set. It supports at least insertion and min/max
 * extraction in O(log(n)) time. 
 * 
 */

/**
 * @brief The mode of the heap: in a MIN_HEAP (MAX_HEAP) the elements are
 * extracted in MIN-first (resp. MAX-first) order.
 */
typedef enum {
    MIN_HEAP = 0,
    MAX_HEAP = 1
} heap_mode;


/**
 * @brief Binary heap type.
 */
typedef struct _binheap binheap;


/**
 * @brief Creates a new empty binary heap.
 * @param typesize The size (in bytes) of the individual elements.
 * @param mode The mode of the heap
 */
binheap *binheap_new(cmp_func cmp, size_t typesize,
                     heap_mode mode);


/**
 * @brief Destructor.
 */
void binheap_free(binheap *heap, bool free_elements);


void binheap_dispose(void *ptr, dtor *dst); 


/**
 * @brief Returns the number of elements stored in the heap.
 */
size_t binheap_size(binheap *heap);


/**
 * @brief Stores a new element in the heap.
 * @param elt A pointer to the element to be stored. Only this reference is
 * actually stored. Upon heap destruction, the pointed memory is only freed
 * if free_elements is set to 1.
 *
 * @see binheap_free
 */
void binheap_push(binheap *heap, void *elt);


/**
 * @brief Removes and returns the reference MIN/MAX element stored in
 *        the heap depending on the heap mode.
 *
 * @see heap_mode
 */
void binheap_pop(binheap *heap, void *dest);



#define BINHEAP_PUSH_DECL( TYPE )\
    void binheap_push_##TYPE(binheap *heap, TYPE val);


#define BINHEAP_POP_DECL( TYPE )\
    TYPE binheap_pop_##TYPE(binheap *heap);

#define BINHEAP_ALL_DECL( TYPE )\
BINHEAP_PUSH_DECL(TYPE)\
BINHEAP_POP_DECL(TYPE)

BINHEAP_ALL_DECL( int )
BINHEAP_ALL_DECL( size_t )




#endif