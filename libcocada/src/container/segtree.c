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

#include <stdlib.h>
#include <string.h>

#include "arrays.h"
#include "coretype.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"
#include "segtree.h"
#include "vec.h"



#define SEGTREE_MERGE_IMPL(TYPE,...)\
	void segtree_merge_sum_##TYPE(const void *left, const void *right, void *dest) {\
		*((TYPE *)dest) = *((TYPE *)left) + *((TYPE *)right);\
	}\
	\
	void segtree_merge_min_##TYPE(const void *left, const void *right, void *dest) {\
		*((TYPE *)dest) = MIN(*((TYPE *)left), *((TYPE *)right));\
	}\
	\
	void segtree_merge_max_##TYPE(const void *left, const void *right, void *dest) {\
		*((TYPE *)dest) = MAX(*((TYPE *)left), *((TYPE *)right));\
	}

XX_PRIMITIVES(SEGTREE_MERGE_IMPL)



struct __segtree {
	size_t range;
	size_t typesize;
	merge_func merge;
	void *init_val;
	vec *tree;
};


static void segtree_reset(segtree *self)
{
	vec_push_n(self->tree, self->init_val, 2 * self->range);
}


segtree *segtree_new(size_t range, size_t typesize, merge_func merge,
                     const void *init_val)
{
	segtree *ret = NEW(segtree);
	ret->range = range;
	ret->typesize = typesize;
	ret->merge = merge;
	ret->init_val = malloc(sizeof(typesize));
	memcpy(ret->init_val, init_val, typesize);
	ret->tree = vec_new_with_capacity(typesize, 2 * range);
	segtree_reset(ret);
	return ret;
}


void segtree_free(segtree *self)
{
	DESTROY_FLAT(self->tree, vec);
	FREE(self->init_val);
	FREE(self);
}


void segtree_upd(segtree *self, size_t pos, const void *val)
{
	pos += self->range;
	vec_set(self->tree, pos, val);
	for (pos /= 2 ; pos > 0; pos /= 2) {
		self->merge( vec_get(self->tree, 2 * pos),
		             vec_get(self->tree, 2 * pos + 1),
		             vec_get_mut(self->tree, pos) );
	}
}


const void *segtree_qry(segtree *self, size_t pos)
{
	return vec_get(self->tree, pos + self->range);
}


void segtree_range_qry(segtree *self, size_t left, size_t right, void *dest)
{
	memcpy(dest, self->init_val, self->typesize);
	for (left += self->range, right += self->range; left < right;
	        left /= 2, right /= 2) {
		if (IS_ODD(left)) {
			self->merge(vec_get(self->tree, left++), (const void *)dest, dest);
		}
		if (IS_ODD(right)) {
			self->merge((const void *)dest, vec_get(self->tree, --right), dest);
		}
	}
}


#define SEGTREE_UPD_IMPL(TYPE)\
	void segtree_upd_##TYPE(segtree *self, size_t pos, TYPE val) {\
		segtree_upd(self, pos, &val);\
	}


#define SEGTREE_QRY_IMPL(TYPE)\
	TYPE segtree_qry_##TYPE(segtree *self, size_t pos) {\
		return *((TYPE *)segtree_qry(self, pos));\
	}


#define SEGTREE_RANGE_QRY_IMPL(TYPE)\
	TYPE segtree_range_qry_##TYPE(segtree *self, size_t left, size_t right) {\
		TYPE ret;\
		segtree_range_qry(self, left, right, &ret);\
		return ret;\
	}


#define SEGTREE_ALL_IMPL(TYPE, ...)\
	SEGTREE_UPD_IMPL(TYPE)\
	SEGTREE_QRY_IMPL(TYPE)\
	SEGTREE_RANGE_QRY_IMPL(TYPE)


XX_PRIMITIVES(SEGTREE_ALL_IMPL)