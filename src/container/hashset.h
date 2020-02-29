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

#include "new.h"
#include "hash.h"
#include "hashmap.h"


/**
 * @file hashset.h
 * @author Paulo Fonseca
 * 
 * @brief Unordered, hashmap-based set - Hashset - ADT.
 */

typedef hashmap hashset;


typedef struct _hashset_iter hashset_iter;


/**
 * @brief Constructor.
 * 
 * @param hashfunc Hash function pointer.
 * @param eqfunc Equality comparator function pointer.
 */
hashset *hashset_new( size_t typesize, hash_func hashfunc, equals_func eqfunc ); 


/**
 * @brief Destructor.
 * @param free_elements Indicates whether stored elements should be disposed.
 */
void hashset_free(hashset *set, bool free_elements);


/**
 * @brief Returns the number of stored elements.
 */
size_t hashset_size();


/**
 * @brief Checks whether the @p set contains a given element @p elt. 
 * @returns true iff @p set contains an element x s.t. 
 *          @p set->eqfunc(x, @p elt) == true
 */
bool hashset_contains(hashset *set, void *elt);


/**
 * @brief Adds a copy of the element @p elt to the @p set. 
 */
void hashset_add(hashset *set, void *elt);


/**
 * @brief Removes an element @p elt from the @p set. 
 */
void hashset_remove(hashset *set, void *elt);


#define HASHSET_CONTAINS_DECL( TYPE ) \
   bool hashset_contains_##TYPE(hashset *set, TYPE elt );


#define HASHSET_ADD_DECL( TYPE ) \
   void hashset_add_##TYPE(hashset *set, TYPE elt );


#define HASHSET_REMOVE_DECL( TYPE ) \
   void hashset_remove_##TYPE(hashset *set, TYPE elt );


#define HASHSET_DECL_ALL( TYPE )\
HASHSET_CONTAINS_DECL(TYPE)\
HASHSET_ADD_DECL(TYPE)\
HASHSET_REMOVE_DECL(TYPE)

HASHSET_DECL_ALL(int)
HASHSET_DECL_ALL(size_t)
HASHSET_DECL_ALL(byte_t)


/**
 * @brief Returns a new iterator for the @p set.
 */
hashset_iter *hashset_get_iter(hashset *set);


/**
 * @brief Iterator destructor.
 * @warning Only the iterator is destroyed. The set is left unmodified.
 */
void hashset_iter_free(hashset_iter *iter);


/**
 * @brief Indicates whether there are still entries to be iterated over.
 */
bool hashset_iter_has_next(hashset_iter *iter);


/**
 * @brief Gets the next element of the iteration.
 */
void *hashset_iter_next(hashset_iter *iter);



#endif