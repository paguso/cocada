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

//#include <immintrin.h>

#include "coretype.h"
#include "arrays.h"
#include "bitbyte.h"
#include "new.h"
#include "cstrutil.h"
#include "hashmap.h"
#include "mathutil.h"
#include "order.h"
#include "string.h"
#include "iter.h"


static size_t MIN_CAPACITY = 128; // HAS TO BE A MULTIPLE OF GROUPSIZE
static float GROW_BY = 2.0F; // DON´T TOUCH
static float MAX_LOAD = 0.75; 

static byte_t  ST_EMPTY = 0x80; // empty slot ctrl code   0b10000000
static byte_t  ST_DEL   = 0xFE; // deleted slot ctrl code 0b11111110

struct _hashmap {
	size_t cap;
	size_t size;
	size_t occ;
	size_t max_occ;
	size_t keysize;
	size_t valsize;
	eq_func keyeq;
	hash_func keyhash;
	void   *data;
	byte_t *tally;
	void   *entries;
};



void hashmap_init(hashmap *map, size_t keysize, size_t valsize, hash_func keyhash, eq_func keyeq)
{
	hashmap_init_with_capacity(map, keysize, valsize, keyhash, keyeq, MIN_CAPACITY);
}


hashmap *hashmap_new(size_t keysize, size_t valsize, hash_func keyhash, eq_func keyeq)
{
	return hashmap_new_with_capacity(keysize, valsize, keyhash, keyeq, MIN_CAPACITY);
}


static void _reset_data(hashmap *hmap, size_t cap)
{
	hmap->cap = cap;
	hmap->size = 0;
	hmap->occ = 0;
	hmap->max_occ = MAX_LOAD * cap;
	hmap->data = malloc(hmap->cap * (1 + hmap->keysize + hmap->valsize ));
	hmap->tally = (byte_t *) hmap->data;
	memset(hmap->tally, ST_EMPTY, hmap->cap);
	hmap->entries = hmap->data + hmap->cap;
}


void hashmap_init_with_capacity(hashmap *ret, size_t keysize, size_t valsize, hash_func keyhash, eq_func keyeq,
                                size_t min_capacity)
{
	ret->keysize = keysize;
	ret->valsize = valsize;
	ret->keyhash = keyhash;
	ret->keyeq   = keyeq;
	_reset_data(ret, pow2ceil_size_t(MAX(MIN_CAPACITY, min_capacity)));
}


hashmap *hashmap_new_with_capacity(size_t keysize, size_t valsize, hash_func keyhash, eq_func keyeq,
                                   size_t min_capacity)
{
	hashmap *ret = NEW(hashmap);
	hashmap_init_with_capacity(ret, keysize, valsize, keyhash, keyeq, min_capacity);
	return ret;
}


void hashmap_dtor(void *ptr, const dtor *dst)
{
	hashmap *hmap = (hashmap *)ptr;
	if (dst != NULL) {
		bool free_keys = (dtor_nchd(dst) > 0);
		const dtor *keys_dst = (free_keys)?dtor_chd(dst, 0):NULL;
		bool free_vals = (dtor_nchd(dst) > 1);
		const dtor *vals_dst = (free_vals)?dtor_chd(dst, 1):NULL;
		if (free_keys || free_vals) {
			hashmap_iter *it = hashmap_get_iter(hmap);
			FOREACH_IN_ITER(keyval, hashmap_entry, hashmap_iter_as_iter(it)) {
				if (free_keys) FINALISE(keyval->key, keys_dst);
				if (free_vals) FINALISE(keyval->val, vals_dst);
			}
			FREE(it);
		}
	}
}


size_t hashmap_sizeof()
{
	return sizeof(hashmap);
}


