#ifndef AVLORDMAP_H
#define AVLORDMAP_H

#include "avl.h"
#include "coretype.h"
#include "iter.h"
#include "new.h"
#include "order.h"

typedef struct _avlordmap avlordmap;


avlordmap *avlordmap_new(size_t sizeof_key, size_t sizeof_val, cmp_func key_cmp);

void avlordmap_finalise(void *ptr, const finaliser *fnr);

bool avlordmap_contains(avlordmap *self, const void *key);

const void *avlordmap_get(avlordmap *self, const void *key);

void avlordmap_set(avlordmap *self, const void *key, const void *val);

avl_iter *avlordmap_get_iter(avlordmap *self);

const void *avlordmap_key(const avlordmap *self, const void *key_val_entry);

const void *avlordmap_val(const avlordmap *self, void *key_val_entry);

#endif