#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "cocadautil.h"
#include "order.h"
#include "minqueue.h"
#include "deque.h"

struct _minqueue 
{
    deque *elts;
    deque *prec;
    size_t dels;
    cmp_func cmp;
};

struct _minqueue_iter 
{
    minqueue *src;
};

minqueue *minqueue_new(size_t typesize,  cmp_func cmp)
{
    minqueue *ret = NEW(minqueue);
    ret->elts = deque_new(typesize);
    ret->prec = deque_new(sizeof(size_t));
    ret->cmp = cmp;
    ret->dels = 0;
    return ret; 
}

minqueue *minqueue_new_with_cap(size_t typesize,  cmp_func cmp, size_t capacity)
{
    minqueue *ret = NEW(minqueue);
    ret->elts = deque_new(typesize);
    ret->prec = deque_new_with_capacity(sizeof(size_t), capacity);
    ret->cmp = cmp;
    ret->dels = 0;
    return ret; 
}

void minqueue_free(minqueue *queue, bool free_elts)
{

}

size_t minqueue_size(minqueue *queue)
{
    return deque_len(queue->elts);
}

void minqueue_push(minqueue *queue, void *elt) 
{
    deque_push_back(queue->elts, elt);
}

void minqueue_pop(minqueue *queue, void *dest) 
{
    deque_pop_front(queue->elts, dest);
}

void *minqueue_min(minqueue *queue)
{
    return NULL;
}


minqueue_iter *minqueue_get_iter(minqueue *queue)
{
    return NULL;
}

minqueue_iter *minqueue_all_min(minqueue *queue)
{
    return NULL;
}

bool minqueue_iter_has_next(minqueue_iter *iter)
{
    return false;
}

void *minqueue_iter_next(minqueue_iter *iter)
{
    return NULL;
}

