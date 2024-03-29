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
#include "memdbg.h"


struct _finaliser {
	finalise_func fn;
	size_t nchd;
	struct _finaliser **chd;
};


finaliser *finaliser_new( finalise_func fn )
{
	finaliser *fnr = NEW(finaliser);
	fnr->fn = fn;
	fnr->nchd = 0;
	fnr->chd = NULL;
	return fnr;
}


finaliser *finaliser_clone(const finaliser *src)
{
	if (src == NULL) {
		return NULL;
	}
	finaliser *fnr = NEW(finaliser);
	fnr->fn = src->fn;
	fnr->nchd = src->nchd;
	fnr->chd = (finaliser **) calloc(src->nchd, sizeof(finaliser *));
	for (size_t i = 0; i < src->nchd; i++ ) {
		fnr->chd[i] = finaliser_clone(src->chd[i]);
	}
	return fnr;
}


void finaliser_free(finaliser *self)
{
	if (self == NULL) return;
	for (size_t i = 0; i < self->nchd; i++) {
		finaliser_free(self->chd[i]);
	}
	FREE(self->chd);
	FREE(self);
}


void finaliser_call(const finaliser *self, void *ptr)
{
	self->fn(ptr, (finaliser *)self);
}


size_t finaliser_nchd(const finaliser *self)
{
	return self->nchd;
}


const finaliser *finaliser_chd(const finaliser *self, size_t index)
{
	return ((finaliser **)self->chd)[index];
}


finaliser *finaliser_cons(finaliser *par, const finaliser *chd)
{
	par->chd = (finaliser **) realloc(par->chd,
	                                  ( par->nchd + 1) * sizeof(finaliser *));
	par->chd[par->nchd++] =  (finaliser *)chd;
	return par;
}


static void _empty_finalise(void *ptr, const finaliser *fnr ) {}


finaliser *finaliser_new_empty()
{
	finaliser *ret = NEW(finaliser);
	ret->fn = _empty_finalise;
	ret->nchd = 0;
	ret->chd = NULL;
	return ret;
}


// ptr is a pointer to an object reference (pointer)
static void _ptr_finalise(void *ptr, const finaliser *fr )
{
	void *pointee = *((void **)ptr);
	if ( finaliser_nchd(fr) > 0 )
		FINALISE( pointee, finaliser_chd(fr, 0) );
	FREE(pointee);
	*((void **)ptr) = NULL;
}


finaliser *finaliser_new_ptr()
{
	finaliser *ret = NEW(finaliser);
	ret->fn = _ptr_finalise;
	ret->nchd = 0;
	ret->chd = NULL;
	return ret;
}


finaliser *finaliser_new_ptr_to_obj(const finaliser *chd)
{
	return finaliser_cons(finaliser_new_ptr(), chd);
}