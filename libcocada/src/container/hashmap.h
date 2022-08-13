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

#include "coretype.h"
#include "hash.h"
#include "iter.h"
#include "new.h"
#include "order.h"
#include "trait.h"

/**
 * @file hashmap.h
 * @author Paulo Fonseca
 *
 * @brief Generic N:1 key->val map (for <b>non-null</b> keys) implemented as a hash table.
 */

/**
 * Hashmap type
 */
typedef struct _hashmap hashmap;


/**
 * @brief Hashmap entry type representing a key->value association.
 * @warning Do not alter the key or value directly.
 */
typedef struct  {
	const void *key; /**< key */
	const void *val; /**< value */
} hashmap_entry;


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
hashmap *hashmap_new(size_t keysize, size_t valsize, hash_func keyhash,
                     eq_func keyeq);


/**
 * @brief Initialiser for an already allocated hashmap
 * Analogous to ::hashmap_new
 * @see hashmap_new
 */
void hashmap_init(hashmap *map, size_t keysize, size_t valsize,
                  hash_func keyhash, eq_func keyeq);


/**
 * @brief Creates a hash map with **at least** some initial capacity.
 * @note  Ensures initial capacity >= @p min_capacity
 * @see hashmap_new
 */
hashmap *hashmap_new_with_capacity(size_t keysize, size_t valsize,
                                   hash_func keyhash, eq_func keyeq,
                                   size_t min_capacity);


/**
 * @brief Initialiser for an already allocated hashmap.
 * Analogous to ::hashmap_new_with_capacit
 * @see hashmap_new_with_capacity
 */
void hashmap_init_with_capacity(hashmap *map, size_t keysize, size_t valsize,
                                hash_func keyhash, eq_func keyeq,
                                size_t min_capacity);


/**
 * @brief Finaliser
 * If the destructor has one child, it is considered to be the destructor
 * for the keys.  If it has two children, the second is the destructor for
 * the values.
 * @see new.h
 */
void hashmap_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Returns the size of the type in bytes
 */
size_t hashmap_sizeof();


/**
 * @brief Adjusts the size of the table to the number of stored elements.
 */
void hashmap_fit(hashmap *hmap);


/**
 * @brief Checks whether the @p map already contains a given @p key.
 */
bool hashmap_contains(const hashmap *hmap, const void *key);


/**
 * @brief Gets an internal reference to the value associated to a given @p key.
 *
 * @warning If the map does not contain the provided @p key, an error may occur.
 */
const void *hashmap_get(const hashmap *hmap, const void *key);


/**
 * @brief Returns internal references to the (key,value) matching given @p key.
 *
 * @warning If the map does not contain the provided @p key, an error may occur.
 */
const hashmap_entry hashmap_get_entry(const hashmap *hmap, const void *key);


/**
 * @brief Gets a mutable internal reference to the value associated to a given @p key.
 *
 * @warning If the map does not contain the provided @p key, an error may occur.
 */
void *hashmap_get_mut(const hashmap *hmap, const void *key);


/**
 * @brief Sets the value associated to a given @p key.
 *
 * @warning If the map already contains the provided @p key, the current value
 * gets overwitten.
 */
void hashmap_ins(hashmap *hmap, const void *key, const void *val);


/**
 * @brief Deletes the association corresponding to a given @p key.
 *
 * @warning If the map does not contain the provided @p key, the operation has no effect.
 * @warning This operation does not destroy the key or its value.
 * @see hashmap_remv
 */
void hashmap_del(hashmap *hmap, const void *key);


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
 * after removal from the hashmap.
 *
 * ## Example
 * ```C
 * keyobj *key1 = keyobj_new("key1");
 * valobj *val1 = valobj_new("val1");
 * keyobj *key2 = keyobj_new("key2");
 * valobj *val2 = valobj_new("val2");
 * // insert references in the map
 * hashmap_ins(map, &key1, &val1);
 * hashmap_ins(map, &key2, &val2);
 * // later elsewhere...
 * keyobj *k1 = keyobj_new("key1");
 * hashmap_del(map, &k1);
 * keyobj_free(k1);
 * // k1 is deallocated but the original key1 and val1 are leaking (!)
 * keyobj *k2 = keyobj_new("key2");
 * keyobj *destk2;
 * valobj *destv2;
 * hashmap_remv(map, k2, &destk2, &destv2);
 * // destk2 and destv2 have the same values as key2 and val2
 * FREE(destk2);
 * FREE(destv2);
 * // key2 and val2 properly destroyed
 * ```
 * @warning If the map does not contain the provided @p key, this operation has no effect.
 * @warning This does not destroy the key or its value.
 */
void hashmap_remv(hashmap *hmap, const void *key, void *dest_key,
                          void *dest_val);


/**
 * @brief Returns the number of elements currently stored.
 */
size_t hashmap_size(const hashmap *hmap);




/**
 * Hashmap iterator type
 */
typedef struct _hashmap_iter hashmap_iter;


/**
 * @brief Returns a new iterator for the given @p map.
 * Implements iter trait.
 * The ::iter_next method returns a pointer to a ::hashmap_entry.
 * @see iter
 */
hashmap_iter *hashmap_get_iter(const hashmap *hmap);


DECL_TRAIT(hashmap_iter, iter);



#define HASHMAP_GET_DECL( TYPE ) \
	TYPE hashmap_get_##TYPE(hashmap *hmap, const void *key);

#define HASHMAP_SET_DECL( TYPE ) \
	void hashmap_ins_##TYPE(hashmap *hmap, const void *key, TYPE val);

#define HASHMAP_ALL_DECL( TYPE , ...) \
	HASHMAP_GET_DECL(TYPE) \
	HASHMAP_SET_DECL(TYPE)

XX_CORETYPES(HASHMAP_ALL_DECL)

#endif