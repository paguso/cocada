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
#include "bitsandbytes.h"
#include "cocadautil.h"
#include "cstringutil.h"
#include "hashmap.h"
#include "mathutil.h"
#include "string.h"


static size_t MIN_CAPACITY = 128;
static float GROW_BY = 2;
static float MIN_LOAD = 0.25;
static float MAX_LOAD = 0.5;


static byte_t  ST_EMPTY = 0x80; // 0b10000000
static byte_t  ST_DEL   = 0xFE; // 0b11111110


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
    memset(hmap->tally, ST_EMPTY, hmap->cap);
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
    if (free_keys || free_vals) {
        hashmap_iter *iter = hashmap_get_iter(hmap);
        while ( hashmap_iter_has_next(iter) ) {
            hashmap_entry keyval = hashmap_iter_next(iter);
            if (free_keys) FREE(((void **)keyval.key)[0]);
            if (free_vals) FREE(((void **)keyval.val)[0]);
        }
        hashmap_iter_free(iter);
    }
    FREE(hmap->data);
    FREE(hmap);
}


typedef struct {
    size_t pos;
    bool found;
} _find_res;

// Find the target position of the key in the table
// 
static _find_res _find(hashmap *hmap, void *key, uint64_t h) 
{
    uint64_t h1 = _h1(h);
    uint64_t h2 = _h2(h);
    _find_res ret = {.found=false, .pos=h1 % hmap->cap};
    //printf("starting probe at pos %zu\n", ret.pos );
    while (true) {
        //printf("   probing pos %zu\n", ret.pos );
        if ( ((byte_t *)hmap->tally)[ret.pos] == h2 &&
            hmap->keyeq( key, _key_at(hmap, ret.pos) ) ) {
            ret.found = true;
            break;
        }
        if (((byte_t *)hmap->tally)[ret.pos] == ST_EMPTY) {
            break;
        }
        ret.pos = (ret.pos + 1) % hmap->cap;
    }    
    //printf("returning pos=%zu found=%d\n", ret.pos, (int)ret.found );
    return ret;
}


bool hashmap_has_key(hashmap *hmap, void *key)
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

static void _print(hashmap *hmap) {
    printf("Hashmap at %p\n", hmap);
    char *c = cstr_new(8);
    for (size_t i=0; i<hmap->cap; i++) {
        byte_to_str( ((byte_t *)hmap->tally)[i], c );
        printf("   %zu) %s\n", i, c);
    }
    FREE(c);
}

static inline void _set(hashmap *hmap, void *key, void *val)
{
    uint64_t h = _hash(hmap, key);
    _find_res qry = _find(hmap, key, h);
    if (!qry.found) {
        memcpy(_key_at(hmap, qry.pos), key, hmap->keysize);
        ((byte_t *)hmap->tally)[qry.pos] = _h2(h);
        hmap->size++;
        hmap->occ++;
    } 
    memcpy(_value_at(hmap, qry.pos), val, hmap->valsize);
    //char *c = cstr_new(8);
    //byte_to_str(_h2(h), c);
    //printf("adding h2 key %s to position %zu\n",c, qry.pos);
    //FREE(c);
    //_print(hmap);
}


static void check_and_resize(hashmap *hmap)
{
    size_t new_cap = hmap->cap;
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

    _reset_data(hmap, new_cap);
    //_print(hmap);

    size_t rehash_attempts = 0;
    for (size_t i=0; i<old_cap; ++i) {
        if (! (((byte_t *)old_tally)[i] >> 7) ) { 
            rehash_attempts += 1;
            //printf("rehashing element at pos %zu\n",i);
            _set( hmap, 
                  old_entries + ( i * ( hmap->keysize + hmap->valsize ) ), 
                  old_entries + ( i * ( hmap->keysize + hmap->valsize ) ) + hmap->keysize ); 
            //_print(hmap);
            if (rehash_attempts!=hmap->size) {
                printf("failed to rehash pos %zu\n",i );
            }
        }
    }
    assert(old_size == hmap->size);
    FREE(old_data);
}


void hashmap_set(hashmap *hmap, void *key, void *val)
{
    assert(key != NULL);
    check_and_resize(hmap);
    _set(hmap, key, val);
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


struct _hashmap_iter {
    hashmap *src;
    size_t index;
};


void _hashmap_iter_goto_next(hashmap_iter *iter) {
    while ((iter->index < iter->src->cap) &&
           (((byte_t *)iter->src->tally)[iter->index] >> 7))
        iter->index++;
}


hashmap_iter *hashmap_get_iter(hashmap *src)
{
    hashmap_iter *ret;
    ret = NEW(hashmap_iter);
    ret->src = src;
    ret->index = 0;
    _hashmap_iter_goto_next(ret);
    return ret;
}


void hashmap_iter_free(hashmap_iter *iter)
{
    FREE(iter);
}

bool hashmap_iter_has_next(hashmap_iter *iter)
{
    return (iter->index < iter->src->cap);
}

hashmap_entry hashmap_iter_next(hashmap_iter *iter)
{
    hashmap_entry ret = {NULL, NULL};
    if (iter->index >= iter->src->cap ) {
        return ret;
    }
    ret.key = _key_at(iter->src, iter->index);
    ret.val = _value_at(iter->src, iter->index);
    iter->index++;
    _hashmap_iter_goto_next(iter);
    return ret;
}

