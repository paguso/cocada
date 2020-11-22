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
#include "errlog.h"
#include "new.h"
#include "order.h"


#define LEFT  0
#define RIGHT 1


#define AVL_FIELD_DECL( TYPE, ... ) TYPE TYPE##_val;

typedef union {
    XX_CORETYPES(AVL_FIELD_DECL);
} val_t;

typedef struct _avlnode {
	val_t val;
    union {
        XX_CORETYPES(AVL_FIELD_DECL);
    } old_val;
	char bf;
	struct _avlnode *chd[2];
} avlnode;


struct _avl {
	avlnode *root;
	cmp_func cmp;
	size_t typesize;
};


avl *avl_new(cmp_func cmp)
{
	avl *ret = NEW(avl);
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
		int where = self->cmp(key, &(cur->val));
		if (where == 0) {
			break;
		} else if (where < 0) {
			cur = cur->chd[0];
		} else if (where > 0) {
			cur = cur->chd[1];
		}
	}
	return cur ? cur->val.rawptr_val : NULL;
}


#define AVL_GET_IMPL(TYPE,...)\
const void *avl_get_##TYPE (avl *self, TYPE key)\
{\
	return avl_get(self, &key);\
}

XX_CORETYPES(AVL_GET_IMPL)


static avlnode *__rotate_left(avlnode *root)
{
	avlnode *r = root->chd[RIGHT];
	avlnode *rl = r->chd[LEFT];
	r->chd[LEFT] = root;
	root->chd[RIGHT] = rl;
	root->bf = root->bf - 1 - ((r->bf > 0) ? r->bf  : 0);
	r->bf = r->bf - 1 + ( (root->bf < 0) ? root->bf : 0);
	return r;
}


static avlnode *__rotate_right(avlnode *root)
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
} push_t;


static push_t __avl_push(avl *self, avlnode *root, void *val, size_t val_size, bool *insert)
{
	if (root == NULL) {
		push_t ret;
		ret.node = NEW(avlnode);
		ret.node->bf = 0;
		ret.node->chd[LEFT] = ret.node->chd[RIGHT] = NULL;
		memcpy(&(ret.node->val), val, sizeof(val_t));
		memcpy(&(ret.node->val), val, val_size);
		ret.height_chgd = 1;
		*insert = true;
		return ret;
	}
	int where = self->cmp(val, &(root->val));
	push_t ret;
	if (where == 0) { // duplicate value
		ret.node = root;
		ret.height_chgd = 0;
		return ret;
	} else if (where < 0) {
		ret = __avl_push(self, root->chd[0], val, val_size, insert);
		root->chd[LEFT] = ret.node;
		root->bf -= ret.height_chgd;
	} else if (where > 0) {
		ret = __avl_push(self, root->chd[1], val, val_size, insert);
		root->chd[RIGHT] = ret.node;
		root->bf += ret.height_chgd;
	}
	// check for unbalanced
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
			root->chd[LEFT] = __rotate_left(root->chd[LEFT]);
		}
		ret.node = __rotate_right(root);
		ret.height_chgd = 0;
		return ret;
	} else {//  (root->bf == +2)
		assert (root->chd[RIGHT] != NULL);
		if (root->chd[RIGHT]->bf < 0) {
			root->chd[RIGHT] = __rotate_right(root->chd[RIGHT]);
		}
		ret.node = __rotate_left(root);
		ret.height_chgd = 0;
		return ret;
	}
}


bool avl_push(avl *self, void *val)
{
	bool insert = false;
	push_t ret = __avl_push(self, self->root, &val, sizeof(rawptr), &insert);
	self->root = ret.node;
	return insert;
}


#define AVL_PUSH_IMPL(TYPE, ...)\
bool avl_push_##TYPE(avl *self, TYPE val)\
{\
	bool insert = false;\
	push_t ret = __avl_push(self, self->root, &val, sizeof(TYPE), &insert);\
	self->root = ret.node;\
	return insert;\
}\


XX_CORETYPES(AVL_PUSH_IMPL)


push_t __avl_del_min(avlnode *root, val_t *deleted_val) 
{
	assert(root != NULL);
	if (root->chd[LEFT] == NULL) { //root is the min node
		push_t ret = {.height_chgd = true, .node = root->chd[RIGHT]};
		*deleted_val = root->val;
		//memcpy(deleted_val, &(root->val), sizeof(val_t));
		free(root);
		return ret;
	} else {
		push_t ret = __avl_del_min(root->chd[LEFT], deleted_val);
		root->chd[LEFT] = ret.node;
		root->bf += ret.height_chgd;
		assert(!ret.height_chgd || (0 <= root->bf && root->bf <= 2));
		if (!ret.height_chgd) {
			ret.node = root;
			ret.height_chgd = false;
			return ret;
		} else if (root->bf == 0) { // was -1 now 0
			ret.node = root;
			ret.height_chgd = true;
			return ret;
		} else if (root->bf == +1) { // was 0 now +1
			ret.node = root;
			ret.height_chgd = false;
			return ret;
		} else { // was +1 now +2
			if (root->chd[RIGHT]->bf < 0) {
				root->chd[RIGHT] = __rotate_right(root->chd[RIGHT]);
			}
			ret.node = __rotate_left(root);
			ret.height_chgd = true;
			return ret;
		}
	}
}


