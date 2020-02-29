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

#ifndef HASHMAP_H
#define HASHMAP_H


#include "new.h"
#include "hash.h"

/**
 * @file hashmap.h
 * @author Paulo Fonseca
 *
 * @brief Generic key->val hash map, for <b>non-null</b> keys.
 */

/**
 * Hashmap type
 */
typedef struct _hashmap hashmap;


/**
 * Hashmap entry type
 */
typedef struct  {
    void *key;
    void *val;
} hashmap_entry;


/**
 * Hashmap iterator type
 */
typedef struct _hashmap_iter hashmap_iter;


/**
 * @brief Constructor.
 * In order to define a hash map, one has to provide:
 * 1. A hash function, and
 * 2. A key comparator function.
 * The hash function computes an integer 'hash value'
 * for a given key. This function is not strictly required to be
 * one-to-one, although this is usually a desired property.
 * However, this is a proper function in the sense that
 * (a) every non-null key must have a hash value and (b) keys with
 * different hash values must compare as different and,
 * conversely, any two keys which compare as equal by the key
 * comparator function are required to have the same hash value.

 * @param hfunc A pointer to a hash function.
 * @param keqfunc A pointer to a key comparator function.
 */
hashmap *hashmap_new(size_t keysize, size_t valsize, hash_func keyhash, equals_func keyeq);


/**
 * @brief Created a hash map with **at least** some initial capacity.
 * @note  Ensures initial capacity >= @p min_capacity
 * @see hashmap_new
 */
hashmap *hashmap_new_with_capacity(size_t keysize, size_t valsize, hash_func keyhash, equals_func keyeq,
                                   size_t min_capacity);


/**
 * @brief Destructor.
 * @param free_keys Indicates whether the key objects are to be freed.
 * @param free_vals Indicates whether the value objects are to be freed.
 */
void hashmap_free(hashmap *hmap, bool free_keys, bool free_vals);


void hashmap_dispose(void *ptr, dstr *dst);

DSTR_DECL(hashmap)

/**
 * @brief Checks whether the @p map already contains a given @p key.
 */
bool hashmap_has_key(hashmap *hmap, void *key);


/**
 * @brief Gets the value associated to a given @p key.
 * 
 * @warning If the map does not contain the provided @p key, an error may occur.
 */
void *hashmap_get(hashmap *hmap, void *key);


/**
 * @brief Sets the value associated to a given @p key.
 * 
 * @warning If the map already contains the provided @p key, the current value
 * gets overwitten.
 */
void hashmap_set(hashmap *hmap, void *key, void *val);


/**
 * @brief Deletes the association corresponding to a given @p key.
 * 
 * @warning If the map does not contain the provided @p key, an error may occur.
 */
void hashmap_unset(hashmap *hmap, void *key);


/**
 * @brief Returns the number of elements currently stored.
 */
size_t hashmap_size(hashmap *hmap);


/**
 * @brief Returns a new iterator for the given @p map.
 */
hashmap_iter *hashmap_get_iter(hashmap *hmap);


/**
 * @brief Iterator destructor. 
 * @warning Only the iterator is disposed. The hash map is left untouched.
 */
void hashmap_iter_free(hashmap_iter *iter);


/**
 * @brief Indicates whether there are still entries to be iterated over.
 */
bool hashmap_iter_has_next(hashmap_iter *iter);


/**
 * @brief Gets the next entry of the iteration.
 */
hashmap_entry hashmap_iter_next(hashmap_iter *iter);


#endif