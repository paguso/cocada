#ifndef ROARING_H
#define ROARING_H

#include <stdlib.h>
#include <stdio.h>

#include "coretype.h"


typedef struct _roaringbitvec roaringbitvec;

roaringbitvec *roaringbitvec_new(size_t n);

roaringbitvec *roaringbitvec_new_from_bitarr(byte_t *b, size_t n);

void roaringbitvec_free(roaringbitvec *self);

size_t roaringbitvec_card(roaringbitvec *self);

size_t roaringbitvec_memsize(roaringbitvec *self);

void roaringbitvec_fit(roaringbitvec *self);

void roaringbitvec_set(roaringbitvec *self, uint32_t pos, bool val);
 
bool roaringbitvec_get(roaringbitvec *self, uint32_t pos);

void roaringbitvec_fprint(FILE *stream, roaringbitvec *self);

#endif