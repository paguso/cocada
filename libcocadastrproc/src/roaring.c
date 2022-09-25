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
#include <stdio.h>

#include "bitarr.h"
#include "bitvec.h"
#include "errlog.h"
#include "mathutil.h"
#include "new.h"
#include "order.h"
#include "roaring.h"
#include "vec.h"
#include "segtree.h"


#define MAX_ARRAY_SIZE 4096
#define BITVEC_SIZE (1<<16)
#define CTNR_SIZE (1<<16)


#define MSB(u32_) ((u32_) >> 16)
#define LSB(u32_) ((u32_) & 0x0000FFFF)


typedef enum {
	EMPTY = 0,
	ARRAY_TYPE = 1,
	BITVEC_TYPE = 2
} ctnr_type;


typedef struct {
	ctnr_type type;
	uint32_t  card;
	void *data;
} ctnr_t;


struct _roaringbitvec {
	uint32_t size;
	uint32_t ncntrs;
	ctnr_t *ctnrs;
	segtree *count_st;
};


size_t roaringbitvec_memsize(roaringbitvec *self)
{
	size_t ret = sizeof(struct _roaringbitvec);
	ret += self->ncntrs * sizeof(ctnr_t);
	for (size_t i = 0; i < self->ncntrs; i++) {
		ctnr_t *ctnr = self->ctnrs + i;
		switch (ctnr->type) {
		case EMPTY:
			break;
		case ARRAY_TYPE:
			ret += vec_memsize(ctnr->data);
			break;
		case BITVEC_TYPE:
			ret += bitvec_memsize(ctnr->data);
			break;
		default:
			break;
		}
	}
	return ret;
}



void arrctnr_init(ctnr_t *ctnr)
{
	ctnr->type = ARRAY_TYPE;
	ctnr->data = vec_new_uint16_t();
}


size_t arrctnr_succ(ctnr_t *ctnr, uint16_t val)
{
	if (vec_len(ctnr->data) == 0 || val <= vec_first_uint16_t(ctnr->data)) {
		return 0;
	}
	else if (vec_last_uint16_t(ctnr->data) < val) {
		return vec_len(ctnr->data);
	}
	size_t l = 0, r = vec_len(ctnr->data) - 1; // successor in (l,r]
	while ((r - l) > 1) {
		size_t m = MEAN(l, r);
		if (val <= vec_get_uint16_t(ctnr->data, m)) {
			r = m;
		}
		else {
			l = m;
		}
	}
	return r;
}


bool arrctnr_get(ctnr_t *ctnr, uint16_t index)
{
	return vec_bsearch(ctnr->data, &index, cmp_uint16_t) < vec_len(ctnr->data);
}

// returns the change in the container size after this operation
// possible values: -1 (decrement), 0 (no effect), +1 (increment)
int arrctnr_set(ctnr_t *ctnr, uint16_t index, bool val)
{
	size_t pos = arrctnr_succ(ctnr, index);
	if ((pos == vec_len(ctnr->data))
	        || (vec_get_uint16_t(ctnr->data, pos) != index)) {
		if (val) {
			vec_ins_uint16_t(ctnr->data, pos, index);
			ctnr->card++;
			return +1;
		}
	}
	else {   // ctnr->data[pos] == val
		if (!val) {
			vec_del(ctnr->data, pos);
			ctnr->card--;
			return -1;
		}
	}
	return 0;
}


size_t arrctnr_rank(ctnr_t *ctnr, uint16_t index)
{
	return (uint16_t) arrctnr_succ(ctnr, index);
}


#define NZEROSUPTO(i) (vec_get_uint16_t(ctnr->data, (i)) - (i))

static uint32_t arrctnr_select(ctnr_t *ctnr, bool bit, uint32_t rank)
{
	if (bit) {
		return vec_get_uint16_t(ctnr->data, rank);
	} 
	else {
		uint32_t c0 = NZEROSUPTO(0);
		if ( rank < c0 ) {
			return rank;
		} 
		size_t l = 0, m;
		size_t r = vec_len(ctnr->data);
		while ((r - l) > 1) {
			m = MEAN(l, r);
			c0 = NZEROSUPTO(m);
			if ( rank < c0 ) {
				r = m;
			} else {
				l = m;
			}
		}
		c0 = NZEROSUPTO(l);
		return vec_get_int16_t(ctnr->data, l) + (rank - c0) + 1;
	}
}


void convert_arr_to_bitvec_ctnr(ctnr_t *ctnr)
{
	vec *v = ctnr->data;
	bitvec *b = bitvec_new_with_len(BITVEC_SIZE);
	for (size_t i = 0, l = vec_len(v); i < l; i++) {
		bitvec_set_bit(b, vec_get_uint16_t(v, i), 1);
	}
	ctnr->data = b;
	ctnr->type = BITVEC_TYPE;
	DESTROY_FLAT(v, vec);
}


