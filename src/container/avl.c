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
#include <stdio.h>
#include <string.h>

#include "avl.h"
#include "coretype.h"
#include "new.h"
#include "order.h"


#define LEFT  0
#define RIGHT 1


typedef struct _avlnode {
	void *val;
	char bf;
	struct _avlnode *chd[2];
} avlnode;


avlnode *avlnode_new(void *val, size_t typesize)
{
	avlnode *ret = NEW(avlnode);
	ret->val = malloc(typesize);
	memcpy(ret->val, val, typesize);
	ret->bf = 0;
	ret->chd[LEFT] = ret->chd[RIGHT] = NULL;
	return ret;
}


struct _avl {
	size_t typesize;
	cmp_func cmp;
	avlnode *root;
};


avl *avl_new(size_t typesize, cmp_func cmp)
{
	avl *ret = NEW(avl);
	ret->typesize = typesize;
	ret->cmp = cmp;
	ret->root = NULL;
	return ret;
}


static void __avl_dtor(avlnode *root, const dtor *dt)
{
	if (root==NULL) {
		return;
	} else {
		__avl_dtor(root->chd[LEFT], dt);
		__avl_dtor(root->chd[RIGHT], dt);
		FINALISE(&(root->val), dt);
		FREE(root);
	}
}


void avl_dtor(void *ptr, const dtor *dt)
{
	avl *self = (avl *)ptr;
	if (dt->nchd > 0) {
		__avl_dtor(self->root, dtor_chd(dt, 0));
	} else {
		__avl_dtor(self->root, empty_dtor());
	}
}


const void *avl_get(avl *self, void *key)
{
	avlnode *cur = self->root;
	while (cur != NULL) {
		int where = self->cmp(key, cur->val);
		if (where == 0) {
			break;
		} else if (where < 0) {
			cur = cur->chd[0];
		} else if (where > 0) {
			cur = cur->chd[1];
		}
	}
	return cur ? cur->val : NULL;
}



avlnode *_rotate_left(avlnode *root)
{
	avlnode *r = root->chd[RIGHT];
	avlnode *rl = r->chd[LEFT];
	r->chd[LEFT] = root;
	root->chd[RIGHT] = rl;
	root->bf = root->bf - 1 - ((r->bf > 0) ? r->bf  : 0);
	r->bf = r->bf - 1 + ( (root->bf < 0) ? root->bf : 0);
	return r;
}


avlnode *_rotate_right(avlnode *root)
{
	avlnode *l = root->chd[LEFT];
	avlnode *lr = l->chd[RIGHT];
	root->chd[LEFT] = lr;
	l->chd[RIGHT] = root;
	root->bf = root->bf + 1 - ( (l->bf < 0) ? l->bf : 0 );
	l->bf = l->bf + 1 + ( (root->bf > 0) ? root->bf : 0 );
	return l;
}


typedef struct {
	avlnode *node;
	bool height_chgd;
} _push_ret;


_push_ret _push(avl *self, avlnode *root, void *val)
{
	if (root == NULL) {
		_push_ret ret;
		ret.node = avlnode_new(val, self->typesize);
		ret.height_chgd = 1;
		return ret;
	}
	int where = self->cmp(val, root->val);
	_push_ret ret;
	if (where == 0) { // duplicate value
		ret.node = root;
		ret.height_chgd = 0;
		return ret;
	} else if (where < 0) {
		ret = _push(self, root->chd[0], val);
		root->chd[LEFT] = ret.node;
		root->bf -= ret.height_chgd;
	} else if (where > 0) {
		ret = _push(self, root->chd[1], val);
		root->chd[RIGHT] = ret.node;
		root->bf += ret.height_chgd;
	}
	// check for unbalance
	assert (-2 <= root->bf && root->bf <= 2);
	if (!ret.height_chgd) {
		ret.node = root;
		return ret;
	} else if (root->bf == 0) {
		ret.node = root;
		ret.height_chgd = 0;
		return ret;
	} else if (root->bf == -1 || root->bf == +1) {
		ret.node = root;
		ret.height_chgd = 1;
		return ret;
	} else if (root->bf == -2) {
		assert (root->chd[LEFT] != NULL);
		if (root->chd[LEFT]->bf > 0) {
			root->chd[LEFT] = _rotate_left(root->chd[LEFT]);
		}
		ret.node = _rotate_right(root);
		ret.height_chgd = 0;
		return ret;
	} else {//  (root->bf == +2)
		assert (root->chd[RIGHT] != NULL);
		if (root->chd[RIGHT]->bf < 0) {
			root->chd[RIGHT] = _rotate_right(root->chd[RIGHT]);
		}
		ret.node = _rotate_left(root);
		ret.height_chgd = 0;
		return ret;
	}
}


void avl_push(avl *self, void *val)
{
	_push_ret ret = _push(self, self->root, val);
	self->root = ret.node;
}


static void __print(avlnode *root, size_t level, FILE *stream, void (*prt_val)(FILE *, const void *))
{
	if (root == NULL) {
		return;
	}
	__print(root->chd[LEFT], level+1, stream, prt_val);
	for (size_t i=0; i<level; i++) {
		fprintf(stream, "    ");
	}
	fprintf(stream, "[val=");
	prt_val(stream, root->val);
	fprintf(stream, "  bf=%d]\n", (int)(root->bf));
	__print(root->chd[RIGHT], level+1, stream, prt_val);
}


void avl_print(const avl *self, FILE *stream, void (*prt_val)(FILE *, const void *))
{
	__print(self->root, 0, stream, prt_val);
}