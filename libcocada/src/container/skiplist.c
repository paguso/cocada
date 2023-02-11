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

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "errlog.h"
#include "new.h"
#include "order.h"
#include "randutil.h"
#include "skiplist.h"
#include "vec.h"


typedef struct __skiplist_node {
	struct __skiplist_node *next;
	struct __skiplist_node *down;
	void *key;
} skiplist_node;

static skiplist_node head = {.next = NULL, .down = NULL, .key = NULL};

struct __skiplist {
	size_t sizeof_key;
	size_t len;
	double p;
	cmp_func key_cmp;
	size_t height;
	vec *levels;
	vec *precursors;
};


static void skiplist_add_level(skiplist *self)
{
	vec_push(self->levels, &head);
	skiplist_node *new_top_head = (skiplist_node *)vec_last_mut(self->levels);
	self->height++;
	if (self->height > 1) {
		skiplist_node *former_top_head = (skiplist_node *)vec_get(self->levels,
		                                 self->height - 2);
		new_top_head->down = former_top_head;
	}
	vec_push_rawptr(self->precursors, NULL);
	DEBUG("SkipList adding new level (new height=%zu)\n", self->height);
}


skiplist *skiplist_new(size_t sizeof_key, cmp_func key_comp)
{
	skiplist *ret = NEW(skiplist);
	ret->sizeof_key = sizeof_key;
	ret->len = 0;
	ret->key_cmp = key_comp;
	ret->p = 0.5;
	ret->height = 0;
	ret->levels = vec_new(sizeof(skiplist_node));
	ret->precursors = vec_new(sizeof(skiplist_node *));
	skiplist_add_level(ret);
	return ret;
}


void skiplist_finalise(void *ptr, const finaliser *fnr)
{
	skiplist *self = (skiplist *)ptr;
	if (finaliser_nchd(fnr)) {
		const finaliser *key_fnr = finaliser_chd(fnr, 0);
		for (skiplist_node *cur = vec_first_mut(self->levels); cur; cur = cur->next) {
			FINALISE(cur->key, key_fnr);
		}
	}
	skiplist_node *cur = vec_get_mut(self->levels, 0);
	while (cur->next) {
		skiplist_node *to_del = cur->next;
		cur->next = to_del->next;
		FREE(to_del->key);
		FREE(to_del);
	}
	for (size_t lvl = 1; lvl < self->height; lvl++) {
		skiplist_node *cur = vec_get_mut(self->levels, lvl);
		while (cur->next) {
			skiplist_node *to_del = cur->next;
			cur->next = to_del->next;
			FREE(to_del);
		}
	}
	DESTROY_FLAT(self->levels, vec);
	DESTROY_FLAT(self->precursors, vec);
}


size_t skiplist_len(skiplist *self)
{
	return self->len;
}


static void skiplist_get_precursors(skiplist *self, const void *key)
{
	assert(vec_len(self->precursors) == self->height);
	for (int lvl = self->height - 1; lvl >= 0; lvl--) {
		skiplist_node *cur = (skiplist_node *) vec_get_mut(self->levels, lvl);
		while ( cur->next != NULL &&
		        ( self->key_cmp(cur->next->key, key) < 0 ) ) {
			cur = cur->next;
		}
		vec_set_rawptr(self->precursors, lvl, cur);
	}
}


size_t random_height(skiplist *self)
{
	size_t ret = 1;
	while (rand_unif() < self->p && ret <= self->height) {
		ret++;
	}
	return ret;
}

bool skiplist_ins(skiplist *self, const void *src)
{
	skiplist_get_precursors(self, src);
	skiplist_node *lvl0_node = (skiplist_node *) vec_first_rawptr(self->precursors);
	if (lvl0_node->next && !self->key_cmp(lvl0_node->next->key, src)) {
		// duplicate (do nothing)
		DEBUG("Trying to insert a duplicate value in SkipList. Do nothing!\n");
		return false;
	}
	size_t h = random_height(self);
	while ( self->height < h ) {
		skiplist_add_level(self);
	}
	skiplist_get_precursors(self, src); // (!) must recompute
	DEBUG("SkipList adding new node of height %zu\n", h);
	void *new_node_data = malloc(self->sizeof_key);
	memcpy(new_node_data, src, self->sizeof_key);
	skiplist_node *down_node = NULL;
	for (int lvl = 0; lvl < h ; lvl++) {
		skiplist_node *prec = vec_get_rawptr(self->precursors, lvl);
		skiplist_node *new_node = NEW(skiplist_node);
		new_node->next = prec->next;
		new_node->down = down_node;
		new_node->key = new_node_data;
		prec->next = new_node;
		down_node = new_node;
	}
	self->len++;
	return true;
}