void convert_bitvec_to_arr_ctnr(ctnr_t *ctnr)
{
	vec *v = vec_new_with_capacity(sizeof(uint16_t), ctnr->card);
	bitvec *b = ctnr->data;
	for (uint16_t i = 0; i < BITVEC_SIZE; i++) {
		if (bitvec_get_bit(b, i)) {
			vec_push_uint16_t(v, i);
		}
	}
	ctnr->data = v;
	ctnr->type = ARRAY_TYPE;
	bitvec_free(b);
}


void bitvecctnr_init(ctnr_t *ctnr)
{
	ctnr->type = BITVEC_TYPE;
	ctnr->data = bitvec_new_with_capacity(BITVEC_SIZE);
}


bool bitvecctnr_get(ctnr_t *ctnr, uint16_t index)
{
	return bitvec_get_bit((const bitvec *)ctnr->data, index);
}


// returns the change in the container size after this operation
// possible values: -1 (decrement), 0 (no effect), +1 (increment)
int bitvecctnr_set(ctnr_t *ctnr, uint16_t index, bool val)
{
	if (bitvec_get_bit(ctnr->data, index) != val) {
		bitvec_set_bit(ctnr->data, index, val);
		ctnr->card += ((val) ? 1 : -1);
		return val ? +1 : -1;
	}
	return 0;
}


uint16_t bitvecctnr_rank(ctnr_t *ctnr, uint16_t index)
{
	return bitvec_count_range(ctnr->data, 1, 0, index);
}

uint16_t bitvecctnr_select(ctnr_t *ctnr, bool bit, uint16_t rank)
{
	return (uint16_t)bitvec_select(ctnr->data, bit, rank);
}



static size_t ctnr_card(roaringbitvec *self,  size_t ctnr_index)
{
	return self->ctnrs[ctnr_index].card;
}


static const uint32_t ZERO32 = 0;

roaringbitvec *roaringbitvec_new(size_t n)
{
	roaringbitvec *ret = NEW(roaringbitvec);
	ret->size = n;
	ret->ncntrs = (size_t)DIVCEIL(n, BITVEC_SIZE);
	ret->ctnrs = calloc(ret->ncntrs, sizeof(ctnr_t));
	for (size_t i = 0; i < ret->ncntrs; i++) {
		ret->ctnrs[i] = (ctnr_t) {
			.type = EMPTY, .card = 0, .data = NULL
		};
	}
	ret->count_st = segtree_new(ret->ncntrs, sizeof(uint32_t),
	                            segtree_merge_sum_uint32_t, &ZERO32);
	return ret;
}


roaringbitvec *roaringbitvec_new_from_bitarr(byte_t *b, size_t n)
{
	setbuf(stdout, NULL);
	roaringbitvec *ret = roaringbitvec_new(n);
	for (size_t i = 0; i < n; i++) {
		if (bitarr_get_bit(b, i)) {
			roaringbitvec_set(ret, i, 1);
		}
	}
	return ret;
}


void roaringbitvec_free(roaringbitvec *self)
{
	for (size_t i = 0; i < self->ncntrs; i++) {
		switch (self->ctnrs[i].type) {
		case ARRAY_TYPE:
			DESTROY_FLAT(self->ctnrs[i].data, vec);
			break;
		case BITVEC_TYPE:
			bitvec_free(self->ctnrs[i].data);
			break;
		default:
			break;
		}
	}
	FREE(self->ctnrs);
	segtree_free(self->count_st);
	FREE(self);
}


uint32_t roaringbitvec_card(roaringbitvec *self)
{
	return self->size ? segtree_range_qry_uint32_t(self->count_st, 0, self->ncntrs) : 0;
}


uint32_t roaringbitvec_count(roaringbitvec *self, bool bit)
{
	return bit ? roaringbitvec_card(self) : self->size - roaringbitvec_card(self);
}


void roaringbitvec_set(roaringbitvec *self, uint32_t pos, bool val)
{
	assert(pos < self->size);
	uint16_t bucket = MSB(pos);
	uint16_t index = LSB(pos);
	ctnr_t *ctnr = self->ctnrs + bucket;
	uint16_t old_card = ctnr_card(self, bucket);
	int card_incr = 0;
	switch (ctnr->type) {
	case EMPTY:
		if (val) {
			arrctnr_init(ctnr);
		}
		else {
			break;
		}
	case ARRAY_TYPE:
		card_incr = arrctnr_set(ctnr, index, val);
		if (ctnr->card > MAX_ARRAY_SIZE) {
			convert_arr_to_bitvec_ctnr(ctnr);
		}
		break;
	case BITVEC_TYPE:
		card_incr = bitvecctnr_set(ctnr, index, val);
		if (ctnr->card <= MAX_ARRAY_SIZE) {
			convert_bitvec_to_arr_ctnr(ctnr);
		}
		break;
	default:
		break;
	}
	if (card_incr) {
		segtree_upd_uint32_t(self->count_st, bucket, old_card + card_incr);
	}
}