static inline uint64_t _hash(const hashmap *hmap, const void *key)
{
	// combine hashing with Fibonacci hashing
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


static inline void * _key_at(const hashmap *hmap, size_t pos)
{
	return hmap->entries + ( pos * (hmap->keysize + hmap->valsize) );
}


static inline void * _value_at(const hashmap *hmap, size_t pos)
{
	return hmap->entries + ( ( pos * (hmap->keysize + hmap->valsize) ) + hmap->keysize);
}


typedef struct {
	size_t pos;
	bool found;
} _find_res;


// Find the target position of the key in the table
static _find_res _find(const hashmap *hmap, const void *key, uint64_t h)
{
	uint64_t h1 = _h1(h);
	uint64_t h2 = _h2(h);
	_find_res ret = {.found=false, .pos=h1 % hmap->cap};
	//printf("starting probe at pos %zu\n", ret.pos );
	while (true) {
		//printf("   probing pos %zu\n", ret.pos );
		if ( hmap->tally[ret.pos] == h2 &&
		     hmap->keyeq( key, _key_at(hmap, ret.pos) ) ) {
			ret.found = true;
			break;
		}
		if ( hmap->tally[ret.pos] == ST_EMPTY ) {
			break;
		}
		ret.pos = (ret.pos + 1) % hmap->cap;
	}
	return ret;
}

/*

#define GROUPSIZE 16

typedef union _g16b {
    byte_t v[16];
    __m128i r;
} g16b;


static _find_res _find_sse(hashmap *hmap, void *key, uint64_t h)
{
    uint64_t h1 = _h1(h);
    uint64_t h2 = _h2(h);
    //size_t pos = h1 % hmap->cap;
    //printf("starting probe at pos %zu\n", ret.pos );
    _find_res ret = {.found=false, .pos=h1 % hmap->cap};
    size_t group = ret.pos / GROUPSIZE;
    size_t ngroups = (size_t)multceil((double)hmap->cap , ((double)GROUPSIZE));
    size_t tested_groups = 0;
    uint32_t stamp = 0x00000001;
    ret.pos = group * GROUPSIZE;
    while ( tested_groups < ngroups ) {
        g16b grp_tally = ((g16b *)(hmap->tally+(group*GROUPSIZE)))[0];
        uint32_t eqmask = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(h2), grp_tally.r));
        if (eqmask == 0x00) {
            return ret;
        }
        while (eqmask) {
            int p = uint32_lobit(eqmask);
            if ( hmap->keyeq( key, _key_at(hmap, ret.pos + p) ) )  {
                ret.found = true;
                ret.pos = ret.pos + p;
            }
            eqmask &= (~(stamp<<p));
        }
        tested_groups++;
        ret.pos = MIN(hmap->cap, ret.pos + GROUPSIZE) % hmap->cap;
    }
    //printf("returning pos=%zu found=%d\n", ret.pos, (int)ret.found );
    return ret;
}

*/

bool hashmap_has_key(const hashmap *hmap, const void *key)
{
	return _find(hmap, key, _hash(hmap, key)).found;
}


const void *hashmap_get(const hashmap *hmap, const void *key)
{
	_find_res qry = _find(hmap, key, _hash(hmap, key));
	if (qry.found) {
		return _value_at(hmap, qry.pos);
	} else {
		return NULL;
	}
}


void *hashmap_get_mut(const hashmap *hmap, const void *key)
{
	return (void *)hashmap_get(hmap, key);
}


static void _print(const hashmap *hmap)
{
	printf("Hashmap at %p\n", hmap);
	char *c = cstr_new(8);
	for (size_t i=0; i<hmap->cap; i++) {
		byte_to_str(hmap->tally[i], c);
		printf("   %zu) %s\n", i, c);
	}
	FREE(c);
}


static inline void _set(hashmap *hmap, const void *key, const void *val)
{
	uint64_t h = _hash(hmap, key);
	_find_res qry = _find(hmap, key, h);
	if (!qry.found) {
		memcpy(_key_at(hmap, qry.pos), key, hmap->keysize);
		hmap->tally[qry.pos] = _h2(h);
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



static void _resize(hashmap *hmap, size_t new_cap)
{
	size_t old_cap = hmap->cap;
	size_t old_size = hmap->size;
	void   *old_data = hmap->data;
	byte_t *old_tally = (byte_t *) old_data;
	void   *old_entries = old_data + old_cap;

	_reset_data(hmap, new_cap);
	//_print(hmap);

	//size_t rehash_attempts = 0;
	for (size_t i = 0; i < old_cap; ++i) {
		if (! (old_tally[i] >> 7) ) {
			//rehash_attempts += 1;
			//printf("rehashing element at pos %zu\n",i);
			_set( hmap,
			      old_entries + ( i * ( hmap->keysize + hmap->valsize ) ),
			      old_entries + ( i * ( hmap->keysize + hmap->valsize ) ) + hmap->keysize );
			//_print(hmap);
			//if (rehash_attempts!=hmap->size) {
			//    printf("failed to rehash pos %zu\n",i );
			//}
		}
	}
	assert(old_size == hmap->size);
	FREE(old_data);
}


static void _check_resize(hashmap *hmap)
{	
	if (hmap->occ < hmap->max_occ) {
		return;
	}
	_resize( hmap, (size_t)(GROW_BY * hmap->cap) );
}

	

void hashmap_set(hashmap *hmap, const void *key, const void *val)
{
	assert(key != NULL);
	_check_resize(hmap);
	_set(hmap, key, val);
}


void hashmap_unset(hashmap *hmap, const void *key)
{
	assert(key != NULL);
	uint64_t h = _hash(hmap, key);
	_find_res qry = _find(hmap, key, h);
	if (qry.found) {
		hmap->tally[qry.pos] = ST_DEL;
		hmap->size--;
		_check_resize(hmap);
	}
}


size_t hashmap_size(const hashmap *map)
{
	return map->size;
}


void hashmap_fit(hashmap *hmap) 
{
	size_t new_cap;
	for ( new_cap = MIN_CAPACITY; 
		  hmap->size >= MAX_LOAD * new_cap; 
		  new_cap = (size_t)(new_cap * GROW_BY) );
	_resize(hmap, new_cap);
}


struct _hashmap_iter {
	iter _t_iter;
	const hashmap *src;
	size_t index;
	hashmap_entry entry;
};


static bool _hashmap_iter_has_next(iter *it)
{
	hashmap_iter *hmit = (hashmap_iter *)it->impltor;
	return (hmit->index < hmit->src->cap);
}


static void _hashmap_iter_goto_next(hashmap_iter *hmit)
{
	while ((hmit->index < hmit->src->cap) &&
	       (hmit->src->tally[hmit->index] >> 7))
		hmit->index++;
}


static const void *_hashmap_iter_next(iter *it)
{
	hashmap_iter *hmit = (hashmap_iter *)it->impltor;
	if (hmit->index >= hmit->src->cap) {
		return NULL;
	}
	hmit->entry.key = _key_at(hmit->src, hmit->index);
	hmit->entry.val = _value_at(hmit->src, hmit->index);
	hmit->index++;
	_hashmap_iter_goto_next(hmit);
	return &hmit->entry;
}


static iter_vt _hashmap_iter_vt = { .has_next = _hashmap_iter_has_next,
                                    .next = _hashmap_iter_next
                                  };


hashmap_iter *hashmap_get_iter(const hashmap *src)
{
	hashmap_iter *ret = NEW(hashmap_iter);
	ret->_t_iter.vt = &_hashmap_iter_vt;
	ret->_t_iter.impltor = ret;
	ret->src = src;
	ret->index = 0;
	_hashmap_iter_goto_next(ret);
	return ret;
}


IMPL_TRAIT(hashmap_iter, iter);


#define HASHMAP_GET_IMPL( TYPE )\
TYPE hashmap_get_##TYPE(hashmap *hmap, const void *key) {\
	const void *v = hashmap_get(hmap, key);\
	return v ? ((TYPE *)v)[0] : (TYPE)0;\
}


#define HASHMAP_SET_IMPL( TYPE )\
void hashmap_set_##TYPE(hashmap *hmap, const void *key, TYPE val) {\
	hashmap_set(hmap, key, &val);\
}


#define HASHMAP_ALL_IMPL( TYPE, ... )\
HASHMAP_GET_IMPL(TYPE)\
HASHMAP_SET_IMPL(TYPE)


XX_CORETYPES(HASHMAP_ALL_IMPL)
