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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "errlog.h"
#include "mathutil.h"
#include "new.h"
#include "qdigest.h"


typedef struct _qdnode {
	size_t qty;
	struct _qdnode *chd[2];
}
qdnode;


#define LEFT 0
#define RIGHT 1
#define HAS_CHD(node) ((node)->chd[LEFT]!=NULL || (node)->chd[RIGHT]!=NULL)


struct _qdigest {
	double err;
	size_t range;
	size_t next_compress_cap;
	double errlogrange; // err / log(range) constant
	size_t total_qty;
	qdnode *root;
};


qdigest *qdigest_new(size_t range, double err)
{
	assert(err > 0);
	assert(range > 0);
	qdigest *ret = NEW(qdigest);
	ret->range = range;
	ret->next_compress_cap = 2;
	ret->errlogrange = err / log10(range);
	ret->err = err;
	ret->root = NEW(qdnode);
	ret->root->chd[LEFT] = ret->root->chd[RIGHT] = NULL;
	ret->root->qty = 0;
	return ret;
}


static inline size_t qdigest_cap(qdigest *self)
{
	return MAX(1, (size_t)(self->errlogrange * (double)(self->total_qty)));
}


typedef struct qdigest {
	size_t fst;
	size_t snd;
} size_pair;


static const size_pair zeropair = {.fst=0, .snd=0};


static size_pair tree_size (qdnode *root)
{
	if (root==NULL) {
		return zeropair;
	} else {
		size_pair ret = {.fst=0, .snd=0};
		size_pair l = tree_size(root->chd[LEFT]);
		size_pair r = tree_size(root->chd[RIGHT]);
		ret.fst = 1 + l.fst + r.fst;
		ret.snd = 1 + MAX(l.snd, r.snd);
		return ret;
	}
}


typedef struct {
	qdnode *new_root;
	size_t move_up;
} comp_pair;


static comp_pair __qdigest_compress(qdnode *root, size_t cap, size_t spare_up)
{

	assert(root != NULL);
	assert(root->qty > 0);
	comp_pair cp;
	size_t move_up = 0, spare_here, put_here;
	if ( HAS_CHD(root) ) { //non-leaf
		for (int dir = LEFT; dir <= RIGHT; dir++)  {
			if ( root->chd[dir] != NULL ) {
				spare_here = (cap - root->qty);
				cp  = __qdigest_compress(root->chd[dir], cap, spare_up + spare_here);
				root->chd[dir] = cp.new_root;
				put_here = MIN(spare_here, cp.move_up);
				root->qty += put_here;
				move_up += (cp.move_up - put_here);
				spare_up -= (cp.move_up - put_here);
			}
		}
	}
	size_t m = MIN(spare_up, root->qty);
	move_up += m;
	root->qty -= m;
	if ( root->qty == 0 ) {
		assert( !HAS_CHD(root) );
		free(root);
		cp.new_root = NULL;
		cp.move_up = move_up;
	} else {
		cp.new_root = root;
		cp.move_up = move_up;
	}
	return cp;
}


static void qdigest_compress(qdigest *self)
{
	if ( self->root != NULL ) {
		__qdigest_compress(self->root, self->errlogrange * self->total_qty, 0);
	}
}


void qdigest_upd(qdigest *self, size_t val, size_t qty)
{
	if ( val > self->range ) {
		WARN("QDigest: ignoring insertion of invalid value %zu.\n", val );
		return;
	}
	self->total_qty += qty;
	size_t cap = qdigest_cap(self);
	assert(self->root != NULL);
	qdnode *par = self->root, *cur = self->root;
	int dir;
	size_t l = 0, r = self->range, m;
	while( qty ) {
		if (cur == NULL) {
			cur = NEW(qdnode);
			cur->qty = 0;
			cur->chd[0] = cur->chd[1] = NULL;
			par->chd[dir] = cur;
		}
		if ( r - l > 1) { // non-leaf
			size_t qty_to_add = MIN( cap - cur->qty, qty);
			cur->qty += qty_to_add;
			qty -= qty_to_add;

			par = cur;
			m = (l + r) / 2;
			if (val < m) {
				dir = LEFT;
				r = m;
			} else {
				dir = RIGHT;
				l = m;
			}
			cur = cur->chd[dir];
		} else { // leaf
			cur->qty += qty;
			qty = 0;
		}
	}

	if (cap == self->next_compress_cap) {
		self->next_compress_cap *= 2;
		DEBUG("Before compress:\n");
		DEBUG_ACTION(qdigest_print(self, stdout));
		size_pair nh = tree_size(self->root);
		DEBUG("#nodes=%zu  height=%zu\n", nh.fst, nh.snd);
		qdigest_compress(self);
		DEBUG("After compress:\n");
		DEBUG_ACTION(qdigest_print(self, stdout));
		nh = tree_size(self->root);
		DEBUG("#nodes=%zu  height=%zu\n\n", nh.fst, nh.snd);
	}
}


size_t _sum_tree(qdnode *root)
{
	return (root) ?
	       root->qty + _sum_tree(root->chd[0]) + _sum_tree(root->chd[1]) : 0;
}


size_t qdigest_qry(qdigest *self, size_t val)
{
	qdnode *cur = self->root;
	size_t l = 0, r = self->range, m;
	size_t ret = 0;
	while( cur != NULL && HAS_CHD(cur) ) {
		m = (l + r) / 2;
		if (val < m) {
			r = m;
			cur = cur->chd[LEFT];
		} else {
			ret += _sum_tree(cur->chd[LEFT]);
			l = m;
			cur = cur->chd[RIGHT];
		}
	}
	return ret;
}


static void _print(FILE *stream, qdnode *root, size_t l, size_t r, size_t level)
{
	if (root == NULL ) return;
	for (int i=0; i<level; i++)  {
		fprintf(stream, "   ");
	}
	if (r-l==1) {
		fprintf(stream, "[val=%zu qty=%zu]\n", l, root->qty);
	} else {
		fprintf(stream, "[l=%zu r=%zu qty=%zu]\n", l, r, root->qty);
	}
	_print(stream, root->chd[LEFT], l, (l+r)/2, level+1);
	_print(stream, root->chd[RIGHT], (l+r)/2, r, level+1);
}


void qdigest_print(qdigest *self, FILE *stream)
{
	fprintf(stream, "QDigest @%p\n", self);
	fprintf(stream, "- range = [0..%zu)\n", self->range);
	fprintf(stream, "- err = %f\n", self->err);
	fprintf(stream, "- cap = %zu\n", qdigest_cap(self));
	fprintf(stream, "- total_qty = %zu\n", self->total_qty );
	_print(stream, self->root, 0, self->range, 0);
}