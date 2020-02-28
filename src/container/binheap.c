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

#include "vec.h"
#include "arrayutil.h"
#include "binheap.h"
#include "cocadautil.h"


struct _binheap {
    heap_mode mode;
    vec *data;
    int (*comp_fn)(const void *, const void *);
};

binheap *binheap_new( int (*comp_fn)(const void *, const void *), 
                      size_t typesize, heap_mode mode ) 
{
    binheap *heap = NEW(binheap);
    heap->mode = mode;
    heap->data = vec_new(typesize);
    heap->comp_fn = comp_fn;
    return heap;
}


void binheap_free(binheap *heap, bool free_elements)
{
    if (heap == NULL) return;
    vec_free(heap->data, free_elements);
    FREE(heap);
}


size_t binheap_size(binheap *heap)
{
    return vec_len(heap->data);
}


static size_t bubble_up(binheap *heap)
{
    size_t i=binheap_size(heap)-1;
    switch (heap->mode) {
    case MIN_HEAP:
        while ( i>0 &&
                heap->comp_fn( vec_get(heap->data, i) ,
                               vec_get(heap->data, (i-1)/2) ) < 0 ) {
            vec_swap(heap->data, i, (i-1)/2);
            i = (i-1)/2;
        }
        break;
    case MAX_HEAP:
        while ( i>0 &&
                heap->comp_fn( vec_get(heap->data, i) ,
                               vec_get(heap->data, (i-1)/2) ) > 0 )  {
            vec_swap(heap->data, i, (i-1)/2);
            i = (i-1)/2;
        }
        break;
    }
    return i;
}

        
static size_t bubble_down(binheap *heap, size_t pos)
{
    size_t i, l, r, m, n;
    n = binheap_size(heap);
    i = pos;
    switch (heap->mode) {
    default:
    case MIN_HEAP:
        while (true) {
            m = i;
            l = (2*i)+1;
            r = (2*i)+2;
            if ( l < n &&
                 heap->comp_fn( vec_get(heap->data, l),
                                vec_get(heap->data, m) ) < 0 )  {
                m = l;
            }
            if ( r < n &&
                 heap->comp_fn( vec_get(heap->data, r),
                                vec_get(heap->data, m) ) < 0 ) {
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
        break;
    case MAX_HEAP:
        while (true) {
            m = i;
            l = (2*i)+1;
            r = (2*i)+2;
            if ( l < n &&
                 heap->comp_fn( vec_get(heap->data, l),
                                vec_get(heap->data, m) ) > 0 ) {
                m = l;
            }
            if ( r < n &&
                 heap->comp_fn( vec_get(heap->data, r),
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
        break;
    }
}

        
void binheap_push(binheap *heap, void *elt)
{
    vec_app(heap->data, elt);
    bubble_up(heap);
}


void binheap_pop(binheap *heap, void *dest)
{
    if (vec_len(heap->data)==0) return;
    vec_swap(heap->data, 0, vec_len(heap->data)-1);
    vec_del(heap->data, vec_len(heap->data)-1, dest);
    if (vec_len(heap->data) > 0) 
        bubble_down(heap, 0);
}




#define BINHEAP_PUSH( TYPE )\
    void binheap_push_##TYPE(binheap *heap, TYPE val)\
        {   binheap_push(heap, &val);   }


#define BINHEAP_POP( TYPE )\
    TYPE binheap_pop_##TYPE(binheap *heap)\
        {   TYPE s; binheap_pop(heap, &s); return s; }


#define BINHEAP_ALL( TYPE )\
BINHEAP_PUSH(TYPE)\
BINHEAP_POP(TYPE)

BINHEAP_ALL(int)
BINHEAP_ALL(size_t)
