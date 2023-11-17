#ifndef vebset_H
#define vebset_H

#include <coretype.h>

typedef struct _vebset vebset;


vebset *vebset_new();

void vebset_free(vebset *self);

bool vebset_contains(vebset *self, uint32_t x);

bool vebset_add(vebset *self, uint32_t x);

int64_t vebset_succ(vebset *self, uint32_t x);

int64_t vebset_pred(vebset *self, uint32_t x);

int64_t vebset_min(vebset *self);

int64_t vebset_max(vebset *self);

bool vebset_del(vebset *self, uint32_t x);


#endif