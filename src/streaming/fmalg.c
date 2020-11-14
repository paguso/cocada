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

#include "arrutil.h"
#include "bitbyte.h"
#include "errlog.h"
#include "fmalg.h"
#include "kwayrng.h"
#include "mathutil.h"
#include "new.h"


struct _fmalg {
	size_t n, m;
	kwayrng ***rng;
	uint64_t maxval;
	uint64_t p2ceil;
	byte_t **maxlsb;
	long double *avgs;
};


fmalg *fmalg_init_single(uint64_t maxval)
{
	return fmalg_init(maxval, 1, 1);
}


fmalg *fmalg_init(uint64_t maxval, size_t n, size_t m)
{
	assert(maxval <= 0x7FFFFFFFFFFFFFFF);
	fmalg *ret = NEW(fmalg);
	ret->maxval = maxval;
	ret->n = n;
	ret->m = m;
	ret->p2ceil = uint64_lobit( pow2ceil_uint64_t(maxval) );
	assert (ret->p2ceil <= 63);
	NEW_MATRIX(rngs, kwayrng*, m, n);
	FILL_MATRIX(rngs, m, n, kwayrng_new(2, ret->p2ceil));
	ret->rng = rngs;
	NEW_MATRIX_0(lsbs, byte_t, m, n);
	ret->maxlsb = lsbs;
	ret->avgs = NEW_ARR_0(long double, m);
	return ret;
}


void fmalg_free(fmalg *fm)
{
	for (size_t i = 0; i < fm->m; i++) {
		for (size_t j = 0; j < fm->n; j++) {
			kwayrng_free(fm->rng[i][j]);
		}
	}
	FREE(fm->rng);
	FREE(fm->maxlsb);
	FREE(fm);
}


void fmalg_reset(fmalg *fm)
{
	FILL_MATRIX(fm->maxlsb, fm->m, fm->n, 0);
}



void fmalg_process(fmalg *fm, uint64_t val)
{
	WARN_ASSERT(val < fm->maxval, "Ignoring invalid FM value %"PRIu64\
	            ". Max allowed value is %"PRIu64"", val, fm->maxval-1);
	uint64_t hashval;
	byte_t lsb;
	for (size_t i = 0; i < fm->m; i++) {
		for (size_t j = 0; j < fm->n; j++) {
			hashval = kwayrng_val(fm->rng[i][j], val);
			lsb = uint64_lobit( hashval );
			lsb = MIN(fm->p2ceil, lsb);
			fm->maxlsb[i][j] = MAX(lsb, fm->maxlsb[i][j]);
		}
	}

}


static long double pow_avg(byte_t *vals, size_t n)
{
	long double avg = 0;
	uint64_t acc = 0;
	for (size_t i = 0; i < n; i++) {
		if ((UINT64_MAX - acc) < vals[i]) {
			avg += (long double) acc / (long double) n;
			acc = 0;
		}
		acc += ( ( (uint64_t) 1 ) << vals[i] );
	}
	avg += (long double) acc / (long double) n;
	return avg;
}


uint64_t fmalg_query(fmalg *fm)
{
	FILL_ARR(fm->avgs, 0, fm->m, 0);
	for (size_t i = 0; i < fm->m; i++) {
		fm->avgs[i] = pow_avg(fm->maxlsb[i], fm->n);
	}
	PRINT_ARR(fm->avgs, "FM estimates ", "%Lf", 0, fm->m, fm->m );
	long double med =  median_ldouble(fm->avgs, fm->m, true);
	//PRINT_ARR(fm->avgs, "FM estimates after ", "%Lf", 0, fm->m, fm->m );
	return med;
}