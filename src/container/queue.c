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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "arrayutil.h"
#include "cocadautil.h"
#include "mathutil.h"
#include "queue.h"


const static size_t MIN_CAPACITY = 4; // (!) MIN_CAPACITY > 1
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2 
const static float  MIN_LOAD = 0.5;   // (!) GROW_BY*MIN_LOAD < 1

struct _queue {
    size_t typesize;
    size_t start;
    size_t len;
    size_t capacity;
    void  *data;
};


queue *queue_new(size_t typesize)
{
    return queue_new_with_capacity(typesize, MIN_CAPACITY);
}


queue *queue_new_with_capacity(size_t typesize, size_t capacity)
{
    queue *q = NEW(queue);
    q->typesize = typesize;
    q->start = 0;
    q->len = 0;
    q->capacity = MAX(MIN_CAPACITY, capacity);
    q->data = malloc(q->capacity*q->typesize);
    return q;
}


void queue_free(queue *q, bool free_elements)
{
    if (q==NULL) return;
    if (free_elements)
        for (size_t i=0; i<q->len; i++)
            FREE(((void **)q->data)[(q->start+i)%q->capacity]);
    FREE(q->data);
    FREE(q);
}


bool queue_empty(queue *q)
{
    return (q->len == 0);
}


size_t queue_len(queue *q)
{
    return q->len;
}


static void check_and_resize(queue *q)
{
    if (q->len == q->capacity) { 
        size_t offset = q->capacity;
        q->capacity = MAX(GROW_BY*q->capacity, MIN_CAPACITY);
        q->data = realloc(q->data, q->capacity*q->typesize);
        if (q->start==0) return;
        offset = q->capacity - offset;
        memmove( q->data+(q->start+offset)*q->typesize, 
                 q->data+(q->start*q->typesize), 
                 (q->len-q->start)*q->typesize );
        q->start += offset;
    }
    else if (q->len < MIN_LOAD*q->capacity) {
        if (q->start+q->len < q->capacity) {
            memmove(q->data, q->data+(q->start*q->typesize), q->len*q->typesize);
        }
        else {
            memmove( q->data + (q->capacity - q->start)*q->typesize, 
                     q->data, ((q->start+q->len)%q->capacity)*q->typesize );
            memmove( q->data,  q->data + (q->start * q->typesize), 
                     (q->capacity - q->start)*q->typesize );
        }
        q->capacity = MAX(q->len/MIN_LOAD, MIN_CAPACITY);
        q->data = realloc(q->data, q->capacity*q->typesize);
        q->start = 0;
    }
}


void queue_push(queue *q, void *elt)
{
    check_and_resize(q);
    memcpy(q->data+(((q->start+q->len)%q->capacity)*q->typesize), elt, q->typesize);
    q->len++;
}


void queue_pop(queue *q, void *dest)
{
    if (queue_empty(q)) return;
    if (dest!=NULL)
        memcpy(dest, q->data+(q->start*q->typesize), q->typesize);
    q->len--;
    q->start = (q->start+1)%q->capacity;
    check_and_resize(q);
}


#define QUEUE_PUSH( TYPE ) \
    void queue_push_##TYPE(queue *q, TYPE val)\
    { queue_push(q, &val); }


#define QUEUE_POP( TYPE ) \
    TYPE queue_pop_##TYPE(queue *q)\
    { TYPE val; queue_pop(q, &val); return val;}
    

#define QUEUE_ALL_OPS( TYPE ) \
QUEUE_PUSH(TYPE) \
QUEUE_POP(TYPE)


QUEUE_ALL_OPS(int)
QUEUE_ALL_OPS(size_t)

