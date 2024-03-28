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

#ifndef _ITER_H_
#define _ITER_H_

#include <stdbool.h>

#include "trait.h"

/**
 * @file iter.h
 *
 * @brief Iterator trait
 *
 * @author Paulo Fonseca
 *
 * To implement this iterator trait, follow the indications in trait.h
 *
 * If a concrete type implements this trait, say `citer` ,then it
 * is used as follows.
 *
 * ```C
 * citer *cit = // get a concrete iter reference
 * iter *it = citer_as_Iter(cit); // AS_TRAIT(cit, citer, iter)
 * while (iter_has_next(it)) {
 * 		void *elt = iter_next(it);
 * }
 * FREE(cit);
 * ```
 *
 * or, alternatively, with the ::FOREACH_IN_ITER macro.
 *
 * @see trait.h
 */


typedef struct _Iter Iter;


/**
 * @brief Iterator virtual table
 */
typedef struct {
	bool (*has_next) (Iter *it);
	const void *(*next) (Iter *it);
} Iter_vt;


/**
 * @brief Iterator type
 */
struct _Iter {
	Iter_vt *vt;
	void *impltor;
};


/**
 * @brief Does the iterator have any remaining element?
 */
bool iter_has_next(Iter *it);


/**
 * @brief Gets the next iterator element.
 * @return If  iter_has_next(it) == true returns a pointer to the next
 * element. Otherwise, the behaviour is undefined and may result in a
 * fatal error.
 */
const void *iter_next(Iter *it);


/**
 * @brief Meaning:
 * `for each element ELT_NAME of type ELT_TYPE of the iterator ITER do...`
 * that is, scans an iterator pointed by @p ITER naming each element
 * @p ELT_NAME (local iteration variable), which are of type pointer to
 * @p ELT_TYPE.
 *
 * For example, to iterate over a vector `v` of int
 *
 * ```C
 * vec *v = vec_new(sizeof(int));
 * vec_push_int(v, 10);
 * vec_push_int(v, 20);
 * vec_push_int(v, 30);
 * vec_iter *it = vec_get_iter(v);
 * FOREACH_IN_ITER(val, int, vec_iter_as_Iter(it)) {
 * 		printf ("Element is %d\n", *val); // notice that val is a pointer
 * }
 * FREE(it); // iterator must be manually disposed of after use
 * ```
 *
 * @param ELT_NAME The variable identifier for the elements returned in the iteration (local)
 * @param ELT_TYPE The base type of the elements. The iteration returns pointers to ELT_TYPE
 * @param ITER 		An initialised pointer to an ::iter
 */
#define FOREACH_IN_ITER(ELT_NAME, ELT_TYPE, ITER) \
	for (Iter *__it = (Iter *) (ITER); __it ; __it = NULL) \
		for (bool __has = iter_has_next(__it); __has; __has = false ) \
			for ( ELT_TYPE *ELT_NAME = (ELT_TYPE *) iter_next(__it); \
			        __has && ( (__has = iter_has_next(__it)) || !__has ); \
			        ELT_NAME = (__has) ? (ELT_TYPE *) iter_next(__it) : ELT_NAME )



#endif
