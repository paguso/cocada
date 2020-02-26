#ifndef MINQUEUE_H
#define MINQUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "order.h"

typedef struct _minqueue minqueue;

typedef struct _minqueue_iter minqueue_iter;

minqueue *minqueue_new(size_t typesize,  cmp_func cmp);

minqueue *minqueue_new_with_capacity(size_t typesize,  cmp_func, size_t capacity);

void minqueue_free(minqueue *queue, bool free_elts);

size_t minqueue_len(minqueue *queue);

void minqueue_push(minqueue *queue, void *elt);

void minqueue_pop(minqueue *queue, void *dest);

void *minqueue_min(minqueue *queue);

minqueue_iter *minqueue_get_iter(minqueue *queue);

minqueue_iter *minqueue_all_min(minqueue *queue);

bool minqueue_iter_has_next(minqueue_iter *iter);

void *minqueue_iter_next(minqueue_iter *iter);


#endif