
#include <stdlib.h>
#include <string.h>

#include "avl.h"
#include "new.h"
#include "order.h"
#include "avlordmap.h"



struct _avlordmap {
    size_t sizeof_key;
    size_t sizeof_val;
    size_t sizeof_entry;
    avl *tree;
    cmp_func key_cmp;
    void *sandbox;
};


avlordmap *avlordmap_new(size_t sizeof_key, size_t sizeof_val, cmp_func key_cmp)
{
    avlordmap *ret = NEW(avlordmap);
    ret->sizeof_key = sizeof_key;
    ret->sizeof_val = sizeof_val;
    ret->sizeof_entry = sizeof_key + sizeof_val;
    ret->key_cmp = key_cmp;
    ret->tree = avl_new(ret->sizeof_entry, key_cmp);
    ret->sandbox = malloc(ret->sizeof_entry);
    return ret;
}

void avlordmap_finalise(void *ptr, const finaliser *fnr)
{
    finaliser *key_fnr = NULL, *val_fnr = NULL;
    if (finaliser_nchd(fnr) > 0) {
        key_fnr = finaliser_chd(fnr, 0);
    }
    if (finaliser_nchd(fnr) > 1) {
        val_fnr = finaliser_chd(fnr, 1);
    }
    avlordmap *map = (avlordmap *)ptr;
    avl_iter *avliter = avl_get_iter(map->tree, IN_ORDER);
    FOREACH_IN_ITER(entry, void, avl_iter_as_iter(avliter)) {
        if (key_fnr) FINALISE(entry, key_fnr);
        if (val_fnr) FINALISE(entry + map->sizeof_key, val_fnr);
    }
    avl_iter_free(avliter);
    DESTROY_FLAT(map->tree, avl);
    FREE(map->sandbox);
}


bool avlordmap_contains(avlordmap *self, const void *key)
{
    return avl_get(self->tree, key) != NULL;
}


const void *avlordmap_get(avlordmap *self, const void *key)
{
    const void *ret = avl_get(self->tree, key);
    return ret ? ret + self->sizeof_key : NULL;  
}


void avlordmap_set(avlordmap *self, const void *key, const void *val)
{
    memcpy(self->sandbox, key, self->sizeof_key);
    memcpy(self->sandbox + self->sizeof_key, val, self->sizeof_val);
    if (avl_get(self->tree, key)) {
        return avl_upd(self->tree, self->sandbox);
    }
    else {
        return avl_ins(self->tree, self->sandbox);
    }
}


avl_iter *avlordmap_get_iter(avlordmap *self)
{
    return avl_get_iter(self->tree, IN_ORDER);
}

const void *avlordmap_key(const avlordmap *self, const void *key_val_entry) 
{
    return key_val_entry;
}

const void *avlordmap_val(const avlordmap *self, void *key_val_entry) {
    return key_val_entry + self->sizeof_key;
}
