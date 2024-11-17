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

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "errlog.h"
#include "gk.h"
#include "memdbg.h"
#include "new.h"
#include "strbuf.h"
#include "vec.h"


typedef struct {
	usize qty;
	usize delta;
} GKQuantity ;


struct _GKSumm {
	Vec *vals;
	Vec *qtys;
	CmpFunc cmp;
	double err;
	usize total_qty;
};


GKSumm *gk_new(usize typesize, CmpFunc cmp, double err)
{
	GKSumm *ret = NEW(GKSumm);
	ret->vals = vec_new(typesize);
	ret->qtys = vec_new(sizeof(GKQuantity));
	void *inf = malloc(typesize);
	memset(inf, ~0, typesize);
	vec_push(ret->vals, inf);
	FREE(inf);
	GKQuantity inf_qty = {.qty = 1, .delta = 0};
	vec_push(ret->qtys, &inf_qty);
	ret->err = err;
	ret->cmp = cmp;
	ret->total_qty = 0;
	return ret;
}


static usize succ(Vec *data, CmpFunc cmp, const void *val)
{
	if ( vec_len(data) == 0
	        || cmp(val, vec_get(data, 0)) < 0 ) { // treat last elt as INFINITY
		return 0;
	}
	else {
		usize l = 0;
		usize r = vec_len(data) - 1;
		while ( r - l > 1 ) {
			usize m = (l + r) / 2;
			if ( cmp(val, vec_get(data, m)) < 0 ) {
				r = m;
			}
			else {
				l = m;
			}
		}
		return r;
	}
}


void gk_upd(GKSumm *self, const void *val)
{
	self->total_qty++;
	usize succ_pos = succ(self->vals, self->cmp, val);
	GKQuantity *succ_qty = (GKQuantity *) vec_get(self->qtys, succ_pos);
	const usize qty_thres = ceil(2.0 * self->err * self->total_qty);
	if ( succ_qty->qty + succ_qty->delta + 1 < qty_thres ) {
		succ_qty->qty++;
	}
	else {
		vec_ins(self->vals, succ_pos, val);
		GKQuantity new_qty = {.qty = 1, .delta = succ_qty->qty + succ_qty->delta - 1};
		vec_ins(self->qtys, succ_pos, &new_qty);

		GKQuantity *ith_qty = (GKQuantity *) vec_get(self->qtys, 0);
		GKQuantity *iplus1th_qty;
		for (usize i = 0, l = vec_len(self->vals); i < l - 1; i++ ) {
			iplus1th_qty = (GKQuantity *) vec_get(self->qtys, i + 1);
			if (ith_qty->qty + iplus1th_qty->qty + iplus1th_qty->delta < qty_thres) {
				iplus1th_qty->qty += ith_qty->qty;
				vec_del(self->vals, i);
				vec_del(self->qtys, i);
				break;
			}
			ith_qty = iplus1th_qty;
		}
	}
}


void gk_merge(GKSumm *self, const GKSumm *other)
{
	ERROR_ASSERT( self->cmp == other->cmp
	              && self->err == other->err
	              && vec_typesize(self->vals) == vec_typesize(other->vals),
	              "Incompatible GK sketches." );
	usize i = 0, j = 0;
	GKQuantity *i_qty = (GKQuantity *) vec_get(self->qtys, i);
	GKQuantity *j_qty = (GKQuantity *) vec_get(other->qtys, j);
	while (i < ( vec_len(self->vals) - 1 ) && j < ( vec_len(other->vals) - 1 ) ) {
		if ( self->cmp(vec_get(self->vals, i), vec_get(other->vals, j)) <= 0 ) {
			i_qty->delta += (j_qty->qty + j_qty->delta - 1 );
			i++;
			i_qty = (GKQuantity *) vec_get(self->qtys, i);
		}
		else {
			vec_ins(self->vals, i, vec_get(other->vals, j));
			GKQuantity new_qty = {.qty = j_qty->qty, .delta = (j_qty->delta + i_qty->qty + i_qty->delta - 1)};
			vec_ins(self->qtys, i, &new_qty);
			i++;
			i_qty = (GKQuantity *) vec_get(self->qtys, i);
			j++;
			j_qty = (GKQuantity *) vec_get(other->qtys, j);
		}
	}
	while (j < ( vec_len(other->vals) - 1 )) {
		vec_ins(self->vals, i, vec_get(other->vals, j));
		vec_ins(self->qtys, i, j_qty);
		i++;
		j++;
		j_qty = (GKQuantity *) vec_get(other->qtys, j);
	}
	self->total_qty += other->total_qty;
	const usize qty_thres = ceil(2.0 * self->err * self->total_qty);
	i = 0;
	while ( i < vec_len(self->vals) - 1 ) {
		GKQuantity *ith_qty = (GKQuantity *) vec_get(self->qtys, i);
		GKQuantity *iplus1th_qty = (GKQuantity *) vec_get(self->qtys, i + 1);
		if (ith_qty->qty + iplus1th_qty->qty + iplus1th_qty->delta < qty_thres) {
			iplus1th_qty->qty += ith_qty->qty;
			vec_del(self->vals, i);
			vec_del(self->qtys, i);
		}
		else {
			i++;
		}
	}
}


usize gk_rank(GKSumm *self, const void *val)
{
	if (vec_len(self->vals) == 1) {
		return 0;
	}
	usize succ_pos = succ(self->vals, self->cmp, val);
	GKQuantity *succ_qty = (GKQuantity *) vec_get(self->qtys, succ_pos);
	usize ret = 0;
	for (usize i = 0; i < succ_pos; i++) {
		ret += ((GKQuantity *)vec_get(self->qtys, i))->qty;
	}
	return ret - 1 + (succ_qty->qty + succ_qty->delta) / 2;
}


void gk_print(GKSumm *self, FILE *stream, void (*print_val)(FILE *,
              const void *))
{
	usize l = vec_len(self->vals);
	for (usize i = 0; i < l - 1; i++) {
		fprintf(stream, "(");
		print_val(stream, vec_get(self->vals, i));
		fprintf(stream, ", ");
		GKQuantity *q = (GKQuantity *) vec_get(self->qtys, i);
		fprintf(stream, "%zu, %zu) ", q->qty, q->delta);
	}
	GKQuantity *q = (GKQuantity *) vec_get(self->qtys, l - 1);
	fprintf(stream, "(INF, %zu, %zu)", q->qty, q->delta);
}
