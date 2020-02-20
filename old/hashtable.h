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

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>

#include "bitsandbytes.h"

/**
 * @file hashtable.h
 * @author Paulo Fonseca
 * @brief  Hashtable for dynamic collections with constant-time add, delete, 
 *         and membership test operations.
 */


/**
 * @brief The hash function is used by the hash table to obtain a numeric key 
 *        for an element. This key is then used internally to determine the 
 *        location in which the element will be stored.
 */
typedef size_t (*hash_func)(const void *);


/**
 * @brief The equality function is used by the hashtable to compare elements. 
 *        It is related to the hash function in which elements that compare as
 *        equal *must* have the same hash key and, conversely, if two elements 
 *        have different hash values, then they must compare as different.
 */
typedef bool (*equals_func)(const void *, const void *);


typedef struct _hashtable hashtable;


/**
 * @brief Constructor.
 * @param typesize The size of the stored elements in bytes
 * @param hfunc    The hash key function 
 * @param eqfunc   The equality test function
 */
hashtable *hashtable_new(size_t typesize, hash_func hfunc, equals_func eqfunc);


/**
 * @brief Constructor.
 * @param capacity Initial table capacity
 * @param typesize The size of the stored elements in bytes
 * @param hfunc    The hash key function 
 * @param eqfunc   The equality test function
 */
hashtable *hashtable_new_with_capacity( size_t capacity, size_t typesize, 
                                        hash_func hfunc, equals_func eqfunc );


/**
 * @brief Destructor.
 * @param free_elts This argument must be true <b>only if</b> the hashtable is
 *                  used to store pointers that must be freed prior to the 
 *                  hashtable destruction. 
 */
void hashtable_free(hashtable *ht, bool free_elts);


/**
 * @brief Returns the number of elements in the hashtable.
 */
size_t hashtable_size(hashtable *ht);

    
/**
 * @brief Tests whether the hashtable contains a copy of @p elt. 
 *        Returns true if the hashtable tcontains any element which compares 
 *        as true to @p elt according to the comparator function provided 
 *        to the constructor.
 */
bool hashtable_contains(hashtable *ht, void *elt);


/**
 * @brief Adds a copy of @p elt to the hashtable.
 *        If <code>hashtable_contains(ht, elt)==true</code> then no element 
 *        is added and false is returned. 
 *        Otherwise the contents of the memory pointed to  by @p elt is 
 *        physically copied into the HT. 
 *        The size of the element is given by the typesize parameter provided 
 *        to the constructor.
 */
bool hashtable_add(hashtable *ht, void *elt);


/**
 * @brief Deletes @p elt from the hashtable.
 *        If <code>hashtable_contains(ht, elt)==false</code>  then no element
 *        is deleted and false is returned. 
 *        Otherwise the element that compares as true is deleted. 
 *        If the hashtable stores pointers and @p free_elt is true, then the 
 *        pointer is freed prior to removal.
 */
bool hashtable_del(hashtable *ht, void *elt, bool free_elt);


/**
 * @brief Copies the element that compares as true to @p elt into @p dest.
 *        If <code>hashtable_contains(ht, elt)==false</code> then no element
 *        is copied and false is returned.
 */
bool hashtable_get_cpy (hashtable *ht, void *elt, void *dest);


/**
 * @brief Returns a pointer to the element that compares as true to @p elt.
 *        If <code>hashtable_contains(ht, elt)==false</code> returns NULL.
 * @warn  Changing the memory content at the returned address might 
 *        render the hashtable inconsistent. Use this for read only purposes.
 */
const void* hashtable_get (hashtable *ht, void *elt);



#define HT_CONTAINS_DECL( TYPE )\
    bool hashtable_contains_##TYPE( hashtable *ht, TYPE val );

#define HT_ADD_DECL( TYPE )\
    bool hashtable_add_##TYPE( hashtable *ht, TYPE val );

#define HT_DEL_DECL( TYPE )\
    bool hashtable_del_##TYPE( hashtable *ht, TYPE val, bool free_elt );


#define HT_ALL_DECL( TYPE )\
HT_CONTAINS_DECL(TYPE)\
HT_ADD_DECL(TYPE)\
HT_DEL_DECL(TYPE)

HT_ALL_DECL(int)
HT_ALL_DECL(size_t)
HT_ALL_DECL(byte_t)


typedef struct _ht_iterator hashtable_iterator;

hashtable_iterator *hashtable_get_iterator(hashtable *ht);

void hashtable_iterator_free(hashtable_iterator *htit);

bool ht_iterator_has_next(hashtable_iterator *htit);

const void *ht_iterator_next(hashtable_iterator *htit);



#endif
