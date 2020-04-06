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

#include <stdlib.h>
#include <stdio.h>

#include "new.h"


dtor *dtor_new_with_func( strbuf_func df )
{
	dtor *dt = NEW(dtor);
	dt->df = df;
	dt->nchd = 0;
	dt->chd = NULL;//calloc(1, sizeof(dtor *));
	return dt;
}


void dtor_free(dtor *dt)
{
	if (dt==NULL) return;
	for (size_t i=0; i<dt->nchd; i++) {
		dtor_free(dt->chd[i]);
	}
	FREE(dt->chd);
	FREE(dt);
}

size_t dtor_nchd(const dtor *dt)
{
	return dt->nchd;
}

const dtor *dtor_chd(const dtor *par, size_t index)
{
	return ((dtor **)par->chd)[index];
}


dtor *dtor_cons(dtor *par, const dtor *chd)
{
	par->chd = (dtor **) realloc(par->chd, ( par->nchd + 1) * sizeof(dtor *));
	par->chd[par->nchd++] =  (dtor *)chd;
	return par;
}


static void _empty_free(void *ptr, const dtor *dt )
{
}


dtor *empty_dtor()
{
	dtor *ret = NEW(dtor);
	ret->df = _empty_free;
	ret->nchd = 0;
	ret->chd = NULL;
	return ret;
}


static void _ptr_free(void *ptr, const dtor *dt )
{
	void *pointee = *((void **)ptr);
	if ( dtor_nchd(dt) > 0 )
		FINALISE( pointee, dtor_chd(dt, 0) );
	FREE(pointee);
	*((void **)ptr) = NULL;
}


dtor *ptr_dtor()
{
	dtor *ret = NEW(dtor);
	ret->df = _ptr_free;
	ret->nchd = 0;
	ret->chd = NULL;
	return ret;
}
