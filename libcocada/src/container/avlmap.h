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

#ifndef AVLMAP_H
#define AVLMAP_H

#include "avl.h"
#include "coretype.h"
#include "iter.h"
#include "new.h"
#include "order.h"
#include "trait.h"


/**
 * @file avlmap.h
 * @author Paulo Fonseca
 * @brief An ordered map backed by an AVL tree.
 */

/**
 * @brief AVL-Map type
 */
typedef struct _AVLMap AVLMap;


/**
 * @brief Constructor
 * @param keysize Size of key in bytes
 * @param valsize Size of value in bytes
 * @param keycmp key total-order comparator function
*/
AVLMap *avlmap_new(size_t keysize, size_t valsize, cmp_func keycmp);


/**
 * @brief Finaliser.
 * If the given finaliser @p fnr has one child, it is assumed to be the
 * finaliser for the keys. If it has two childre, the second is assumed
 * to be the finaliser for the values.
 */
void avlmap_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Returns the number of represented key-value pairs.
 */
size_t avlmap_size(const AVLMap *self);


/**
 * @brief Checks whether the map contains the given @p key.
 */
bool avlmap_contains(const AVLMap *self, const void *key);


/**
 * @brief  Gets an internal pointer to the value associated to the given @p key, if any.
 * Else returns NULL.
 */
const void *avlmap_get(const AVLMap *self, const void *key);

/**
 * @brief Gets an internal non-const pointer to the value associated to
 * the given @p key, if any. Else returns NULL.
 */
void *avlmap_get_mut(const AVLMap *self, const void *key);


/**
 * @brief Inserts a given @p key associated to a value @p val.
 * @warning  the map already contains the given key, the corresponding
 * value gets overwritten and any reference is therefore lost.
 */
void avlmap_ins(AVLMap *self, const void *key, const void *val);


/**
 * @brief Deletes the association corresponding to a given @p key.
 *
 * @warning If the map does not contain the provided @p key, the operation has no effect.
 * @warning This operation does not destroy the key or its value.
 * @see avlmap_remv
*/
void avlmap_del(AVLMap *self, void *key);


/**
 * @brief Removes an association corresponding to a given @p key,
 * if it exists, and returns the previously stored key and value
 * by copying them to @p dest_key and @p dest_val respectively.
 *
 * This is particularly useful when the key/value is an owned reference to
 * a heap-allocated object. Neither this method nor the ::hashmap_del method
 * destroy the dissociated key or value, which may cause a memory leak.
 * The copies returned via @p dest_key and
 * @p dest_val can be used by the caller to explicitly destroy those objects
 * after removal from the map.
 *
 * @warning If the map does not contain the provided @p key, this operation has no effect.
 * @warning This does not destroy the key or its value.
 */
void avlmap_remv(AVLMap *self, void *key, void *dest_key,
                 void *dest_val);


#define AVLMAP_DECL(TYPE, ...)\
	TYPE avlmap_get_##TYPE(AVLMap *self, const void *key);\
	void avlmap_ins_##TYPE(AVLMap *self, const void *key, TYPE val);\

XX_CORETYPES(AVLMAP_DECL)



/**
 * @brief Entry type to be used for iteration.
 */
typedef struct  {
	const void *key;
	const void *val;
} AVLMapEntry;
typedef struct _AVLMapIter AVLMapIter;
/**
 * @brief Returns an avlmap_entry (key-val pair) iterator.
 * @param self The source map.
 * @param order The iteration order of the underlying AVL tree.
 * @see avl_get_iter
 */
AVLMapIter *avlmap_get_iter(AVLMap *self, AVLTraversalOrder order);

/**
 * @brief Iterator destructor.
 */
void avlmap_iter_free(AVLMapIter *self);

DECL_TRAIT(AVLMapIter, Iter);

#endif
