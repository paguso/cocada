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

#include "arrays.h"
#include "bjkst.h"
#include "coretype.h"
#include "errlog.h"
#include "hash.h"
#include "hashset.h"
#include "mathutil.h"
#include "twuhash.h"
#include "new.h"


struct _bjkst {
	size_t nbits;      // size of stream elts in bits
	uint64_t max_val;  // max allowed stream value = 1<<nbits
	double eps;        // error
	double delta;      // error prob
	twuhash *g;        // 2-way indep hash function
	size_t buf_cap;    // buffer capacity
	size_t buf_size;   // current buffer size
	hashset **buf;     // buffers. one for each nb of trailing zeros
	size_t min_zeros;  // min nb of trailing zeros currently being accounted for
};


static size_t get_buf_cap(double eps, double delta)
{
	return  (size_t)(1.0 / (eps * eps));
}


bjkst* bjkst_init(size_t nbits, double eps, double delta)
{
	ERROR_ASSERT( 0 < nbits && nbits < 64,
	              "BJKST: Allowed #bits range is 1..63.");
	bjkst *ret = NEW(bjkst);
	ret->nbits = nbits;
	ret->max_val = (1 << nbits);
	ret->eps = eps;
	ret->delta = delta;
	ret->g = twuhash_new(nbits, nbits);
	ret->buf = NEW_ARR(hashset*, nbits + 1);
	ret->buf_cap = get_buf_cap(eps, delta);
	ret->buf_size = 0;
	ret->min_zeros = 0;
	FILL_ARR(ret->buf, 0, nbits + 1, \
	         hashset_new(64, ident_hash_uint64_t, eq_uint64_t));
	return ret;
}



void bjkst_process(bjkst *counter, uint64_t val)
{
	WARN_ASSERT( val < counter->max_val,
	             "BJKST: Ignoring out-of-range value %"PRIu64"."
	             " Allowed range is [0,%"PRIu64").", val, counter->max_val);
	uint64_t hval = twuhash_hash(counter->g, val);
	ERROR_ASSERT( hval < counter->max_val, "BJKST: invalid hvalue." );
	size_t zeros = uint64_lobit(hval);
	zeros = MIN(zeros, counter->nbits);
	if ( zeros < counter->min_zeros
	        || hashset_contains_uint64_t(counter->buf[zeros], hval)) return;
	// make sure free space is available
	while (counter->buf_size >= counter->buf_cap ) {//&& counter->min_zeros <= counter->nbits) {
		counter->buf_size -= hashset_size(counter->buf[counter->min_zeros]);
		FREE(counter->buf[counter->min_zeros], hashset);
		counter->buf[counter->min_zeros] = NULL;
		counter->min_zeros++;
	}
	// insert new element, if possible
	if ( zeros < counter->min_zeros ) return;
	hashset_add_uint64_t(counter->buf[zeros], hval);
	counter->buf_size++;
}


uint64_t bjkst_qry(bjkst *counter)
{
	size_t min_nonempty_zeros = counter->min_zeros;
	while(min_nonempty_zeros < counter->nbits
	        && hashset_size(counter->buf[min_nonempty_zeros])==0 ) {
		min_nonempty_zeros++;
	}
	uint64_t ret = (1 << min_nonempty_zeros);
	ret *= counter->buf_size;
	return ret;
}