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
#include <stdint.h>

#include "vec.h"
#include "arrutil.h"
#include "binheap.h"
#include "new.h"
#include "order.h"


struct _binheap {
	vec *data;
	cmp_func cmp;
};

binheap *binheap_new( cmp_func cmp,
                      size_t typesize )
{
	binheap *heap = NEW(binheap);
	heap->data = vec_new(typesize);
	heap->cmp = cmp;
	return heap;
}


void binheap_free(binheap *heap, bool free_elements)
{
	if (heap == NULL) return;
	vec_free(heap->data, free_elements);
	FREE(heap);
}


void binheap_dispose(void *ptr, const dtor *dt)
{
	binheap *heap = (binheap *)ptr;
	vec_dispose(heap->data, dt);
	FREE(heap->data);
}


size_t binheap_size(const binheap *heap)
{
	return vec_len(heap->data);
}


static size_t _bubble_up(binheap *heap)
{
	size_t i=binheap_size(heap)-1;
	while ( i>0 &&
			heap->cmp( vec_get(heap->data, i),
						vec_get(heap->data, (i-1)/2) ) > 0 )  {
		vec_swap(heap->data, i, (i-1)/2);
		i = (i-1)/2;
	}
	return i;
}


static size_t _bubble_down(binheap *heap, size_t pos)
{
	size_t i, l, r, m, n;
	n = binheap_size(heap);
	i = pos;
	while (true) {
		m = i;
		l = (2*i)+1;
		r = (2*i)+2;
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
		} else {
			break;
		}
	}
	return i;
}


void binheap_push(binheap *heap, const void *elt)
{
	vec_push(heap->data, elt);
	_bubble_up(heap);
}


void binheap_pop(binheap *heap, void *dest)
{
	if (vec_len(heap->data)==0) return;
	vec_swap(heap->data, 0, vec_len(heap->data)-1);
	vec_pop(heap->data, vec_len(heap->data)-1, dest);
	if (vec_len(heap->data) > 0)
		_bubble_down(heap, 0);
}


#define BINHEAP_PUSH_IMPL( TYPE )\
    void binheap_push_##TYPE(binheap *heap, TYPE val)\
        {   binheap_push(heap, &val);   }


#define BINHEAP_POP_IMPL( TYPE )\
    TYPE binheap_pop_##TYPE(binheap *heap)\
        {   TYPE s; binheap_pop(heap, &s); return s; }


#define BINHEAP_ALL_IMPL( TYPE )\
BINHEAP_PUSH_IMPL(TYPE)\
BINHEAP_POP_IMPL(TYPE)


BINHEAP_ALL_IMPL(byte_t)
BINHEAP_ALL_IMPL(char)
BINHEAP_ALL_IMPL(short)
BINHEAP_ALL_IMPL(int)
BINHEAP_ALL_IMPL(long)
BINHEAP_ALL_IMPL(float)
BINHEAP_ALL_IMPL(double)
BINHEAP_ALL_IMPL(size_t)
BINHEAP_ALL_IMPL(int8_t)
BINHEAP_ALL_IMPL(int16_t)
BINHEAP_ALL_IMPL(int32_t)
BINHEAP_ALL_IMPL(int64_t)
BINHEAP_ALL_IMPL(uint8_t)
BINHEAP_ALL_IMPL(uint16_t)
BINHEAP_ALL_IMPL(uint32_t)
BINHEAP_ALL_IMPL(uint64_t)