#define SKIPLIST_INS_IMPL(TYPE, ...)\
	bool skiplist_ins_##TYPE(skiplist *self, TYPE val)\
	{\
		return skiplist_ins(self, &val);\
	}\


XX_CORETYPES(SKIPLIST_INS_IMPL)


bool skiplist_remv(skiplist *self, const void *key, void *dest)
{
	skiplist_get_precursors(self, key);
	skiplist_node *lvl0_prec = (skiplist_node *) vec_first_rawptr(self->precursors);
	if (lvl0_prec->next == NULL || self->key_cmp(lvl0_prec->next->key, key) != 0) {
		// not found. do nothing
		return false;
	}
	void *to_del_key = lvl0_prec->next->key;
	for (int lvl = 0; lvl < self->height ; lvl++) {
		skiplist_node *prec = vec_get_rawptr(self->precursors, lvl);
		skiplist_node *to_del = prec->next;
		if (to_del == NULL || self->key_cmp(to_del->key, key) > 0) break;
		assert(self->key_cmp(to_del->key, key) == 0);
		prec->next = to_del->next;
		FREE(to_del);
	}
	if (dest) {
		memcpy(dest, to_del_key, self->sizeof_key);
	}
	FREE(to_del_key);
	self->len -= 1;
	return true;
}


bool skiplist_del(skiplist *self, const void *key)
{
	return skiplist_remv(self, key, NULL);
}


#define SKIPLIST_DEL_IMPL(TYPE, ...) \
	bool skiplist_del_##TYPE(skiplist *self, TYPE val) \
	{\
		return skiplist_del(self, &val);\
	}

XX_CORETYPES(SKIPLIST_DEL_IMPL)


const void *skiplist_get(skiplist *self, const void *key)
{
	skiplist_get_precursors(self, key);
	skiplist_node *lvl0_node = (skiplist_node *) vec_first_rawptr(self->precursors);
	if (lvl0_node->next == NULL || self->key_cmp(lvl0_node->next->key, key) != 0) {
		// not found
		return NULL;
	}
	return (const void *)(lvl0_node->next->key);
}


#define SKIPLIST_CONTAINS_IMPL(TYPE,...)\
	bool skiplist_contains_##TYPE (const skiplist *self, TYPE key)\
	{\
		return skiplist_get(self, &key) != NULL;\
	}

XX_CORETYPES(SKIPLIST_CONTAINS_IMPL)



/// Iterator implementation


struct __skiplist_iter {
	iter _t_iter;
	const skiplist *src;
	skiplist_node *cur;
};


static bool skiplist_iter_has_next(iter *it)
{
	skiplist_iter  *slit = (skiplist_iter *) it->impltor;
	return slit->cur->next != NULL;
}


static const void *skiplist_iter_next(iter *it)
{
	skiplist_iter  *slit = (skiplist_iter *) it->impltor;
	const void *key =  slit->cur->next->key;
	slit->cur = slit->cur->next;
	return key;
}


static iter_vt skiplist_iter_vt = {
	.has_next = skiplist_iter_has_next,
	.next = skiplist_iter_next
};


IMPL_TRAIT(skiplist_iter, iter);


skiplist_iter *skiplist_get_iter(const skiplist *self)
{
	skiplist_iter *ret = NEW(skiplist_iter);
	ret->_t_iter.impltor = ret;
	ret->_t_iter.vt = &skiplist_iter_vt;
	ret->src = self;
	ret->cur = (skiplist_node *) vec_get(ret->src->levels, 0);
	return ret;
}
