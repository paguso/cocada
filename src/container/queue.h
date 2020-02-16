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

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @file queue.h
 * @author Paulo Fonseca
 * 
 * @brief Queue ADT: dynamic linear collection with FIFO access policy. 
 * 
 * This queue can store references to external objects as well as 
 * internal primitive values via type-specific read/write operations. 
 */


/**
 * Queue type
 */
typedef struct _queue queue;


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 */
queue *queue_new(size_t typesize);


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 * @param capacity The initial capacity.
 */
queue *queue_new_with_capacity(size_t typesize, size_t capacity);


/**
 * @brief Destructor.
 * @param free_elements Indicates whether referenced elements should be freed.
 */
void queue_free(queue *q, bool free_elements);


/**
 * @brief Checks whether the queue is empty.
 */
bool queue_empty(queue *q);


/**
 * @brief Returns the length (logical size) of the queue.
 */
size_t queue_len(queue *q);


/**
 * @brief Pushes an element onto the back of the queue.
 */
void queue_push(queue *q, void *elt);


/**
 * @brief Pops the element from the front of a nonempty queue and copies its 
 *        value into @p dest. If @p dest is NULL, the value is discarded. 
 */
void queue_pop(queue *q, void *dest);


#define QUEUE_PUSH_DECL( TYPE ) \
    void queue_push_##TYPE(queue *q, TYPE val);


#define QUEUE_POP_DECL( TYPE ) \
    TYPE queue_pop_##TYPE(queue *q);    


#define QUEUE_ALL_DECL( TYPE ) \
QUEUE_PUSH_DECL(TYPE) \
QUEUE_POP_DECL(TYPE)


QUEUE_ALL_DECL(int)
QUEUE_ALL_DECL(size_t)



#endif