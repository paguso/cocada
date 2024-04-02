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
#include <stddef.h>
#include <stdint.h>

#include "bitbyte.h"
#include "new.h"
#include "order.h"
#include "minqueue.h"
#include "deque.h"

struct _MinQueue {
	Deque *elts;
	Deque *mins;
	size_t dels;
	CmpFunc cmp;
};


struct _MinQueueIter {
	const MinQueue *src;
	size_t index;
};


MinQueue *minqueue_new(size_t typesize,  CmpFunc cmp)
{
	MinQueue *ret = NEW(MinQueue);
	ret->elts = deque_new(typesize);
	ret->mins = deque_new_size_t();
	ret->cmp = cmp;
	ret->dels = 0;
	return ret;
}


MinQueue *minqueue_new_with_capacity(size_t typesize,  CmpFunc cmp,
                                     size_t capacity)
{
	MinQueue *ret = NEW(MinQueue);
	ret->elts = deque_new_with_capacity(typesize, capacity);
	ret->mins = deque_new_with_capacity(sizeof(size_t), capacity);
	ret->cmp = cmp;
	ret->dels = 0;
	return ret;
}


void minqueue_finalise(void *ptr, const Finaliser *fnr )
{
	MinQueue *mq = (MinQueue *)ptr;
	deque_finalise(mq->elts, fnr);
	FREE(mq->elts);
	DESTROY_FLAT(mq->mins, deque);
}


size_t minqueue_len(const MinQueue *queue)
{
	return deque_len(queue->elts);
}


void minqueue_push(MinQueue *queue, const void *elt)
{
	while ( deque_len(queue->mins) > 0 &&
	        queue->cmp( elt, deque_get(queue->elts,
	                                   deque_back_size_t(queue->mins) - queue->dels) ) < 0 ) {
		deque_pop_back_size_t(queue->mins);
	}
	deque_push_back_size_t(queue->mins, queue->dels + deque_len(queue->elts));
	deque_push_back(queue->elts, elt);
}


void minqueue_pop(MinQueue *queue, void *dest)
{
	if ( deque_front_size_t(queue->mins) == queue->dels ) {
		deque_pop_front_size_t(queue->mins);
	}
	queue->dels++;
	deque_pop_front(queue->elts, dest);
}


void minqueue_del(MinQueue *queue)
{
	if ( deque_front_size_t(queue->mins) == queue->dels ) {
		deque_pop_front_size_t(queue->mins);
	}
	queue->dels++;
	deque_del_front(queue->elts);

}


const void *minqueue_front(const MinQueue *queue)
{
	return deque_front(queue->elts);
}


const void *minqueue_back(const MinQueue *queue)
{
	return deque_back(queue->elts);
}


const void *minqueue_min(const MinQueue *queue)
{
	return deque_get(queue->elts, deque_front_size_t(queue->mins) - queue->dels );
}


void minqueue_min_cpy(const MinQueue *queue, void *dest)
{
	deque_get_cpy(queue->elts, deque_front_size_t(queue->mins) - queue->dels,
	              dest );

}


#define MINQUEUE_PUSH_IMPL( TYPE )\
	void minqueue_push_##TYPE(MinQueue *queue, TYPE val) {\
		minqueue_push(queue, &val);\
	}


#define MINQUEUE_POP_IMPL( TYPE )\
	TYPE minqueue_pop_##TYPE(MinQueue *queue) {\
		TYPE ret;\
		minqueue_pop(queue, &ret);\
		return ret;\
	}

#define MINQUEUE_FRONT_IMPL( TYPE )\
	TYPE minqueue_front_##TYPE(const MinQueue *queue) {\
		return deque_front_##TYPE(queue->elts);\
	}

#define MINQUEUE_BACK_IMPL( TYPE )\
	TYPE minqueue_back_##TYPE(const MinQueue *queue) {\
		return deque_back_##TYPE(queue->elts);\
	}

#define MINQUEUE_MIN_IMPL( TYPE )\
	TYPE minqueue_min_##TYPE(const MinQueue *queue){\
		return ((TYPE *)minqueue_min(queue))[0];\
	}




#define MINQUEUE_ALL_IMPL( TYPE , ... )\
	MINQUEUE_PUSH_IMPL(TYPE)\
	MINQUEUE_POP_IMPL(TYPE)\
	MINQUEUE_FRONT_IMPL(TYPE)\
	MINQUEUE_BACK_IMPL(TYPE)\
	MINQUEUE_MIN_IMPL(TYPE)

XX_CORETYPES(MINQUEUE_ALL_IMPL)

/*
struct _minqueue_iter
{
    const minqueue *src;
    size_t index;
};
*/


static void _minqueue_iter_goto_next(MinQueueIter *iter)
{
	if (iter->index == 0) return;
	const MinQueue *src = iter->src;
	if ( iter->index < deque_len(src->mins) &&
	        src->cmp( deque_get(src->elts, deque_front_size_t(src->mins) - src->dels),
	                  deque_get(src->elts, deque_get_size_t(src->mins,
	                            iter->index) - src->dels) ) < 0 ) {
		iter->index = deque_len(src->mins);
	}

}


MinQueueIter *minqueue_all_min(const MinQueue *queue)
{
	MinQueueIter *iter = NEW(MinQueueIter);
	iter->src = queue;
	iter->index = 0;
	_minqueue_iter_goto_next(iter);
	return iter;
}


void minqueue_iter_free(MinQueueIter *iter)
{
	FREE(iter);
}


bool minqueue_iter_has_next(const MinQueueIter *iter)
{
	return (iter->index < deque_len(iter->src->mins));
}


const void *minqueue_iter_next(MinQueueIter *iter)
{
	const void *ret;
	ret = deque_get(iter->src->elts, deque_get_size_t(iter->src->mins,
	                iter->index) - iter->src->dels );
	iter->index += 1;
	_minqueue_iter_goto_next(iter);
	return ret;
}

