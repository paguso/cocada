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

typedef struct _avlnode {
	int8_t bf; // balance factor
	struct _avlnode *left;
	struct _avlnode *right;
} avlnode;

// store node data right after the avlnode header
#define NODE_DATA(N) ((void *)((void *)(N) + sizeof(avlnode)))


struct _avl {
	avlnode *root;
	size_t typesize;
	cmp_func cmp;
};


avl *avl_new(size_t typesize, cmp_func cmp)
{
	avl *ret = NEW(avl);
	ret->typesize = typesize;
	ret->cmp = cmp;
	ret->root = NULL;
	return ret;
}


static void __avl_finaliser(avlnode *root, const finaliser *fnr)
{
	if (root == NULL) {
		return;
	}
	else {
		__avl_finaliser(root->left, fnr);
		__avl_finaliser(root->right, fnr);
		if (fnr != NULL) {
			FINALISE(NODE_DATA(root), fnr);
		}
		FREE(root);
	}
}


void avl_finalise(void *ptr, const finaliser *fnr)
{
	avl *self = (avl *)ptr;
	if (finaliser_nchd(fnr) > 0) {
		__avl_finaliser(self->root, finaliser_chd(fnr, 0));
	}
	else {
		__avl_finaliser(self->root, NULL);
	}
}


bool avl_contains(const avl *self, const void *key)
{
	return avl_get(self, key) != NULL;
}


const void *avl_get(const avl *self, const void *key)
{
	avlnode *cur = self->root;
	while (cur != NULL) {
		int where = self->cmp(key, NODE_DATA(cur));
		if (where == 0) {
			return (const void *)NODE_DATA(cur);
		}
		else if (where < 0) {
			cur = cur->left;
		}
		else if (where > 0) {
			cur = cur->right;
		}
	}
	return NULL;
}


#define AVL_CONTAINS_IMPL(TYPE,...)\
	bool avl_contains_##TYPE (const avl *self, TYPE key)\
	{\
		return avl_get(self, &key) != NULL;\
	}

XX_CORETYPES(AVL_CONTAINS_IMPL)


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
	bool ok;
	avlnode *new_root;
	bool height_chgd;
} indel_result;


static indel_result __avl_ins(avl *self, avlnode *root, void *val)
{
	if (root == NULL) {
		indel_result ret;
		ret.ok = true;
		ret.new_root = malloc(sizeof(avlnode) + self->typesize);
		ret.new_root->bf = 0;
		ret.new_root->left = NULL;
		ret.new_root->right = NULL;
		memcpy(NODE_DATA(ret.new_root), val, self->typesize);
		ret.height_chgd = 1;
		return ret;
	}
	int where = self->cmp(val, NODE_DATA(root));
	indel_result ret;
	if (where == 0) { // duplicate value
		ret.ok = false;
		ret.new_root = root;
		ret.height_chgd = 0;
		return ret;
	}
	else if (where < 0) {
		ret = __avl_ins(self, root->left, val);
		root->left = ret.new_root;
		root->bf -= ret.height_chgd;
	}
	else if (where > 0) {
		ret = __avl_ins(self, root->right, val);
		root->right = ret.new_root;
		root->bf += ret.height_chgd;
	}
	// check for unbalanced
	assert (-2 <= root->bf && root->bf <= 2);
	if (!ret.height_chgd) {
		ret.new_root = root;
		return ret;
	}
	else if (root->bf == 0) {
		ret.new_root = root;
		ret.height_chgd = 0;
		return ret;
	}
	else if (root->bf == -1 || root->bf == +1) {
		ret.new_root = root;
		ret.height_chgd = 1;
		return ret;
	}
	else if (root->bf == -2) {
		assert (root->left != NULL);
		if (root->left->bf > 0) {
			root->left = __rotate_left(root->left);
		}
		ret.new_root = __rotate_right(root);
		ret.height_chgd = 0;
		return ret;
	}
	else {   //  (root->bf == +2)
		assert (root->right != NULL);
		if (root->right->bf < 0) {
			root->right = __rotate_right(root->right);
		}
		ret.new_root = __rotate_left(root);
		ret.height_chgd = 0;
		return ret;
	}
}


bool avl_ins(avl *self, void *val)
{
	indel_result ret = __avl_ins(self, self->root, val);
	self->root = ret.new_root;
	return ret.ok;
}


#define AVL_INS_IMPL(TYPE, ...)\
	bool avl_ins_##TYPE(avl *self, TYPE val)\
	{\
		return avl_ins(self, &val);\
	}\


XX_CORETYPES(AVL_INS_IMPL)

typedef struct {
	bool height_chgd;
	avlnode *root;
	avlnode *remvd_node;
} remv_min_result;


remv_min_result __avl_remv_min(avlnode *root)
{
	assert(root != NULL);
	if (root->left == NULL) { //root is the min node
		remv_min_result ret = {.height_chgd = true, .root = root->right, .remvd_node = root};
		return ret;
	}
	else {
		remv_min_result ret = __avl_remv_min(root->left);
		root->left = ret.root;
		root->bf += ret.height_chgd;
		assert(!ret.height_chgd || (0 <= root->bf && root->bf <= 2));
		if (!ret.height_chgd) {
			ret.root = root;
			ret.height_chgd = false;
			return ret;
		}
		else if (root->bf == 0) {   // was -1 now 0
			ret.root = root;
			ret.height_chgd = true;
			return ret;
		}
		else if (root->bf == +1) {   // was 0 now +1
			ret.root = root;
			ret.height_chgd = false;
			return ret;
		}
		else {   // was +1 now +2
			if (root->right->bf < 0) {
				root->right = __rotate_right(root->right);
			}
			ret.root = __rotate_left(root);
			ret.height_chgd = true;
			return ret;
		}
	}
}


