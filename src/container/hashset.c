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
#include "hash.h"
#include "hashmap.h"
#include "hashset.h"


struct _hashset_iter {
    hashmap_iter *inner;
};


hashset *hashset_new(size_t typesize, hash_func hfunc, equals_func eqfunc)
{
    hashset *ret = hashmap_new(typesize, 0, hfunc, eqfunc);
    return ret;
}


void hashset_free(hashset *set, bool free_elements)
{
    hashmap_free(set, free_elements, false);
}


size_t hashset_size(hashset *set)
{
    return hashmap_size(set);
}


bool hashset_contains(hashset *set, void *elt)
{
    return hashmap_has_key(set, elt);
}

static int NOTHING = 0;

void hashset_add(hashset *set, void *elt)
{
    hashmap_set(set, elt, (void *)&NOTHING);
}


void hashset_remove(hashset *set, void *elt)
{
    hashmap_unset(set, elt);
}

#define HASHSET_CONTAINS_IMPL( TYPE ) \
   bool hashset_contains_##TYPE(hashset *set, TYPE elt ) {\
       return hashset_contains(set, &elt);\
   }


#define HASHSET_ADD_IMPL( TYPE ) \
   void hashset_add_##TYPE(hashset *set, TYPE elt ) {\
       hashset_add(set, &elt);\
   }


#define HASHSET_REMOVE_IMPL( TYPE ) \
   void hashset_remove_##TYPE(hashset *set, TYPE elt ) {\
       hashset_remove(set, &elt);\
   }


#define HASHSET_IMPL_ALL( TYPE )\
HASHSET_CONTAINS_IMPL(TYPE)\
HASHSET_ADD_IMPL(TYPE)\
HASHSET_REMOVE_IMPL(TYPE)

HASHSET_IMPL_ALL(int)
HASHSET_IMPL_ALL(size_t)
HASHSET_IMPL_ALL(byte_t)

hashset_iter *hashset_get_iter(hashset *set)
{
    hashset_iter *ret = NEW(hashset_iter);
    ret->inner = hashmap_get_iter(set);
    return ret;
}

void hashset_iter_free(hashset_iter *iter)
{
    hashmap_iter_free(iter->inner);
    FREE(iter);
}

bool hashset_iter_has_next(hashset_iter *iter)
{
    return hashmap_iter_has_next(iter->inner);
}

void *hashset_iter_next(hashset_iter *iter)
{
    return hashmap_iter_next(iter->inner).key;
}
