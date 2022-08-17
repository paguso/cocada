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

#ifndef HASHSET_H
#define HASHSET_H

#include "coretype.h"
#include "hash.h"
#include "hashmap.h"
#include "iter.h"
#include "new.h"
#include "trait.h"

/**
 * @file hashset.h
 * @author Paulo Fonseca
 *
 * @brief Unordered, hashmap-based set
 */

typedef hashmap hashset;


/**
 * @brief Constructor.
 *
 * @param hashfunc Hash function pointer.
 * @param eqfunc Equality comparator function pointer.
 */
hashset *hashset_new( size_t typesize, hash_func hashfunc, equals_func eqfunc );


/**
 * @brief Destructor
 */
void hashset_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Returns the number of stored elements.
 */
size_t hashset_size(const hashset *set);


/**
 * @brief Checks whether the @p set contains an element equal to @p elt.
 */
bool hashset_contains(const hashset *set, const void *elt);


/**
 * @brief Adds a copy of the element @p elt to the @p set.
 */
void hashset_add(hashset *set, const void *elt);


/**
 * @brief Removes the element equal to @p elt from the @set, if any, 
 * and returns it by copying into @p dest. If no such element is
 * found, the operation has no effect.
 */
void hashset_remv(hashset *set, const void *elt, void *dest);


/**
 * @brief Deletes an element equal to @p elt from the @p set, if any.
 * If no such element is found, the operation has no effect.
 * @warning If the deleted element contains external references to
 * external objects, these might not be properly destroyed.
 * @see hashset_remv
 */
void hashset_del(hashset *set, const void *elt);





#define HASHSET_CONTAINS_DECL( TYPE ) \
	bool hashset_contains_##TYPE(hashset *set, TYPE elt );


#define HASHSET_ADD_DECL( TYPE ) \
	void hashset_add_##TYPE(hashset *set, TYPE elt );


#define HASHSET_DEL_DECL( TYPE ) \
	void hashset_del_##TYPE(hashset *set, TYPE elt );


#define HASHSET_ALL_DECL( TYPE, ... )\
	HASHSET_CONTAINS_DECL(TYPE)\
	HASHSET_ADD_DECL(TYPE)\
	HASHSET_DEL_DECL(TYPE)

XX_CORETYPES(HASHSET_ALL_DECL)

typedef struct _hashset_iter hashset_iter;


/**
 * @brief Returns a new iterator for the @p set.
 * Implements the iter trait.
 * The ::iter_next method returns an internal pointer to an element.
 */
hashset_iter *hashset_get_iter(hashset *set);


/**
 * @brief Finaliser.
 * Takes NO destructor
 */
void hashset_iter_finalise(void *ptr, const finaliser *fnr);


DECL_TRAIT(hashset_iter, iter);


#endif