indel_result __avl_remv(avl *self, avlnode *root, void *val, void *dest)
{
	if (root == NULL) {
		indel_result ret = {.ok = false, .height_chgd = 0, .new_root = NULL};
		return ret;
	}
	int where = self->cmp(val, NODE_DATA(root));
	indel_result ret = {.ok = false};
	if (where < 0) {
		ret = __avl_remv(self, root->left, val, dest);
		root->left = ret.new_root;
		root->bf += ret.height_chgd;
	}
	else if (where > 0) {
		ret = __avl_remv(self, root->right, val, dest);
		root->right = ret.new_root;
		root->bf -= ret.height_chgd;
	}
	else {   //delete this root node
		ret.ok = true;
		if (dest) {
			memcpy(dest, NODE_DATA(root), self->typesize);
		}
		if (root->left == NULL) { // leaf or has only right chd
			ret.height_chgd = true;
			ret.new_root = root->right;
			free(root);
			return ret;
		}
		else if (root->right == NULL) {   // has only left chd
			ret.height_chgd = true;
			ret.new_root = root->left;
			free(root);
			return ret;
		}
		else {   // has two children
			remv_min_result rmin_res = __avl_remv_min(root->right);
			root->right = rmin_res.root;
			memcpy(NODE_DATA(root), NODE_DATA(rmin_res.remvd_node), self->typesize);
			free(rmin_res.remvd_node);
			root->bf -= rmin_res.height_chgd;
		}
	}
	// here ret has recursive call result (maybe to remv_min)
	// root->bf is up-to-date
	assert(-2 <= root->bf && root->bf <= 2);
	if (!ret.height_chgd) {
		ret.new_root = root;
		ret.height_chgd = false;
		return ret;
	}
	else if (root->bf == 0) {   // was +-1 now 0
		ret.new_root = root;
		ret.height_chgd = true;
		return ret;
	}
	else if (root->bf == -1 || root->bf == +1) {   // was 0 now +-1
		ret.new_root = root;
		ret.height_chgd = false;
		return ret;
	}
	else if (root->bf == -2 ) {   // was -1 now -2
		ret.height_chgd = (root->left->bf != 0);
		if (root->left->bf > 0) {
			root->left = __rotate_left(root->left);
		}
		ret.new_root = __rotate_right(root);
		//ret.height_chgd = true;
		return ret;
	}
	else {   // was +1 now +2
		ret.height_chgd = (root->right->bf != 0);
		if (root->right->bf < 0) {
			root->right = __rotate_right(root->right);
		}
		ret.new_root = __rotate_left(root);
		//ret.height_chgd = true;
		return ret;
	}
}


bool avl_remv(avl *self, void *val, void *dest)
{
	indel_result res = __avl_remv(self, self->root, val, dest);
	self->root = res.new_root;
	return res.ok;
}


bool avl_del(avl *self, void *key)
{
	return avl_remv(self, key, NULL);
}


#define AVL_DEL_IMPL(TYPE, ...) \
	bool avl_del_##TYPE(avl *self, TYPE val) \
	{\
		return avl_del(self, &val);\
	}

XX_CORETYPES(AVL_DEL_IMPL)



static void __avl_print(avlnode *root, size_t level, FILE *stream,
                        void (*prt_val)(FILE *, const void *))
{
	if (root == NULL) {
		return;
	}
	__avl_print(root->left, level + 1, stream, prt_val);
	for (size_t i = 0; i < level; i++) {
		fprintf(stream, "    ");
	}
	fprintf(stream, "[val=");
	prt_val(stream, NODE_DATA(root));
	fprintf(stream, "  bf=%d]\n", (int)(root->bf));
	__avl_print(root->right, level + 1, stream, prt_val);
}


void avl_print(const avl *self, FILE *stream, void (*prt_val)(FILE *,
               const void *))
{
	__avl_print(self->root, 0, stream, prt_val);
}


/* Iterator implementation */


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
	const void *ret = NODE_DATA((avlnode *) stack_peek_rawptr(avlit->node_stack));
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
	if (self->root) {
		avlnode *cur;
		switch (order) {
		case PRE_ORDER:
			stack_push_rawptr(ret->node_stack, self->root);
			stack_push_byte_t(ret->next_chd_stack, 0);
			break;
		case IN_ORDER:
			stack_push_rawptr(ret->node_stack, self->root);
			stack_push_byte_t(ret->next_chd_stack, 0);
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
			stack_push_rawptr(ret->node_stack, self->root);
			stack_push_byte_t(ret->next_chd_stack, 0);
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
	}
	return ret;
}


void avl_iter_free(avl_iter *self)
{
	if (!self) return;
	DESTROY_FLAT(self->node_stack, stack);
	DESTROY_FLAT(self->next_chd_stack, stack);
	FREE(self);
}


IMPL_TRAIT(avl_iter, iter)
