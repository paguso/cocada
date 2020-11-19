#ifndef GK_H
#define GK_H

#include <stdio.h>

#include "coretype.h"
#include "order.h"


typedef struct __gksketch gksketch;

gksketch *gk_new(size_t typesize, cmp_func cmp, double err);

void gk_upd(gksketch *self, const void *val);

void gk_merge(gksketch *self, const gksketch *other);

size_t gk_qry(gksketch *self, const void *val);

void gk_print(gksketch *self, FILE *stream, void (*print_val)(FILE *, const void *));

#endif