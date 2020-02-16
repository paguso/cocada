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

#include "cocadautil.h"
#include "hashmap.h"


/**
 * @file hashset.h
 * @author Paulo Fonseca
 * 
 * @brief Unordered, hashtable-based set - Hashset - ADT.
 */

/**
 * Hashset type
 */
typedef hashtable hashset;


/**
 * @brief Constructor.
 * 
 * @param hfunc Hash function pointer.
 * @param eqfunc Equality comparator function pointer.
 */
hashset *hashset_new( size_t typesize,
                      size_t (*hash_fn)(const void *), 
                      bool (*equals_fn)(const void *, const void *));


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



/**
 * Hashset iterator type
 */
typedef hashtable_iterator hashset_iterator;


/**
 * @brief Returns a new iterator for the @p set.
 */
hashset_iterator *hashset_get_iterator(hashset *set);


/**
 * @brief Iterator destructor.
 * @warning Only the iterator is destroyed. The set is left unmodified.
 */
void hashset_iterator_free(hashset_iterator *it);


/**
 * @brief Indicates whether there are still entries to be iterated over.
 */
bool hashset_iterator_has_next(hashset_iterator *it);


/**
 * @brief Gets the next element of the iteration.
 */
void *hashset_iterator_next(hashset_iterator *it);


#endif