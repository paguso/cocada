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
#ifndef MINQUEUE_H
#define MINQUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "bitbyte.h"
#include "minqueue.h"
#include "new.h"
#include "order.h"


/**
 * @file minqueue.h
 * @author Paulo Fonseca
 *
 * @brief A queue with FIFO push/pop and *minimum* query.
 */
typedef struct _minqueue minqueue;

typedef struct _minqueue_iter minqueue_iter;

/**
 * @brief Constructor
 * @param typesize The size of the stored elements in bytes
 * @param cmp Comparator function
 * @see order.h
 */
minqueue *minqueue_new(size_t typesize,  cmp_func cmp);

/**
 * @brief Constructor
 * Creates a min queue with initial capacity.
 * @param typesize The size of the stored elements in bytes
 * @param cmp Comparator function
 * @see order.h
 */
minqueue *minqueue_new_with_capacity(size_t typesize,  cmp_func, size_t capacity);


/**
 * @brief Desctructor
 */
void minqueue_dtor(void *ptr, const dtor *dt);


/**
 * @brief Returns the number of elements in the queue.
 */
size_t minqueue_len(const minqueue *queue);


/**
 * @brief Pushes a new element to the end of the queue.
 */
void minqueue_push(minqueue *queue, const void *elt);


#define MINQUEUE_PUSH_DECL( TYPE )\
void minqueue_push_##TYPE(minqueue *queue, TYPE val);


/**
 * @brief Pops the element from the front of the queue and copies it into @p dest.
 * @warning No check is performed on @p queue bounds, or @p dest.
 */
void minqueue_pop(minqueue *queue, void *dest);


/**
 * @brief  Removes the element from the front of the queue.
 * @warning No check is performed on @p queue bounds.
 */
void minqueue_remv(minqueue *queue);



#define MINQUEUE_POP_DECL( TYPE )\
TYPE minqueue_pop_##TYPE(minqueue *queue);


/**
 * @brief Returns the minimum element of the queue.
 * If there are many, returns the first to have entered the queue.
 */
const void *minqueue_min(const minqueue *queue);


/**
 * @brief Copies the minimum element of the queue into @p dest.
 * If there are many, copies the first to have entered the queue.
 */
void minqueue_min_cpy(const minqueue *queue, void *dest);


#define MINQUEUE_MIN_DECL( TYPE )\
TYPE minqueue_min_##TYPE(const minqueue *queue);


#define MINQUEUE_ALL_DECL( TYPE )\
MINQUEUE_PUSH_DECL(TYPE)\
MINQUEUE_POP_DECL(TYPE)\
MINQUEUE_MIN_DECL(TYPE)


MINQUEUE_ALL_DECL(byte_t)
MINQUEUE_ALL_DECL(char)
MINQUEUE_ALL_DECL(short)
MINQUEUE_ALL_DECL(int)
MINQUEUE_ALL_DECL(long)
MINQUEUE_ALL_DECL(float)
MINQUEUE_ALL_DECL(double)
MINQUEUE_ALL_DECL(size_t)
MINQUEUE_ALL_DECL(int8_t)
MINQUEUE_ALL_DECL(int16_t)
MINQUEUE_ALL_DECL(int32_t)
MINQUEUE_ALL_DECL(int64_t)
MINQUEUE_ALL_DECL(uint8_t)
MINQUEUE_ALL_DECL(uint16_t)
MINQUEUE_ALL_DECL(uint32_t)
MINQUEUE_ALL_DECL(uint64_t)


struct _minqueue_iter {
	const minqueue *src;
	size_t index;
};


/**
 * @brief Returns an iterator over all min elements of the queue
 * in FIFO order.
 */
minqueue_iter minqueue_all_min(const minqueue *queue);


/**
 * @brief Returns whether the iteration is not yet finished.
 */
bool minqueue_iter_has_next(const minqueue_iter *iter);


/**
 * @brief Returns the next element of the iteration.
 * If iteration has finished, a memory access error may occur.
 */
const void *minqueue_iter_next(minqueue_iter *iter);


#endif