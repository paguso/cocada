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


struct _avlmap {
	size_t sizeofkey;
	size_t sizeofval;
	size_t sizeofentry;
	size_t size;
	avl *tree;
};

#define ENTRY_VAL_SZKEY(E, SK) ((E)?((void *)((void *)(E) + (SK))):NULL)
#define ENTRY_VAL(E) ENTRY_VAL_SZKEY(E, self->sizeofkey)

avlmap *avlmap_new(size_t keysize, size_t valsize, cmp_func keycmp)
{
	avlmap *ret = NEW(avlmap);
	ret->sizeofkey = keysize;
	ret->sizeofval = valsize;
	ret->sizeofentry = keysize + valsize;
	ret->size = 0;
	ret->tree = avl_new(ret->sizeofentry, keycmp);
	return ret;
}


void avlmap_finalise(void *ptr, const finaliser *fnr)
{
	avlmap *self = (avlmap *)ptr;
	avlmap_iter *it = avlmap_get_iter(self, POST_ORDER);
	const finaliser *key_fnr = (finaliser_nchd(fnr)>0) ? finaliser_chd(fnr,
	                           0) : NULL;
	const finaliser *val_fnr = (finaliser_nchd(fnr)>1) ? finaliser_chd(fnr,
	                           1) : NULL;
	if (key_fnr != NULL || val_fnr != NULL) {
		FOREACH_IN_ITER(entry, avlmap_entry, avlmap_iter_as_iter(it)) {
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


size_t avlmap_size(const avlmap *self)
{
	return self->size;
}


bool avlmap_contains(const avlmap *self, const void *key)
{
	return (self->size && avl_contains(self->tree, key));
}


const void *avlmap_get(const avlmap *self, const void *key)
{
	const void *entry = avl_get(self->tree, key);
	return ENTRY_VAL(entry);
}


void *avlmap_get_mut(const avlmap *self, const void *key)
{
	void *entry = (void *)avl_get(self->tree, key);
	return ENTRY_VAL(entry);
}


void avlmap_ins(avlmap *self, const void *key, const void *val)
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


void avlmap_del(avlmap *self, void *key)
{
	self->size -= avl_del(self->tree, key);
}


void avlmap_remv(avlmap *self, void *key, void *dest_key, void *dest_val)
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
TYPE avlmap_get_##TYPE(avlmap *self, const void *key)\
{\
	const void *v = avlmap_get(self, key);\
	return v ? ((TYPE *)v)[0] : (TYPE)0;\
}\
\
TYPE avlmap_ins_##TYPE(avlmap *self, const void *key, TYPE val)\
{\
	avlmap_ins(self, key, &val);\
}\

XX_CORETYPES(AVLMAP_IMPL)

struct _avlmap_iter {
	iter _t_iter;
	avlmap *src;
	avl_iter *tree_iter;
	avlmap_entry entry;
};


static bool _avlmap_iter_has_next(iter *it)
{
	avlmap_iter *amit = (avlmap_iter *)it->impltor;
	return (iter_has_next((avl_iter_as_iter(amit->tree_iter))));
}


static const void *_avlmap_iter_next(iter *it)
{
	avlmap_iter *amit = (avlmap_iter *)it->impltor;
	const void *rawentry = iter_next(avl_iter_as_iter(amit->tree_iter));
	amit->entry.key = rawentry;
	amit->entry.val = ENTRY_VAL_SZKEY(rawentry, amit->src->sizeofkey);
	return &amit->entry;
}


static iter_vt _avlmap_iter_vt = { .has_next = _avlmap_iter_has_next,
                                   .next = _avlmap_iter_next
                                 };


IMPL_TRAIT(avlmap_iter, iter)


avlmap_iter *avlmap_get_iter(avlmap *self, avl_traversal_order order)
{
	avlmap_iter *ret = NEW(avlmap_iter);
	ret->_t_iter.impltor = ret;
	ret->_t_iter.vt = &_avlmap_iter_vt;
	ret->src = self;
	ret->tree_iter = avl_get_iter(self->tree, order);
	return ret;
}

void avlmap_iter_free(avlmap_iter *self)
{
	avl_iter_free(self->tree_iter);
	FREE(self);
}