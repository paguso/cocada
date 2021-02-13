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
#include "stack.h"
#include "order.h"


#define LEFT  0
#define RIGHT 1



#define AVL_FIELD_DECL( TYPE, ... ) TYPE TYPE##_val;

typedef union {
	XX_CORETYPES(AVL_FIELD_DECL);
} core_t;

typedef struct _avlnode {
	core_t val;
	char bf; // balance factor
	struct _avlnode *left;
	struct _avlnode *right;
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
	}
	else {
		__avl_dtor(root->left, dt);
		__avl_dtor(root->right, dt);
		FINALISE(&(root->val), dt);
		FREE(root);
	}
}


void avl_destroy(void *ptr, const dtor *dt)
{
	avl *self = (avl *)ptr;
	if (dt->nchd > 0) {
		__avl_dtor(self->root, dtor_chd(dt, 0));
	}
	else {
		__avl_dtor(self->root, empty_dtor());
	}
}


bool avl_get(avl *self, void *key, void **dest)
{
	avlnode *cur = self->root;
	while (cur != NULL) {
		int where = self->cmp(&key, &(cur->val));
		if (where == 0) {
			*dest = cur->val.rawptr_val;
			break;
		}
		else if (where < 0) {
			cur = cur->left;
		}
		else if (where > 0) {
			cur = cur->right;
		}
	}
	return cur != NULL;
}


#define AVL_GET_IMPL(TYPE,...)\
	bool avl_get_##TYPE (avl *self, TYPE key, TYPE *dest)\
	{\
		avlnode *cur = self->root;\
		while (cur != NULL) {\
			int where = self->cmp(&key, &(cur->val));\
			if (where == 0) {\
				*dest = cur->val.TYPE##_val;\
				break;\
			} else if (where < 0) {\
				cur = cur->left;\
			} else if (where > 0) {\
				cur = cur->right;\
			}\
		}\
		return cur != NULL;\
	}

XX_CORETYPES(AVL_GET_IMPL)


static avlnode *__rotate_left(avlnode *root)
{
	avlnode *r = root->right;
	avlnode *rl = r->left;
	r->left = root;
	root->right = rl;
	root->bf = root->bf - 1 - ((r->bf > 0) ? r->bf  : 0);
	r->bf = r->bf - 1 + ( (root->bf < 0) ? root->bf : 0);
	return r;
}


static avlnode *__rotate_right(avlnode *root)
{
	avlnode *l = root->left;
	avlnode *lr = l->right;
	root->left = lr;
	l->right = root;
	root->bf = root->bf + 1 - ( (l->bf < 0) ? l->bf : 0 );
	l->bf = l->bf + 1 + ( (root->bf > 0) ? root->bf : 0 );
	return l;
}


typedef struct {
	avlnode *node;
	bool height_chgd;
} push_t;


static push_t __avl_ins(avl *self, avlnode *root, void *val, size_t val_size,
                        bool *insert)
{
	if (root == NULL) {
		push_t ret;
		ret.node = NEW(avlnode);
		ret.node->bf = 0;
		ret.node->left = ret.node->right = NULL;
		memcpy(&(ret.node->val), val, sizeof(core_t));
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
	}
	else if (where < 0) {
		ret = __avl_ins(self, root->left, val, val_size, insert);
		root->left = ret.node;
		root->bf -= ret.height_chgd;
	}
	else if (where > 0) {
		ret = __avl_ins(self, root->right, val, val_size, insert);
		root->right = ret.node;
		root->bf += ret.height_chgd;
	}
	// check for unbalanced
	assert (-2 <= root->bf && root->bf <= 2);
	if (!ret.height_chgd) {
		ret.node = root;
		return ret;
	}
	else if (root->bf == 0) {
		ret.node = root;
		ret.height_chgd = 0;
		return ret;
	}
	else if (root->bf == -1 || root->bf == +1) {
		ret.node = root;
		ret.height_chgd = 1;
		return ret;
	}
	else if (root->bf == -2) {
		assert (root->left != NULL);
		if (root->left->bf > 0) {
			root->left = __rotate_left(root->left);
		}
		ret.node = __rotate_right(root);
		ret.height_chgd = 0;
		return ret;
	}
	else {  //  (root->bf == +2)
		assert (root->right != NULL);
		if (root->right->bf < 0) {
			root->right = __rotate_right(root->right);
		}
		ret.node = __rotate_left(root);
		ret.height_chgd = 0;
		return ret;
	}
}


bool avl_ins(avl *self, void *val)
{
	bool insert = false;
	push_t ret = __avl_ins(self, self->root, &val, sizeof(rawptr), &insert);
	self->root = ret.node;
	return insert;
}


#define AVL_PUSH_IMPL(TYPE, ...)\
	bool avl_ins_##TYPE(avl *self, TYPE val)\
	{\
		bool insert = false;\
		push_t ret = __avl_ins(self, self->root, &val, sizeof(TYPE), &insert);\
		self->root = ret.node;\
		return insert;\
	}\


