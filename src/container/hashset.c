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

#include "cocadautil.h"
#include "hashset.h"
#include "hashtable.h"



struct _hashset_iterator {
    hashmap_iterator *hmap_it;
};

hashset *hashset_new( size_t typesize, size_t (*hash_fn)(const void *), 
                      bool (*equals_fn)(const void *, const void *))
{
    return hashtable_new(typesize, hash_fn, equals_fn);
}


void hashset_free(hashset *set, bool free_elements)
{
    hashtable_free(set, free_elements);
}


size_t hashset_size(hashset *set)
{
    return hashtable_size(set);
}


bool hashset_contains(hashset *set, void *elt)
{
    return hashtable_contains(set, elt);
}


void hashset_add(hashset *set, void *elt)
{
    if (hashset_contains(set, elt)) return; // no duplicates allowed
    hashtable_add(set, elt);
}


void hashset_remove(hashset *set, void *elt)
{
    hashtable_del(set, elt, NULL);
}


hashset_iterator *hashset_get_iterator(hashset *set)
{
    return hashtable_get_iterator(set);
}

void hashset_iterator_free(hashset_iterator *it)
{
    hashtable_iterator_free(it);
}

bool hashset_iterator_has_next(hashset_iterator *it)
{
    return ht_iterator_has_next(it);
}

void *hashset_iterator_next(hashset_iterator *it)
{
    return ht_iterator_next(it);
}

