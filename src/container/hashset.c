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


#include "new.h"
#include "hash.h"
#include "hashmap.h"
#include "hashset.h"
#include "iter.h"



hashset *hashset_new(size_t typesize, hash_func hfunc, equals_func eqfunc)
{
	hashset *ret = hashmap_new(typesize, 0, hfunc, eqfunc);
	return ret;
}


void hashset_dtor(void *ptr, const dtor *dt)
{

	hashmap_dtor(ptr, dt);
}


size_t hashset_size(const hashset *set)
{
	return hashmap_size(set);
}


bool hashset_contains(const hashset *set, const void *elt)
{
	return hashmap_has_key(set, elt);
}

static int NOTHING = 0;

void hashset_add(hashset *set, const void *elt)
{
	hashmap_set(set, elt, (void *)&NOTHING);
}


void hashset_remove(hashset *set, const void *elt)
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


#define HASHSET_ALL_IMPL( TYPE, ... )\
HASHSET_CONTAINS_IMPL(TYPE)\
HASHSET_ADD_IMPL(TYPE)\
HASHSET_REMOVE_IMPL(TYPE)

XX_CORETYPES(HASHSET_ALL_IMPL)



struct _hashset_iter {
	iter _t_iter;
	hashmap_iter *inner;
};


bool _hashset_iter_has_next(iter *it)
{
	return iter_has_next(hashmap_iter_as_iter(((hashset_iter *)(it->impltor))->inner));
}


const void *_hashset_iter_next(iter *it)
{
	return ((hashmap_entry *)iter_next(hashmap_iter_as_iter(((hashset_iter *)(it->impltor))->inner)))->key;
}


static iter_vt _hashset_iter_vt = { .has_next = _hashset_iter_has_next,
                                    .next = _hashset_iter_next
                                  };



hashset_iter *hashset_get_iter(hashset *set)
{
	hashset_iter *ret = NEW(hashset_iter);
	ret->_t_iter.impltor = ret;
	ret->_t_iter.vt = &_hashset_iter_vt;
	ret->inner = hashmap_get_iter(set);
	return ret;
}


void hashset_iter_dtor(void *ptr, const dtor *dt)
{
	FREE(((hashset_iter *)ptr)->inner);
}


IMPL_TRAIT(hashset_iter, iter)