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

#ifndef DEQUE_H
#define DEQUE_H

#include <stddef.h>
#include <stdbool.h>

#include "coretype.h"
#include "new.h"


/**
 * @file deque.h
 * @brief Double-ended queue (deque)
 * @author Paulo Fonseca
 *
 * A deque is a queue with insert/remove operations at both ends,
 * implemented as a circular buffer.
 */


/**
 * Opaque deque type
 */
typedef struct _deque deque;


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 */
deque *deque_new(size_t typesize);


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 * @param capacity The initial capacity.
 */
deque *deque_new_with_capacity(size_t typesize, size_t capacity);


/**
 * @brief Finaliser
 * @see new.h
 */
void deque_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Checks whether the deque is empty.
 */
bool deque_empty(const deque *q);


/**
 * @brief Returns the length (logical size) of the deque.
 */
size_t deque_len(const deque *q);


/**
 * @brief Returns an internal reference to the element at a given position.
 * @warning No check is performed on @p q bounds.
 */
const void *deque_get(const deque *q, size_t pos);


/**
 * @brief Copies the element at a given position into @p dest.
 * @warning No check is performed on @p q bounds or @p dest.
 */
void deque_get_cpy(const deque *q, size_t pos, void *dest);


/**
 * @brief Returns an internal reference to the element at the first position.
 * @warning No check is performed on @p q bounds.
 */
const void *deque_front(const deque *q);


/**
 * @brief Returns an internal reference to the element at the last position.
 * @warning No check is performed on @p q bounds.
 */
const void *deque_back(const deque *q);


/**
 * @brief Pushes an element onto the back of the deque.
 */
void deque_push_back(deque *q, const void *elt);


/**
 * @brief Pops the last element of @p q and copies its value to @p dest.
 * @warning No check is performed on @p q bounds or @p dest.
 */
void deque_pop_back(deque *q, void *dest);


/**
 * @brief Removes the last element of @p q.
 * @warning No check is performed on @p q bounds.
 */
void deque_del_back(deque *q);


/**
 * @brief Pushes an element onto the back of the deque.
 */
void deque_push_front(deque *q, const void *elt);


/**
 * @brief Pops the first element of @p q and copies its value to @p dest.
 * @warning No check is performed on @p q bounds or @p dest.
 */
void deque_pop_front(deque *q, void *dest);


/**
 * @brief Removes the last element of @p q.
 * @warning No check is performed on @p q bounds.
 */
void deque_del_front(deque *q);


#define DEQUE_NEW_DECL( TYPE )\
	deque *deque_new_##TYPE();

#define DEQUE_GET_DECL( TYPE )\
	TYPE deque_get_##TYPE(const deque *q, size_t pos);

#define DEQUE_FRONT_DECL( TYPE )\
	TYPE deque_front_##TYPE(const deque *q);

#define DEQUE_BACK_DECL( TYPE )\
	TYPE deque_back_##TYPE(const deque *q);

#define DEQUE_PUSH_BACK_DECL( TYPE )\
	void deque_push_back_##TYPE(deque *q, TYPE val);

#define DEQUE_POP_BACK_DECL( TYPE )\
	TYPE deque_pop_back_##TYPE(deque *q);

#define DEQUE_REMV_BACK_DECL( TYPE )\
	TYPE deque_del_back_##TYPE(deque *q);

#define DEQUE_PUSH_FRONT_DECL( TYPE )\
	void deque_push_front_##TYPE(deque *q, TYPE val);

#define DEQUE_POP_FRONT_DECL( TYPE )\
	TYPE deque_pop_front_##TYPE(deque *q);

#define DEQUE_REMV_FRONT_DECL( TYPE )\
	TYPE deque_del_front_##TYPE(deque *q);

#define DEQUE_ALL_DECL( TYPE, ... )\
	DEQUE_NEW_DECL(TYPE)\
	DEQUE_GET_DECL(TYPE)\
	DEQUE_FRONT_DECL(TYPE)\
	DEQUE_BACK_DECL(TYPE)\
	DEQUE_PUSH_BACK_DECL(TYPE)\
	DEQUE_POP_BACK_DECL(TYPE)\
	DEQUE_PUSH_FRONT_DECL(TYPE)\
	DEQUE_POP_FRONT_DECL(TYPE)\

XX_CORETYPES(DEQUE_ALL_DECL)

#endif