XX_CORETYPES(AVL_PUSH_IMPL)


push_t __avl_del_min(avlnode *root, core_t *deleted_val)
{
	assert(root != NULL);
	if (root->left == NULL) { //root is the min node
		push_t ret = {.height_chgd = true, .node = root->right};
		*deleted_val = root->val;
		//memcpy(deleted_val, &(root->val), sizeof(core_t));
		free(root);
		return ret;
	}
	else {
		push_t ret = __avl_del_min(root->left, deleted_val);
		root->left = ret.node;
		root->bf += ret.height_chgd;
		assert(!ret.height_chgd || (0 <= root->bf && root->bf <= 2));
		if (!ret.height_chgd) {
			ret.node = root;
			ret.height_chgd = false;
			return ret;
		}
		else if (root->bf == 0) {   // was -1 now 0
			ret.node = root;
			ret.height_chgd = true;
			return ret;
		}
		else if (root->bf == +1) {   // was 0 now +1
			ret.node = root;
			ret.height_chgd = false;
			return ret;
		}
		else {   // was +1 now +2
			if (root->right->bf < 0) {
				root->right = __rotate_right(root->right);
			}
			ret.node = __rotate_left(root);
			ret.height_chgd = true;
			return ret;
		}
	}
}


push_t __avl_del(avl *self, avlnode *root, void *val, bool *deleted,
                 core_t *deleted_val)
{
	if (root==NULL) {
		push_t ret = {.height_chgd=0, .node=NULL};
		return ret;
	}
	int where = self->cmp(val, &(root->val));
	push_t ret;
	if (where < 0) {
		ret = __avl_del(self, root->left, val, deleted, deleted_val);
		root->left = ret.node;
		root->bf += ret.height_chgd;
	}
	else if (where > 0) {
		ret = __avl_del(self, root->right, val, deleted, deleted_val);
		root->right = ret.node;
		root->bf -= ret.height_chgd;
	}
	else {   //delete this root node
		*deleted = true;
		root->val = *deleted_val;
		if (root->left == NULL) { // leaf or has only right chd
			ret.height_chgd = true;
			ret.node = root->right;
			free(root);
			return ret;
		}
		else if (root->right == NULL) {   // has only left chd
			ret.height_chgd = true;
			ret.node = root->left;
			free(root);
			return ret;
		}
		else {   // has two children
			core_t min_val;
			ret = __avl_del_min(root->right, &min_val);
			root->right = ret.node;
			memcpy(&(root->val), &min_val, sizeof(core_t));
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
	}
	else if (root->bf == 0) {   // was +-1 now 0
		ret.node = root;
		ret.height_chgd = true;
		return ret;
	}
	else if (root->bf == -1 || root->bf == +1) {   // was 0 now +-1
		ret.node = root;
		ret.height_chgd = false;
		return ret;
	}
	else if (root->bf == -2 ) {   // was -1 now -2
		if (root->left->bf > 0) {
			root->left = __rotate_left(root->left);
		}
		ret.node = __rotate_right(root);
		ret.height_chgd = true;
		return ret;
	}
	else {   // was +1 now +2
		if (root->right->bf < 0) {
			root->right = __rotate_right(root->right);
		}
		ret.node = __rotate_left(root);
		ret.height_chgd = true;
		return ret;
	}
}


bool avl_del(avl *self, void *val, void **dest)
{
	bool deleted;
	core_t deleted_val;
	push_t del_res = __avl_del(self, self->root, &val, &deleted, &deleted_val);
	self->root = del_res.node;
	if (deleted) {
		*dest = deleted_val.rawptr_val;
	}
	return deleted;
}


#define AVL_DEL_IMPL(TYPE, ...) \
	bool avl_del_##TYPE(avl *self, TYPE val, TYPE *dest) \
	{\
		bool deleted;\
		core_t deleted_val;\
		push_t del_res = __avl_del(self, self->root, &val, &deleted, &deleted_val);\
		self->root = del_res.node;\
		if (deleted) {\
			*dest = deleted_val.TYPE##_val;\
		}\
		return (deleted) ? deleted_val.TYPE##_val : 0;\
	}

XX_CORETYPES(AVL_DEL_IMPL)


static void __avl_print(avlnode *root, size_t level, FILE *stream,
                        void (*prt_val)(FILE *, const void *))
{
	if (root == NULL) {
		return;
	}
	__avl_print(root->left, level+1, stream, prt_val);
	for (size_t i=0; i<level; i++) {
		fprintf(stream, "    ");
	}
	fprintf(stream, "[val=");
	prt_val(stream, &(root->val));
	fprintf(stream, "  bf=%d]\n", (int)(root->bf));
	__avl_print(root->right, level+1, stream, prt_val);
}


void avl_print(const avl *self, FILE *stream, void (*prt_val)(FILE *,
               const void *))
{
	__avl_print(self->root, 0, stream, prt_val);
}




struct _avl_iter {
	iter _t_iter;
	avl *src;
	stack *node_stack;
	stack *next_chd_stack;
	avl_traversal_order order;
};


bool avl_iter_has_next (iter *it)
{
	return !stack_empty(((avl_iter *)it->impltor)->node_stack);
}


static void __next(avl *tree, avl_traversal_order order, stack *node_stack,
                   stack *next_chd_stack)
{
	byte_t nxtchd = stack_peek_byte_t(next_chd_stack);
	assert(nxtchd == order);
	bool read = false;
	//void *ret == NULL;
	while (!stack_empty(node_stack)) {
		avlnode *cur = stack_peek_rawptr(node_stack);
		if (cur == NULL) {
			stack_pop_rawptr(node_stack);
			stack_pop_byte_t(next_chd_stack);
			if (!stack_empty(node_stack)) {
				byte_t nc = stack_pop_byte_t(next_chd_stack);
				stack_push_byte_t(next_chd_stack, nc + 1);
			}
			continue;
		}
		byte_t nc = stack_peek_byte_t(next_chd_stack);
		if (nc == 0) {
			if (order == PRE_ORDER && read) {
				return;
			}
			read = true;
			stack_push_rawptr(node_stack, cur->left);
			stack_push_byte_t(next_chd_stack, 0);
		}
		else if (nc == 1) {
			if (order == IN_ORDER && read) {
				return;
			}
			read = true;
			stack_push_byte_t(next_chd_stack, 0);
			stack_push_rawptr(node_stack, cur->right);
		}
		else {   //nc == 2
			if (order == POST_ORDER && read) {
				return;
			}
			read = true;
			stack_pop_rawptr(node_stack);
			stack_pop_byte_t(next_chd_stack);
			if (!stack_empty(node_stack)) {
				byte_t nc = stack_pop_byte_t(next_chd_stack);
				stack_push_byte_t(next_chd_stack, nc + 1);
			}
		}
	}
}


const void *avl_iter_next (iter *it)
{
	assert(avl_iter_has_next(it));
	avl_iter *avlit = (avl_iter *) it->impltor;
	const void *ret = &(((avlnode *) stack_peek_rawptr(avlit->node_stack))->val);
	__next(avlit->src, avlit->order, avlit->node_stack, avlit->next_chd_stack);
	return ret;
}


static iter_vt avl_iter_vt = {.has_next = avl_iter_has_next, .next = avl_iter_next };


avl_iter *avl_get_iter(avl *self, avl_traversal_order order)
{
	avl_iter *ret = NEW(avl_iter);
	ret->_t_iter.impltor = ret;
	ret->_t_iter.vt = &avl_iter_vt;
	ret->src = self;
	ret->order = order;
	ret->node_stack = stack_new(sizeof(rawptr));
	ret->next_chd_stack = stack_new(sizeof(byte_t));
	avlnode *cur;
	switch (order) {
	case PRE_ORDER:
		if (self->root) {
			stack_push_rawptr(ret->node_stack, self->root);
			stack_push_byte_t(ret->next_chd_stack, 0);
		}
		break;
	case IN_ORDER:
		if (self->root) {
			stack_push_rawptr(ret->node_stack, self->root);
			stack_push_byte_t(ret->next_chd_stack, 0);
		}
		cur = (avlnode *)stack_peek_rawptr(ret->node_stack);
		while (cur->left != NULL) {
			stack_push_rawptr(ret->node_stack, cur->left);
			stack_push_byte_t(ret->next_chd_stack, 0);
			cur = (avlnode *)stack_peek_rawptr(ret->node_stack);
		}
		stack_pop_byte_t(ret->next_chd_stack);
		stack_push_byte_t(ret->next_chd_stack, 1);
		break;
	case POST_ORDER:
		if (self->root) {
			stack_push_rawptr(ret->node_stack, self->root);
			stack_push_byte_t(ret->next_chd_stack, 0);
		}
		cur = (avlnode *)stack_peek_rawptr(ret->node_stack);
		while ( cur->left != NULL || cur->right != NULL) {
			if (cur->left != NULL) {
				//stack_pop_byte_t(ret->next_chd_stack);
				//stack_push_byte_t(ret->next_chd_stack, 1);
				stack_push_rawptr(ret->node_stack, cur->left);
				stack_push_byte_t(ret->next_chd_stack, 0);
			}
			else {   // has cur->right
				//stack_pop_byte_t(ret->next_chd_stack);
				//stack_push_byte_t(ret->next_chd_stack, 1);
				stack_push_rawptr(ret->node_stack, cur->right);
				stack_push_byte_t(ret->next_chd_stack, 0);
			}
			cur = (avlnode *)stack_peek_rawptr(ret->node_stack);
		}
		stack_pop_byte_t(ret->next_chd_stack);
		stack_push_byte_t(ret->next_chd_stack, 2);
		break;
	default:
		ERROR("Invalid AVL traversal order");
		break;
	}
	return ret;
}


void avl_iter_free(avl_iter *self)
{
	if (!self) return;
	FREE(self->node_stack);
	FREE(self->next_chd_stack);
	FREE(self);
}


IMPL_TRAIT(avl_iter, iter)