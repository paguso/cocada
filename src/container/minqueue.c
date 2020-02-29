#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "new.h"
#include "order.h"
#include "minqueue.h"
#include "deque.h"

struct _minqueue 
{
    deque *elts;
    deque *mins;
    size_t dels;
    cmp_func cmp;
};

struct _minqueue_iter 
{
    minqueue *src;
    size_t index;
};

minqueue *minqueue_new(size_t typesize,  cmp_func cmp)
{
    minqueue *ret = NEW(minqueue);
    ret->elts = deque_new(typesize);
    ret->mins = deque_new_size_t();
    ret->cmp = cmp;
    ret->dels = 0;
    return ret; 
}

minqueue *minqueue_new_with_capacity(size_t typesize,  cmp_func cmp, size_t capacity)
{
    minqueue *ret = NEW(minqueue);
    ret->elts = deque_new_with_capacity(typesize, capacity);
    ret->mins = deque_new_with_capacity(sizeof(size_t), capacity);
    ret->cmp = cmp;
    ret->dels = 0;
    return ret; 
}

void minqueue_free(minqueue *queue, bool free_elts)
{
    deque_free(queue->elts, free_elts);
    deque_free(queue->mins, false);
    FREE(queue);
}

size_t minqueue_len(minqueue *queue)
{
    return deque_len(queue->elts);
}

void minqueue_push(minqueue *queue, void *elt) 
{
    while ( deque_len(queue->mins)>0 && 
            queue->cmp( elt, deque_get(queue->elts, deque_back_size_t(queue->mins) - queue->dels) ) < 0 ) {
        deque_pop_back_size_t(queue->mins);
    }
    deque_push_back_size_t(queue->mins, queue->dels + deque_len(queue->elts));
    deque_push_back(queue->elts, elt);
}

void minqueue_pop(minqueue *queue, void *dest) 
{
    if ( deque_len(queue->elts) == 0 )
        return;
    if ( deque_front_size_t(queue->mins) == queue->dels ) {
        deque_pop_front_size_t(queue->mins);
    }
    queue->dels++;
    deque_pop_front(queue->elts, dest);
}


void *minqueue_min(minqueue *queue)
{
    return deque_get(queue->elts, deque_front_size_t(queue->mins) - queue->dels );
}


minqueue_iter *minqueue_get_iter(minqueue *queue)
{
    return NULL;
}


static void _minqueue_iter_goto_next(minqueue_iter *iter) 
{
    if (iter->index == 0) return;
    minqueue *src = iter->src;
    if ( iter->index < deque_len(src->mins) &&
         src->cmp( deque_get(src->elts, deque_front_size_t(src->mins) - src->dels),
                   deque_get(src->elts, deque_get_size_t(src->mins, iter->index) - src->dels) ) < 0 ) 
    {
        iter->index = deque_len(src->mins);
    }

}


minqueue_iter *minqueue_all_min(minqueue *queue)
{
    minqueue_iter *iter = NEW(minqueue_iter);
    iter->src = queue;
    iter->index = 0;
    _minqueue_iter_goto_next(iter);
    return iter;
}

bool minqueue_iter_has_next(minqueue_iter *iter)
{
    return (iter->index < deque_len(iter->src->mins));
}


void *minqueue_iter_next(minqueue_iter *iter)
{
    void *ret;
    ret = deque_get(iter->src->elts, deque_get_size_t(iter->src->mins, iter->index) - iter->src->dels );
    iter->index +=1;
    _minqueue_iter_goto_next(iter);
    return ret;
}

