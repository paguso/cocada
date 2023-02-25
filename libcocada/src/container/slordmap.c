#include <stdlib.h>
#include <string.h>

#include "new.h"
#include "order.h"
#include "slordmap.h"
#include "skiplist.h"



struct _slordmap {
    size_t sizeof_key;
    size_t sizeof_val;
    size_t sizeof_entry;
    skiplist *list;
    cmp_func key_cmp;
    void *sandbox;
};


slordmap *slordmap_new(size_t sizeof_key, size_t sizeof_val, cmp_func key_cmp)
{
    slordmap *ret = NEW(slordmap);
    ret->sizeof_key = sizeof_key;
    ret->sizeof_val = sizeof_val;
    ret->sizeof_entry = sizeof_key + sizeof_val;
    ret->key_cmp = key_cmp;
    ret->list = skiplist_new(ret->sizeof_entry, key_cmp);
    ret->sandbox = malloc(ret->sizeof_entry);
    return ret;
}

void slordmap_finalise(void *ptr, const finaliser *fnr)
{
    finaliser *key_fnr = NULL, *val_fnr = NULL;
    if (finaliser_nchd(fnr) > 0) {
        key_fnr = finaliser_chd(fnr, 0);
    }
    if (finaliser_nchd(fnr) > 1) {
        val_fnr = finaliser_chd(fnr, 1);
    }
    slordmap *map = (slordmap *)ptr;
    skiplist_iter *sliter = skiplist_get_iter(map->list);
    FOREACH_IN_ITER(entry, void, skiplist_iter_as_iter(sliter)) {
        if (key_fnr) FINALISE(entry, key_fnr);
        if (val_fnr) FINALISE(entry + map->sizeof_key, val_fnr);
    }
    FREE(sliter);
    DESTROY_FLAT(map->list, skiplist);
    FREE(map->sandbox);
}


bool slordmap_contains(slordmap *self, const void *key)
{
    return skiplist_search(self->list, key) != NULL;
}


const void *slordmap_get(slordmap *self, const void *key)
{
    const void *ret = skiplist_search(self->list, key);
    return ret ? ret + self->sizeof_key : NULL;  
}


bool slordmap_set(slordmap *self, const void *key, const void *val)
{
    memcpy(self->sandbox, key, self->sizeof_key);
    memcpy(self->sandbox + self->sizeof_key, val, self->sizeof_val);
    if (skiplist_search(self->list, key)) {
        return skiplist_upd(self->list, self->sandbox);
    }
    else {
        return skiplist_ins(self->list, self->sandbox);
    }
}


skiplist_iter *slordmap_get_iter(slordmap *self)
{
    return skiplist_get_iter(self->list);
}

const void *slordmap_key(const slordmap *self, const void *key_val_entry) 
{
    return key_val_entry;
}

const void *slordmap_val(const slordmap *self, void *key_val_entry) {
    return key_val_entry + self->sizeof_key;
}
