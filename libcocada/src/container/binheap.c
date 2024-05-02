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

#include <string.h>
#include <stddef.h>

#include "coretype.h"
#include "vec.h"
#include "binheap.h"
#include "new.h"
#include "order.h"


struct _BinHeap {
	Vec *data;
	CmpFunc cmp;
};

BinHeap *binheap_new( usize typesize,
                      CmpFunc cmp )
{
	BinHeap *heap = NEW(BinHeap);
	heap->data = vec_new(typesize);
	heap->cmp = cmp;
	return heap;
}


void binheap_finalise(void *ptr, const Finaliser *fnr)
{
	BinHeap *heap = (BinHeap *)ptr;
	vec_finalise(heap->data, fnr);
	FREE(heap->data);
}


usize binheap_size(const BinHeap *heap)
{
	return vec_len(heap->data);
}


static usize _bubble_up(BinHeap *heap)
{
	usize i = binheap_size(heap) - 1;
	while ( i > 0 &&
	        heap->cmp( vec_get(heap->data, i),
	                   vec_get(heap->data, (i - 1) / 2) ) > 0 )  {
		vec_swap(heap->data, i, (i - 1) / 2);
		i = (i - 1) / 2;
	}
	return i;
}


static usize _bubble_down(BinHeap *heap, usize pos)
{
	usize i, l, r, m, n;
	n = binheap_size(heap);
	i = pos;
	while (true) {
		m = i;
		l = (2 * i) + 1;
		r = (2 * i) + 2;
		if ( l < n &&
		        heap->cmp( vec_get(heap->data, l),
		                   vec_get(heap->data, m) ) > 0 ) {
			m = l;
		}
		if ( r < n &&
		        heap->cmp( vec_get(heap->data, r),
		                   vec_get(heap->data, m) ) > 0 ) {
			m = r;
		}
		if ( m != i ) {
			vec_swap(heap->data, i, m);
			i = m;
		}
		else {
			break;
		}
	}
	return i;
}


void binheap_ins(BinHeap *heap, const void *elt)
{
	vec_push(heap->data, elt);
	_bubble_up(heap);
}


void binheap_remv(BinHeap *heap, void *dest)
{
	if (vec_len(heap->data) == 0) return;
	vec_swap(heap->data, 0, vec_len(heap->data) - 1);
	vec_pop(heap->data, vec_len(heap->data) - 1, dest);
	if (vec_len(heap->data) > 0)
		_bubble_down(heap, 0);
}


#define BINHEAP_PUSH_IMPL( TYPE )\
	void binheap_ins_##TYPE(BinHeap *heap, TYPE val)\
	{   binheap_ins(heap, &val);   }


#define BINHEAP_POP_IMPL( TYPE )\
	TYPE binheap_remv_##TYPE(BinHeap *heap)\
	{   TYPE s; binheap_remv(heap, &s); return s; }


#define BINHEAP_ALL_IMPL( TYPE , ...)\
	BINHEAP_PUSH_IMPL(TYPE)\
	BINHEAP_POP_IMPL(TYPE)

XX_CORETYPES(BINHEAP_ALL_IMPL)
