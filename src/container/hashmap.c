/*
 * Copyright (C) 2015-  Paulo G.S. da Fonseca
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "arrayutil.h"
#include "cocadautil.h"
#include "hashmap.h"
#include "mathutil.h"
#include "string.h"


static size_t MIN_CAPACITY = 128;
static float GROW_BY = 2;
static float MIN_LOAD = 0.25;
static float MAX_LOAD = 0.5;


hashmap_entry *hashmap_entry_new(void *key, void *val)
{
    hashmap_entry *ret;
    ret = NEW(hashmap_entry);
    ret->key = key;
    ret->val = val;
    return ret;
}

struct _hashmap_iter_t {
    hashmap *src;
    size_t index;
};


typedef enum {
    ST_EMPTY = 0x80, // 0b10000000
    ST_DEL   = 0xFE, // 0b11111110
    ST_SENT  = 0xFF
} SLOT_ST;


struct _hashmap {
    size_t cap;
    size_t size;
    size_t occ;
    size_t keysize;
    size_t valsize;
    equals_func keyeq;
    hash_func keyhash;
    void   *data;
    void   *tally;
    void   *entries;
};


hashmap *hashmap_new(size_t keysize, size_t valsize, hash_func keyhash, equals_func keyeq)
{
   return hashmap_new_with_capacity(keysize, valsize, keyhash, keyeq, MIN_CAPACITY); 
}


static void _reset_data(hashmap *hmap, size_t cap)  
{
    hmap->cap = cap;
    hmap->size = 0;
    hmap->occ = 0;
    hmap->data = malloc(hmap->cap * (1 + hmap->keysize + hmap->valsize ));
    hmap->tally = hmap->data;
    memset(hmap->tally, hmap->cap,  ST_EMPTY);
    hmap->entries = hmap->data + hmap->cap;
}

hashmap *hashmap_new_with_capacity(size_t keysize, size_t valsize, hash_func keyhash, equals_func keyeq,
                                   size_t min_capacity)
{
    hashmap *ret;
    ret = NEW(hashmap);
    ret->keysize = keysize;
    ret->valsize = valsize;
    ret->keyhash = keyhash;
    ret->keyeq   = keyeq;
    _reset_data(ret, pow2ceil_size_t(MAX(MIN_CAPACITY, min_capacity)));
    return ret;
}

void hashmap_free(hashmap *hmap, bool free_keys, bool free_vals)
{
    if (hmap==NULL) return;
    size_t i;
    if (free_keys) {
    }
    if (free_vals) {
    }
    FREE(hmap->data);
    FREE(hmap);
}


static inline uint64_t _hash(hashmap *hmap, void *key)
{
    return fib_hash(hmap->keyhash(key));
}

static inline byte_t _h2(uint64_t h) 
{
    return h & 0x7F;
}

static inline uint64_t _h1(uint64_t h) 
{
    return h >> 7;
}

static inline void * _key_at(hashmap *hmap, size_t pos) 
{
    return hmap->entries + ( pos * (hmap->keysize + hmap->valsize) );
}


static inline void * _value_at(hashmap *hmap, size_t pos) 
{
    return hmap->entries + ( ( pos * (hmap->keysize + hmap->valsize) ) + hmap->keysize);
}

typedef struct {
    size_t pos;
    bool found;
} _find_res;

static _find_res _find(hashmap *hmap, void *key, uint64_t h) 
{
    uint64_t h1 = _h1(h);
    uint64_t h2 = _h2(h);
    _find_res ret = {.found=false, .pos=h1 % hmap->cap};
    while (true) {
        if ( ((byte_t *)hmap->tally)[ret.pos] == h2 &&
            hmap->keyeq( key, _key_at(hmap, ret.pos) ) ) {
            ret.found = true;
            return ret;
        }
        if (((byte_t *)hmap->tally)[ret.pos] == ST_EMPTY) {
            return ret; 
        }
        ret.pos = (ret.pos + 1) % hmap->cap;
    }    
}

static void check_and_resize(hashmap *hmap)
{
    size_t new_cap;
    if ( hmap->occ >= MAX_LOAD * hmap->cap ) 
        new_cap = (size_t)(GROW_BY * hmap->cap);
    //else if ( hmap->size < MIN_LOAD*hmap->cap ) {
    //    new_cap = (size_t)(hmap->cap / GROW_BY) ;
    //    new_cap = MAX(new_cap, MIN_CAPACITY);
    //}
    if (new_cap == hmap->cap)
        return; 
    
    size_t old_cap = hmap->cap;
    size_t old_size = hmap->size;
    void *old_data = hmap->data;
    void *old_tally = old_data;
    void *old_entries = old_data + old_cap; 

    for (size_t i=0; i<old_cap; ++i) {
    }

    FREE(old_data);
}


bool hashmap_haskey(hashmap *hmap, void *key)
{
    return _find(hmap, key, _hash(hmap, key)).found;
}


void *hashmap_get(hashmap *hmap, void *key)
{
    _find_res qry = _find(hmap, key, _hash(hmap, key));
    if (qry.found) {
        return _value_at(hmap, qry.pos);
    } 
    else {
        return NULL;
    }
}

void hashmap_set(hashmap *hmap, void *key, void *val)
{
    assert(key != NULL);
    check_and_resize(hmap);
    uint64_t h = _hash(hmap, key);
    _find_res qry = _find(hmap, key, h);
    if (!qry.found) {
        memcpy(_key_at(hmap, qry.pos), key, hmap->keysize);
        ((byte_t *)hmap->tally)[qry.pos] = _h2(h);
        hmap->size++;
        hmap->occ++;
    } 
    memcpy(_value_at(hmap, qry.pos), val, hmap->valsize);
}


void hashmap_unset(hashmap *hmap, void *key)
{
    assert(key != NULL);
    uint64_t h = _hash(hmap, key);
    _find_res qry = _find(hmap, key, h);
    if (qry.found) {
        ((byte_t *)hmap->tally)[qry.pos] = ST_DEL;
        hmap->size--;
    } 
}

size_t hashmap_size(hashmap *map)
{
    return map->size;
}


hashmap_iter *hashmap_iter_new(hashmap *src)
{
    hashmap_iter *ret;
    ret = NEW(hashmap_iter);
    ret->src = src;
    ret->index = 0;
    while ((ret->index < ret->src->capacity) &&
            (ret->src->entries[ret->index] == NULL ||
             ret->src->entries[ret->index]->key == NULL)) {
        ret->index++;
    }
    return ret;
}


void hashmap_iterator_free(hashmap_iter *it)
{
    FREE(it);
}

bool hashmap_iterator_has_next(hashmap_iter *it)
{
    return it->index < it->src->capacity;
}

hashmap_entry *hashmap_iterator_next(hashmap_iter *it)
{
    hashmap_entry *ret;
    if (it->index >= it->src->capacity) {
        return NULL;
    }
    ret = hashmap_entry_new(it->src->entries[it->index]->key,
                            it->src->entries[it->index]->val);
    do {
        it->index++;
    }
    while ((it->index < it->src->capacity) &&
            (it->src->entries[it->index] == NULL ||
             it->src->entries[it->index]->key == NULL));
    return ret;
}

