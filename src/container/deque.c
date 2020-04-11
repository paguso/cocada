
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "arrutil.h"
#include "new.h"
#include "mathutil.h"
#include "deque.h"

const static size_t MIN_CAPACITY = 4; // (!) MIN_CAPACITY > 1
const static float  GROW_BY = 1.62f;  // (!) 1 < GROW_BY <= 2
const static float  MIN_LOAD = 0.5;   // (!) GROW_BY*MIN_LOAD < 1

struct _deque {
	size_t typesize;
	size_t start;
	size_t len;
	size_t cap;
	void  *data;
};


deque *deque_new(size_t typesize)
{
	return deque_new_with_capacity(typesize, MIN_CAPACITY);
}


deque *deque_new_with_capacity(size_t typesize, size_t capacity)
{
	deque *q = NEW(deque);
	q->typesize = typesize;
	q->start = 0;
	q->len = 0;
	q->cap = MAX(MIN_CAPACITY, capacity);
	q->data = malloc(q->cap*q->typesize);
	return q;
}


void deque_dtor(void *ptr, const dtor *dt)
{
	deque *dq = (deque *)ptr;
	if (dtor_nchd(dt)) {
		const dtor *chd_dt = dtor_chd(dt, 0);
		for (size_t i=0, l=deque_len(dq); i<l; i++) {
			void *chd = ((void **)deque_get(dq, i))[0];
			FINALISE(chd, chd_dt);
		}
	}
	FREE(dq->data);
}


bool deque_empty(const deque *q)
{
	return (q->len == 0);
}


size_t deque_len(const deque *q)
{
	return q->len;
}


const void *deque_get(const deque *q, size_t pos)
{
	return q->data + ( ((q->start + pos) % q->cap) * q->typesize );
}


void deque_get_cpy(const deque *q, size_t pos, void *dest )
{
	memcpy(dest, q->data + ( ((q->start + pos) % q->cap) * q->typesize ), q->typesize);
}


const void *deque_front(const deque *q)
{
	return deque_get(q, 0);
}


const void *deque_back(const deque *q)
{
	return deque_get(q, q->len-1);
}


static void check_and_resize(deque *q)
{
	if (q->len == q->cap) {
		size_t offset = q->cap;
		q->cap = MAX(GROW_BY*q->cap, MIN_CAPACITY);
		q->data = realloc(q->data, q->cap*q->typesize);
		if (q->start==0) return;
		offset = q->cap - offset;
		memmove( q->data+(q->start+offset)*q->typesize,
		         q->data+(q->start*q->typesize),
		         (q->len-q->start)*q->typesize );
		q->start += offset;
	} else if (q->len < MIN_LOAD*q->cap) {
		if (q->start+q->len < q->cap) {
			memmove(q->data, q->data+(q->start*q->typesize), q->len*q->typesize);
		} else {
			memmove( q->data + (q->cap - q->start)*q->typesize,
			         q->data, ((q->start+q->len)%q->cap)*q->typesize );
			memmove( q->data,  q->data + (q->start * q->typesize),
			         (q->cap - q->start)*q->typesize );
		}
		q->cap = MAX(q->len/MIN_LOAD, MIN_CAPACITY);
		q->data = realloc(q->data, q->cap*q->typesize);
		q->start = 0;
	}
}


void deque_push_back(deque *q, const void *elt)
{
	check_and_resize(q);
	memcpy(q->data+(((q->start+q->len)%q->cap)*q->typesize), elt, q->typesize);
	q->len++;
}


void deque_push_front(deque *q, const void *elt)
{
	check_and_resize(q);
	q->start = (q->start + (q->cap-1)) % q->cap;
	memcpy(q->data+(q->start * q->typesize), elt, q->typesize);
	q->len++;
}


void deque_pop_back(deque *q, void *dest)
{
	memcpy(dest, q->data+(((q->start+q->len-1)%q->cap)*q->typesize), q->typesize);
	q->len--;
	check_and_resize(q);
}


void deque_remv_back(deque *q)
{
	q->len--;
	check_and_resize(q);
}


void deque_pop_front(deque *q, void *dest)
{
	memcpy(dest, q->data+(q->start*q->typesize), q->typesize);
	q->len--;
	q->start = (q->start+1)%q->cap;
	check_and_resize(q);
}


void deque_remv_front(deque *q)
{
	q->len--;
	q->start = (q->start+1)%q->cap;
	check_and_resize(q);
}

#define DEQUE_NEW_IMPL( TYPE )\
    deque *deque_new_##TYPE() {\
        return deque_new(sizeof(TYPE));\
    }

#define DEQUE_GET_IMPL( TYPE )\
    TYPE deque_get_##TYPE(const deque *q, size_t pos) {\
        return ((TYPE *)deque_get(q, pos))[0];\
    }

#define DEQUE_FRONT_IMPL( TYPE )\
    TYPE deque_front_##TYPE(const deque *q) {\
        return deque_get_##TYPE(q, 0);\
    }

#define DEQUE_BACK_IMPL( TYPE )\
    TYPE deque_back_##TYPE(const deque *q) {\
        return deque_get_##TYPE(q, q->len-1);\
    }

#define DEQUE_PUSH_BACK_IMPL( TYPE )\
    void deque_push_back_##TYPE(deque *q, TYPE val) {\
        deque_push_back(q, &val);\
    }

#define DEQUE_PUSH_FRONT_IMPL( TYPE )\
    void deque_push_front_##TYPE(deque *q, TYPE val) {\
        deque_push_front(q, &val);\
    }

#define DEQUE_POP_BACK_IMPL( TYPE ) \
    TYPE deque_pop_back_##TYPE(deque *q) { \
        TYPE ret;\
        deque_pop_back(q, &ret);\
        return ret;\
    }

#define DEQUE_POP_FRONT_IMPL( TYPE ) \
    TYPE deque_pop_front_##TYPE(deque *q) {\
        TYPE ret;\
        deque_pop_front(q, &ret);\
        return ret;\
    }

#define DEQUE_ALL_IMPL( TYPE ) \
DEQUE_NEW_IMPL(TYPE)\
DEQUE_GET_IMPL(TYPE)\
DEQUE_FRONT_IMPL(TYPE)\
DEQUE_BACK_IMPL(TYPE)\
DEQUE_PUSH_BACK_IMPL(TYPE) \
DEQUE_PUSH_FRONT_IMPL(TYPE)\
DEQUE_POP_BACK_IMPL(TYPE)\
DEQUE_POP_FRONT_IMPL(TYPE)

DEQUE_ALL_IMPL(int)
DEQUE_ALL_IMPL(size_t)