push_t __avl_del(avl *self, avlnode *root, void *val, bool *deleted, val_t *deleted_val) 
{
	if (root==NULL) {
		push_t ret = {.height_chgd=0, .node=NULL};
		return ret;
	}
	int where = self->cmp(val, &(root->val));
	push_t ret;
	if (where < 0) {
		ret = __avl_del(self, root->chd[LEFT], val, deleted, deleted_val);
		root->chd[LEFT] = ret.node;
		root->bf += ret.height_chgd;
	} else if (where > 0) {
		ret = __avl_del(self, root->chd[RIGHT], val, deleted, deleted_val);
		root->chd[RIGHT] = ret.node;
		root->bf -= ret.height_chgd;
	} else { //delete this root node
		*deleted = true;
		//memcpy(deleted_val, &(root->val), sizeof(val_t));
		root->val = *deleted_val;
		if (root->chd[LEFT] == NULL) { // leaf or has only right chd
			ret.height_chgd = true;
			ret.node = root->chd[RIGHT];
			free(root);
			return ret;
		} else if (root->chd[RIGHT] == NULL) { // has only left chd 
			ret.height_chgd = true; 
			ret.node = root->chd[LEFT];
			free(root);
			return ret;			
		} else { // has two children
			val_t min_val;
			ret = __avl_del_min(root->chd[RIGHT], &min_val);
			root->chd[RIGHT] = ret.node;
			memcpy(&(root->val), &min_val, sizeof(val_t));
			root->bf -= ret.height_chgd;
		}
	} 
	// here ret has recursive call result (maybe to del min)
	// root->bf is up-to-date
	assert(-2 <= root->bf && root->bf <= 2);
	if (!ret.height_chgd) {
		ret.node = root;
		ret.height_chgd = false;
		return ret;
	} else if (root->bf == 0) { // was +-1 now 0
		ret.node = root;
		ret.height_chgd = true;
		return ret;
	} else if (root->bf == -1 || root->bf == +1) { // was 0 now +-1
		ret.node = root;
		ret.height_chgd = false;
		return ret;
	} else if (root->bf == -2 ) { // was -1 now -2
		if (root->chd[LEFT]->bf > 0) {
			root->chd[LEFT] = __rotate_left(root->chd[LEFT]);
		}
		ret.node = __rotate_right(root);
		ret.height_chgd = true;
		return ret;
	} else { // was +1 now +2
		if (root->chd[RIGHT]->bf < 0) {
			root->chd[RIGHT] = __rotate_right(root->chd[RIGHT]);
		}
		ret.node = __rotate_left(root);
		ret.height_chgd = true;
		return ret;
	}
}


void *avl_del(avl *self, void *val) 
{
	bool deleted;
	val_t deleted_val;
	push_t del_res = __avl_del(self, self->root, &val, &deleted, &deleted_val);
	self->root = del_res.node;
	return (deleted) ? deleted_val.rawptr_val : NULL;
}


#define AVL_DEL_IMPL(TYPE, ...) \
TYPE avl_del_##TYPE(avl *self, TYPE val) \
{\
	bool deleted;\
	val_t deleted_val;\
	push_t del_res = __avl_del(self, self->root, &val, &deleted, &deleted_val);\
	self->root = del_res.node;\
	return (deleted) ? deleted_val.TYPE##_val : 0;\
}

XX_CORETYPES(AVL_DEL_IMPL)





static void __avl_print(avlnode *root, size_t level, FILE *stream, void (*prt_val)(FILE *, const void *))
{
	if (root == NULL) {
		return;
	}
	__avl_print(root->chd[LEFT], level+1, stream, prt_val);
	for (size_t i=0; i<level; i++) {
		fprintf(stream, "    ");
	}
	fprintf(stream, "[val=");
	prt_val(stream, &(root->val));
	fprintf(stream, "  bf=%d]\n", (int)(root->bf));
	__avl_print(root->chd[RIGHT], level+1, stream, prt_val);
}


void avl_print(const avl *self, FILE *stream, void (*prt_val)(FILE *, const void *))
{
	__avl_print(self->root, 0, stream, prt_val);
}
