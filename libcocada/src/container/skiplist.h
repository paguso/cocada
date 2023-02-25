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

#ifndef SKIPLIST_H
#define SKIPLIST_H


#include "coretype.h"
#include "order.h"
#include "trait.h"
#include "iter.h"

typedef struct __skiplist skiplist;


/**
 * @brief Constructor.
 * @param cmp Key comparison function
 * @see order.h
 * @warning The comparison function receives pointers to whatever
 * is stored in the SkipList. If the SL store pointers to
 * external objects, then @p cmp receives pointers to pointers as arguments.
 */
skiplist *skiplist_new(size_t typesize, cmp_func cmp);


/**
 * @brief Destructor
 * @see new.h
 */
void skiplist_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Returns the lenth, i.e. the number of elements stored in the list.
 */
size_t skiplist_len(skiplist *self);


/**
 * @brief Searches for an element matching a given @p key
 * (according to the SL comparison function) , if any, and
 * returns a pointer to the data stored therein.
 * In no such node is found, returns NULL.
 */
const void *skiplist_search(skiplist *self, const void *key);


/**
 * @brief Inserts a copy of the element pointed to by @p src
 * if it doesn't already contain an element matching this
 * value (according to the comparison function). Otherwise
 * this operation has no effect.
 */
bool skiplist_ins(skiplist *self, const void *src);


/**
 * @brief Removes the element matching a given @p key
 * (according to the comparison function) , if any,
 * and copies the value to @p dest.
 * If however @p dest is NULL, the node is just deleted.
 * If no element matching the @p key is found, the operation
 * has no effect and the function returns false.
 * @returns A boolean indicating whether the removal was
 * successful.
 * @warning After the operation, the SL has no longer a
 * reference to the removed value.
 */
bool skiplist_remv(skiplist *self, const void *key, void *dest);


/**
 * @brief Same as skiplist_remv(self, key, NULL)
 */
bool skiplist_del(skiplist *self, const void *key);


/**
 * @brief Updates the element matching @p src currently stored
 * in the SkipList, if it exists, otherwise do nothing.
 * @return true if the update ocurred, or false if no element
 * matching @p src is found.
 */
bool skiplist_upd(skiplist *self, const void *src);


#define SKIPLIST_DECLARE_ALL(TYPE, ...)\
	bool skiplist_contains_##TYPE(const skiplist *self, TYPE val);\
	bool skiplist_ins_##TYPE(skiplist *self, TYPE val);\
	bool skiplist_del_##TYPE(skiplist *self, TYPE val);


XX_CORETYPES(SKIPLIST_DECLARE_ALL)


typedef struct __skiplist_iter skiplist_iter;

DECL_TRAIT(skiplist_iter, iter);

/**
 * @brief Returns an iterator over the sorted SkipList elements.
 */
skiplist_iter *skiplist_get_iter(const skiplist *self);


#endif