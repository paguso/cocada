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

#include <string.h>

#include "avl.h"
#include "avlmap.h"
#include "coretype.h"
#include "new.h"
#include "trait.h"
#include "iter.h"


struct _AVLMap {
	size_t sizeofkey;
	size_t sizeofval;
	size_t sizeofentry;
	size_t size;
	AVL *tree;
};

#define ENTRY_VAL_SZKEY(E, SK) ((E)?((void *)((void *)(E) + (SK))):NULL)
#define ENTRY_VAL(E) ENTRY_VAL_SZKEY(E, self->sizeofkey)

AVLMap *avlmap_new(size_t keysize, size_t valsize, cmp_func keycmp)
{
	AVLMap *ret = NEW(AVLMap);
	ret->sizeofkey = keysize;
	ret->sizeofval = valsize;
	ret->sizeofentry = keysize + valsize;
	ret->size = 0;
	ret->tree = avl_new(ret->sizeofentry, keycmp);
	return ret;
}


void avlmap_finalise(void *ptr, const finaliser *fnr)
{
	AVLMap *self = (AVLMap *)ptr;
	AVLMapIter *it = avlmap_get_iter(self, POST_ORDER);
	const finaliser *key_fnr = (finaliser_nchd(fnr) > 0) ? finaliser_chd(fnr,
	                           0) : NULL;
	const finaliser *val_fnr = (finaliser_nchd(fnr) > 1) ? finaliser_chd(fnr,
	                           1) : NULL;
	if (key_fnr != NULL || val_fnr != NULL) {
		FOREACH_IN_ITER(entry, AVLMapEntry, AVLMapIter_as_Iter(it)) {
			if (key_fnr) {
				FINALISE(entry->key, key_fnr);
			}
			if (val_fnr) {
				FINALISE(entry->val, val_fnr);
			}
		}
	}
	avlmap_iter_free(it);
	DESTROY_FLAT(self->tree, avl);
}


size_t avlmap_size(const AVLMap *self)
{
	return self->size;
}


bool avlmap_contains(const AVLMap *self, const void *key)
{
	return (self->size && avl_contains(self->tree, key));
}


const void *avlmap_get(const AVLMap *self, const void *key)
{
	const void *entry = avl_get(self->tree, key);
	return ENTRY_VAL(entry);
}


void *avlmap_get_mut(const AVLMap *self, const void *key)
{
	void *entry = (void *)avl_get(self->tree, key);
	return ENTRY_VAL(entry);
}


void avlmap_ins(AVLMap *self, const void *key, const void *val)
{
	void *entry = (void *)malloc(self->sizeofentry);
	memcpy(entry, key, self->sizeofkey);
	void *valptr = ENTRY_VAL(entry);
	memcpy(valptr, val, self->sizeofval);
	bool inserted = avl_ins(self->tree, entry);
	self->size += inserted;
	if (!inserted) {
		void *cur_val = avlmap_get_mut(self, key);
		memcpy(cur_val, val, self->sizeofval);
	}
	free(entry);
}


void avlmap_del(AVLMap *self, void *key)
{
	self->size -= avl_del(self->tree, key);
}


void avlmap_remv(AVLMap *self, void *key, void *dest_key, void *dest_val)
{
	void *entry = (void *)malloc(self->sizeofentry);
	bool removed = avl_remv(self->tree, key, entry);
	if (removed) {
		self->size--;
		memcpy(dest_key, entry, self->sizeofkey);
		void *val = ENTRY_VAL(entry);
		memcpy(dest_val, val, self->sizeofval);
	}
	free(entry);
}


#define AVLMAP_IMPL(TYPE, ...)\
	TYPE avlmap_get_##TYPE(AVLMap *self, const void *key){\
		const void *v = avlmap_get(self, key);\
		return v ? ((TYPE *)v)[0] : (TYPE)0;\
	}\
	\
	void avlmap_ins_##TYPE(AVLMap *self, const void *key, TYPE val){\
		avlmap_ins(self, key, &val);\
	}\

XX_CORETYPES(AVLMAP_IMPL)

struct _AVLMapIter {
	Iter _t_Iter;
	AVLMap *src;
	AVLIter *tree_iter;
	AVLMapEntry entry;
};


static bool _avlmap_iter_has_next(Iter *it)
{
	AVLMapIter *amit = (AVLMapIter *)it->impltor;
	return (iter_has_next((AVLIter_as_Iter(amit->tree_iter))));
}


static const void *_avlmap_iter_next(Iter *it)
{
	AVLMapIter *amit = (AVLMapIter *)it->impltor;
	const void *rawentry = iter_next(AVLIter_as_Iter(amit->tree_iter));
	amit->entry.key = rawentry;
	amit->entry.val = ENTRY_VAL_SZKEY(rawentry, amit->src->sizeofkey);
	return &amit->entry;
}


static Iter_vt _avlmap_iter_vt = { .has_next = _avlmap_iter_has_next,
                                   .next = _avlmap_iter_next
                                 };


IMPL_TRAIT(AVLMapIter, Iter)


AVLMapIter *avlmap_get_iter(AVLMap *self, AVLTraversalOrder order)
{
	AVLMapIter *ret = NEW(AVLMapIter);
	ret->_t_Iter.impltor = ret;
	ret->_t_Iter.vt = &_avlmap_iter_vt;
	ret->src = self;
	ret->tree_iter = avl_get_iter(self->tree, order);
	return ret;
}

void avlmap_iter_free(AVLMapIter *self)
{
	avl_iter_free(self->tree_iter);
	FREE(self);
}
