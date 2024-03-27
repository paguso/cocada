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
	size_t qty;
	size_t delta;
} gk_qty ;


struct __gksumm {
	vec *vals;
	vec *qtys;
	cmp_func cmp;
	double err;
	size_t total_qty;
};


gksumm *gk_new(size_t typesize, cmp_func cmp, double err)
{
	gksumm *ret = NEW(gksumm);
	ret->vals = vec_new(typesize);
	ret->qtys = vec_new(sizeof(gk_qty));
	void *inf = malloc(typesize);
	memset(inf, ~0, typesize);
	vec_push(ret->vals, inf);
	FREE(inf);
	gk_qty inf_qty = {.qty = 1, .delta = 0};
	vec_push(ret->qtys, &inf_qty);
	ret->err = err;
	ret->cmp = cmp;
	ret->total_qty = 0;
	return ret;
}


static size_t succ(vec *data, cmp_func cmp, const void *val)
{
	if ( vec_len(data) == 0
	        || cmp(val, vec_get(data, 0)) < 0 ) { // treat last elt as INFINITY
		return 0;
	}
	else {
		size_t l = 0;
		size_t r = vec_len(data) - 1;
		while ( r - l > 1 ) {
			size_t m = (l + r) / 2;
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


void gk_upd(gksumm *self, const void *val)
{
	self->total_qty++;
	size_t succ_pos = succ(self->vals, self->cmp, val);
	gk_qty *succ_qty = (gk_qty *) vec_get(self->qtys, succ_pos);
	const size_t qty_thres = ceil(2.0 * self->err * self->total_qty);
	if ( succ_qty->qty + succ_qty->delta + 1 < qty_thres ) {
		succ_qty->qty++;
	}
	else {
		vec_ins(self->vals, succ_pos, val);
		gk_qty new_qty = {.qty = 1, .delta = succ_qty->qty + succ_qty->delta - 1};
		vec_ins(self->qtys, succ_pos, &new_qty);

		gk_qty *ith_qty = (gk_qty *) vec_get(self->qtys, 0);
		gk_qty *iplus1th_qty;
		for (size_t i = 0, l = vec_len(self->vals); i < l - 1; i++ ) {
			iplus1th_qty = (gk_qty *) vec_get(self->qtys, i + 1);
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


void gk_merge(gksumm *self, const gksumm *other)
{
	ERROR_ASSERT( self->cmp == other->cmp
	              && self->err == other->err
	              && vec_typesize(self->vals) == vec_typesize(other->vals),
	              "Incompatible GK sketches." );
	size_t i = 0, j = 0;
	gk_qty *i_qty = (gk_qty *) vec_get(self->qtys, i);
	gk_qty *j_qty = (gk_qty *) vec_get(other->qtys, j);
	while (i < ( vec_len(self->vals) - 1 ) && j < ( vec_len(other->vals) - 1 ) ) {
		if ( self->cmp(vec_get(self->vals, i), vec_get(other->vals, j)) <= 0 ) {
			i_qty->delta += (j_qty->qty + j_qty->delta - 1 );
			i++;
			i_qty = (gk_qty *) vec_get(self->qtys, i);
		}
		else {
			vec_ins(self->vals, i, vec_get(other->vals, j));
			gk_qty new_qty = {.qty = j_qty->qty, .delta = (j_qty->delta + i_qty->qty + i_qty->delta - 1)};
			vec_ins(self->qtys, i, &new_qty);
			i++;
			i_qty = (gk_qty *) vec_get(self->qtys, i);
			j++;
			j_qty = (gk_qty *) vec_get(other->qtys, j);
		}
	}
	while (j < ( vec_len(other->vals) - 1 )) {
		vec_ins(self->vals, i, vec_get(other->vals, j));
		vec_ins(self->qtys, i, j_qty);
		i++;
		j++;
		j_qty = (gk_qty *) vec_get(other->qtys, j);
	}
	self->total_qty += other->total_qty;
	const size_t qty_thres = ceil(2.0 * self->err * self->total_qty);
	i = 0;
	while ( i < vec_len(self->vals) - 1 ) {
		gk_qty *ith_qty = (gk_qty *) vec_get(self->qtys, i);
		gk_qty *iplus1th_qty = (gk_qty *) vec_get(self->qtys, i + 1);
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


size_t gk_rank(gksumm *self, const void *val)
{
	if (vec_len(self->vals) == 1) {
		return 0;
	}
	size_t succ_pos = succ(self->vals, self->cmp, val);
	gk_qty *succ_qty = (gk_qty *) vec_get(self->qtys, succ_pos);
	size_t ret = 0;
	for (size_t i = 0; i < succ_pos; i++) {
		ret += ((gk_qty *)vec_get(self->qtys, i))->qty;
	}
	return ret - 1 + (succ_qty->qty + succ_qty->delta) / 2;
}


void gk_print(gksumm *self, FILE *stream, void (*print_val)(FILE *,
              const void *))
{
	size_t l = vec_len(self->vals);
	for (size_t i = 0; i < l - 1; i++) {
		fprintf(stream, "(");
		print_val(stream, vec_get(self->vals, i));
		fprintf(stream, ", ");
		gk_qty *q = (gk_qty *) vec_get(self->qtys, i);
		fprintf(stream, "%zu, %zu) ", q->qty, q->delta);
	}
	gk_qty *q = (gk_qty *) vec_get(self->qtys, l - 1);
	fprintf(stream, "(INF, %zu, %zu)", q->qty, q->delta);
}