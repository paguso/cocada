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
#include <inttypes.h>

#include "arrays.h"
#include "bitbyte.h"
#include "errlog.h"
#include "fmalg.h"
#include "kwayrng.h"
#include "mathutil.h"
#include "new.h"


struct _FMAlg {
	usize n, m;
	KWayRNG ***rng;
	uint64 maxval;
	uint64 p2ceil;
	byte **maxlsb;
	long double *avgs;
};


FMAlg *fmalg_init_single(uint64 maxval)
{
	return fmalg_init(maxval, 1, 1);
}


FMAlg *fmalg_init(uint64 maxval, usize n, usize m)
{
	assert(maxval <= 0x7FFFFFFFFFFFFFFF);
	FMAlg *ret = NEW(FMAlg);
	ret->maxval = maxval;
	ret->n = n;
	ret->m = m;
	ret->p2ceil = uint64_lobit( pow2ceil_uint64(maxval) );
	assert (ret->p2ceil <= 63);
	NEW_MATRIX(rngs, KWayRNG *, m, n);
	FILL_MATRIX(rngs, m, n, kwayrng_new(2, ret->p2ceil));
	ret->rng = rngs;
	NEW_MATRIX_0(lsbs, byte, m, n);
	ret->maxlsb = lsbs;
	ret->avgs = ARR_OF_0_NEW(long double, m);
	return ret;
}


void fmalg_free(FMAlg *fm)
{
	for (usize i = 0; i < fm->m; i++) {
		for (usize j = 0; j < fm->n; j++) {
			kwayrng_free(fm->rng[i][j]);
		}
	}
	FREE(fm->rng);
	FREE(fm->maxlsb);
	FREE(fm);
}


void fmalg_reset(FMAlg *fm)
{
	FILL_MATRIX(fm->maxlsb, fm->m, fm->n, 0);
}



void fmalg_process(FMAlg *fm, uint64 val)
{
	WARN_ASSERT(val < fm->maxval, "Ignoring invalid FM value %"PRIu64\
	            ". Max allowed value is %"PRIu64"", val, fm->maxval - 1);
	uint64 hashval;
	byte lsb;
	for (usize i = 0; i < fm->m; i++) {
		for (usize j = 0; j < fm->n; j++) {
			hashval = kwayrng_val(fm->rng[i][j], val);
			lsb = uint64_lobit( hashval );
			lsb = MIN(fm->p2ceil, lsb);
			fm->maxlsb[i][j] = MAX(lsb, fm->maxlsb[i][j]);
		}
	}

}


static long double pow_avg(byte *vals, usize n)
{
	long double avg = 0;
	uint64 acc = 0;
	for (usize i = 0; i < n; i++) {
		if ((UINT64_MAX - acc) < vals[i]) {
			avg += (long double) acc / (long double) n;
			acc = 0;
		}
		acc += ( ( (uint64) 1 ) << vals[i] );
	}
	avg += (long double) acc / (long double) n;
	return avg;
}


uint64 fmalg_query(FMAlg *fm)
{
	ARR_FILL(fm->avgs, 0, fm->m, 0);
	for (usize i = 0; i < fm->m; i++) {
		fm->avgs[i] = pow_avg(fm->maxlsb[i], fm->n);
	}
	long double med =  median_ldouble(fm->avgs, fm->m, true);
	return med;
}