bool roaringbitvec_get(roaringbitvec *self, uint32_t pos)
{
	assert(pos < self->size);
	uint16_t bucket = MSB(pos);
	uint16_t index = LSB(pos);
	ctnr_t *ctnr = self->ctnrs + bucket;
	switch (ctnr->type) {
	case EMPTY:
		return 0;
		break;
	case ARRAY_TYPE:
		return arrctnr_get(ctnr, index);
		break;
	case BITVEC_TYPE:
		return bitvecctnr_get(ctnr, index);
		break;
	default:
		ERROR("Invalid container type");
		return 0;
		break;
	}
}



void roaringbitvec_fit(roaringbitvec *self)
{
	for (size_t i = 0; i < self->ncntrs; i++) {
		ctnr_t *ctnr = self->ctnrs + i;
		switch (ctnr->type) {
		case EMPTY:
			break;
		case ARRAY_TYPE:
			vec_fit(ctnr->data);
			break;
		case BITVEC_TYPE:
			bitvec_fit(ctnr->data);
			break;
		default:
			break;
		}
	}
}


uint32_t roaringbitvec_rank1(roaringbitvec *self, uint32_t pos)
{
	pos = MIN(self->size, pos);
	uint16_t bucket = MSB(pos);
	uint16_t index = LSB(pos);
	uint32_t ret = 0;
	ret += segtree_range_qry_uint32_t(self->count_st, 0, bucket);
	ctnr_t *ctnr = self->ctnrs + bucket;
	switch (ctnr->type) {
	case EMPTY:
		break;
	case ARRAY_TYPE:
		ret += arrctnr_rank(ctnr, index);
		break;
	case BITVEC_TYPE:
		ret += bitvecctnr_rank(ctnr, index);
		break;
	default:
		break;
	}
	return ret;
}


uint32_t roaringbitvec_rank0(roaringbitvec *self, uint32_t pos)
{
    return MIN(self->size, pos) - roaringbitvec_rank1(self, pos);
}


uint32_t roaringbitvec_rank(roaringbitvec *self, bool bit, uint32_t pos)
{
	return bit ? roaringbitvec_rank1(self, pos) : roaringbitvec_rank0(self, pos);
}



#define BKTRANK0(b) MIN(self->size, (b) * CTNR_SIZE) - segtree_range_qry_uint32_t(self->count_st, 0, b)
#define BKTRANK1(b) segtree_range_qry_uint32_t(self->count_st, 0, (b))
#define BKTRANK(bkt, bit) ((bit) ? BKTRANK1((bkt)) : BKTRANK0((bkt)))

uint32_t roaringbitvec_select(roaringbitvec *self, bool bit, uint32_t rank)
{
	
	if (rank >= roaringbitvec_count(self, bit)) { // self->size || rank > BKTRANK(self->ncntrs - 1, bit)) {
		return self->size;
	}
	// find the bucket on which to look for the right bit
	uint32_t l = 0, r = self->ncntrs, bkt_rank = 0; 
	while ( (r - l) > 1) { // bucket in [l, r)
		uint32_t m = MEAN(l, r);
		bkt_rank = BKTRANK(m, bit);
		if (rank < bkt_rank) {
			r =  m;
		} else {
			l = m;
		}
	}
	bkt_rank = BKTRANK(l, bit);
	size_t ret = l * CTNR_SIZE;
	ctnr_t *ctnr = self->ctnrs + l;
	assert(rank >= bkt_rank);
	switch (ctnr->type) {
	case EMPTY:
		ret += (rank - bkt_rank);
		break;
	case ARRAY_TYPE:
		ret += arrctnr_select(ctnr, bit, rank - bkt_rank);
		break;
	case BITVEC_TYPE:
		ret += bitvecctnr_select(ctnr, bit, rank - bkt_rank);
		break;
	default:
		ERROR("Invalid container type\n");
		return self->size;
		break;
	}
	return ret;
}


uint32_t roaringbitvec_select0(roaringbitvec *self, uint32_t rank)
{
	return roaringbitvec_select(self, 0, rank);
}


uint32_t roaringbitvec_select1(roaringbitvec *self, uint32_t rank)
{
	return roaringbitvec_select(self, 1, rank);
}

void roaringbitvec_fprint(FILE *stream, roaringbitvec *self)
{
	char *types[3] = {"EMPTY", "ARRAY", "BITVEC"};
	fprintf(stream, "roaringbitvec@%p {\n", self);
	fprintf(stream, "   size=%"PRIu32"\n", self->size);
	for (size_t b = 0; b < self->ncntrs; b++) {
		fprintf(stream, "   [%zu] type=%s card=%"PRIu32"\n", b, 
                types[self->ctnrs[b].type],
		        self->ctnrs[b].card);
	}
	fprintf(stream, "}\n");
}
