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



HashSet *hashset_new(usize typesize, HashFunc hfunc, EqualsFunc eqfunc)
{
	HashSet *ret = hashmap_new(typesize, 0, hfunc, eqfunc);
	return ret;
}


void hashset_finalise(void *ptr, const Finaliser *fnr)
{
	hashmap_finalise(ptr, fnr);
}


usize hashset_size(const HashSet *set)
{
	return hashmap_size(set);
}


bool hashset_contains(const HashSet *set, const void *elt)
{
	return hashmap_contains(set, elt);
}

static char __NOCHAR;
#define __NOTHING ((void *)(&__NOCHAR))


void hashset_add(HashSet *set, const void *elt)
{
	hashmap_ins(set, elt, __NOTHING);
}


void hashset_remv(HashSet *set, const void *elt, void *dest)
{
	hashmap_remv(set, elt, __NOTHING, dest);
}


void hashset_del(HashSet *set, const void *elt)
{
	hashmap_del(set, elt);
}

#define HASHSET_CONTAINS_IMPL( TYPE ) \
	bool hashset_contains_##TYPE(HashSet *set, TYPE elt ) {\
		return hashset_contains(set, &elt);\
	}


#define HASHSET_ADD_IMPL( TYPE ) \
	void hashset_add_##TYPE(HashSet *set, TYPE elt ) {\
		hashset_add(set, &elt);\
	}


#define HASHSET_DEL_IMPL( TYPE ) \
	void hashset_del_##TYPE(HashSet *set, TYPE elt ) {\
		hashset_del(set, &elt);\
	}


#define HASHSET_ALL_IMPL( TYPE, ... )\
	HASHSET_CONTAINS_IMPL(TYPE)\
	HASHSET_ADD_IMPL(TYPE)\
	HASHSET_DEL_IMPL(TYPE)

XX_CORETYPES(HASHSET_ALL_IMPL)



struct _HashSetIter {
	Iter _t_Iter;
	HashMapIter *inner;
};


bool _hashset_iter_has_next(Iter *it)
{
	return iter_has_next(HashMapIter_as_Iter(((HashSetIter *)(
	                         it->impltor))->inner));
}


const void *_hashset_iter_next(Iter *it)
{
	return ((HashMapEntry *)iter_next(HashMapIter_as_Iter(((HashSetIter *)(
	                                      it->impltor))->inner)))->key;
}


static Iter_vt _hashset_iter_vt = { .has_next = _hashset_iter_has_next,
                                    .next = _hashset_iter_next
                                  };



HashSetIter *hashset_get_iter(HashSet *set)
{
	HashSetIter *ret = NEW(HashSetIter);
	ret->_t_Iter.impltor = ret;
	ret->_t_Iter.vt = &_hashset_iter_vt;
	ret->inner = hashmap_get_iter(set);
	return ret;
}


void hashset_iter_finalise(void *ptr, const Finaliser *fnr)
{
	FREE(((HashSetIter *)ptr)->inner);
}


IMPL_TRAIT(HashSetIter, Iter)
