#ifndef SLORDMAP_H
#define SLORDMAP_H

#include "coretype.h"
#include "iter.h"
#include "new.h"
#include "order.h"
#include "skiplist.h"

typedef struct _slordmap slordmap;


slordmap *slordmap_new(size_t sizeof_key, size_t sizeof_val, cmp_func key_cmp);

void slordmap_finalise(void *ptr, const finaliser *fnr);

bool slordmap_contains(slordmap *self, const void *key);

const void *slordmap_get(slordmap *self, const void *key);

bool slordmap_set(slordmap *self, const void *key, const void *val);

skiplist_iter *slordmap_get_iter(slordmap *self);

const void *slordmap_key(const slordmap *self, const void *key_val_entry);

const void *slordmap_val(const slordmap *self, void *key_val_entry);



#endif
