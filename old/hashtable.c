#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "arrayutil.h"
#include "bitsandbytes.h"
#include "bitarray.h"
#include "bytearray.h"
#include "cocadautil.h"
#include "hashtable.h"

#define MIN_CAPACITY 16
#define MIN_LOAD 0.25
#define MAX_LOAD 0.5
#define GROW_BY 1.5

struct _hashtable {
    size_t      size;
    size_t      occupied;
    size_t      capacity;
    size_t      typesize;
    hash_func   hfunc;
    equals_func eqfunc;
    void       *vals;
    byte_t     *occ_slot;
    byte_t     *del_slot;
};

/*
 * returns 
 * - a free slot if elt not in table OR
 * - the slot containing an element that compares equals to elt
 */
static inline size_t hash( hashtable *ht, void *elt )
{
    size_t hashcode = ht->hfunc(elt);
    size_t pos = hashcode%(ht->capacity);
    while( bitarr_get_bit(ht->occ_slot, pos) && 
             ( bitarr_get_bit(ht->del_slot, pos) || 
               !ht->eqfunc(elt, ht->vals+(pos*ht->typesize)) ) ) 
        pos = (pos+1)%ht->capacity;
    return pos;
}


static void check_and_resize(hashtable *ht) 
{
    size_t new_capacity = ht->capacity;
    if ( ht->occupied < (size_t)(ht->capacity*MIN_LOAD) )
        new_capacity = MAX(MIN_CAPACITY, (size_t)(ht->capacity/GROW_BY));
    else if ( ht->occupied >= (size_t)(ht->capacity*MAX_LOAD) ) 
        new_capacity = (size_t)(GROW_BY * ht->capacity);
    if (new_capacity==ht->capacity)
        return;
    void   *new_vals = calloc(new_capacity, ht->typesize);  // init to zero
    byte_t *new_occ_slot  = bitarr_new(new_capacity);
    for (size_t pos=0; pos<ht->capacity; pos++) {
        if ( !bitarr_get_bit(ht->occ_slot, pos) ||
              bitarr_get_bit(ht->del_slot, pos) ) continue;
        void *elt = ht->vals+(pos*ht->typesize);
        size_t hashcode = ht->hfunc(elt);
        size_t new_pos = hashcode%(new_capacity);
        while( bitarr_get_bit(new_occ_slot, new_pos) ) // no dupl or del yet 
            new_pos = (new_pos+1)%new_capacity;
        memcpy( new_vals+(new_pos*ht->typesize), elt, ht->typesize );
        bitarr_set_bit(new_occ_slot, new_pos, 1);
    }
    FREE(ht->vals);
    FREE(ht->occ_slot);
    FREE(ht->del_slot);
    ht->capacity = new_capacity;
    ht->vals = new_vals;
    ht->occ_slot = new_occ_slot;
    ht->del_slot = bitarr_new(new_capacity);
    ht->occupied = ht->size;
} 


hashtable *hashtable_new(size_t typesize, hash_func hfunc, equals_func eqfunc)
{
    return hashtable_new_with_capacity(MIN_CAPACITY, typesize, hfunc, eqfunc);
}


hashtable *hashtable_new_with_capacity(size_t capacity, size_t typesize, 
                          hash_func hfunc, equals_func eqfunc )
{
    hashtable *ht = NEW(hashtable);
    ht->size      = 0;
    ht->occupied  = 0;
    ht->capacity  = MAX(capacity, MIN_CAPACITY);
    ht->typesize  = typesize;
    ht->hfunc     = hfunc;
    ht->eqfunc    = eqfunc;
    ht->vals      = calloc(ht->capacity, ht->typesize); // init to zero
    ht->occ_slot  = bitarr_new(ht->capacity);
    ht->del_slot  = bitarr_new(ht->capacity);
    return ht;
}


void hashtable_free(hashtable *ht, bool free_elts)
{
    if (ht==NULL) return;
    for (size_t i=0; free_elts && i<ht->capacity; i++) 
        if ( bitarr_get_bit(ht->occ_slot, i) && !bitarr_get_bit(ht->del_slot, i))
            FREE(((void **)(ht->vals))[i]);
    FREE(ht->vals);
    FREE(ht);
}


size_t hashtable_size(hashtable *ht) {
    return ht->size;
}


bool hashtable_contains(hashtable *ht, void *elt)
{
    size_t pos = hash( ht, elt );
    return bitarr_get_bit(ht->occ_slot, pos) && 
           !bitarr_get_bit(ht->del_slot, pos) ;
}


bool hashtable_add(hashtable *ht, void *elt)
{
    if (hashtable_contains(ht, elt)) return false; // duplicates not allowed
    check_and_resize(ht);
    size_t pos = hash(ht, elt);
    memcpy(ht->vals+(pos*ht->typesize), elt, ht->typesize); 
    bitarr_set_bit(ht->occ_slot, pos, 1);
    ht->size++;
    ht->occupied++;
    return true;
}


bool hashtable_del (hashtable *ht, void *elt, bool free_elt)
{
    if (!hashtable_contains(ht, elt)) return false; // elt not in table
    size_t pos = hash(ht, elt);
    if (free_elt) FREE(((void **)(ht->vals))[pos]);
    ht->size--;
    bitarr_set_bit(ht->del_slot, pos, 1);
    check_and_resize(ht);
    return true;
}


bool hashtable_get_cpy (hashtable *ht, void *elt, void *dest)
{
    if (!hashtable_contains(ht, elt)) return false; // elt not in table
    size_t pos = hash(ht, elt);
    memcpy(dest, ht->vals+(pos*ht->typesize), ht->typesize); 
    return true;
}


const void* hashtable_get (hashtable *ht, void *elt)
{
    if (!hashtable_contains(ht, elt)) return NULL; // elt not in table
    size_t pos = hash(ht, elt);
    return ht->vals+(pos*ht->typesize);
}

#define HT_CONTAINS( TYPE )\
    bool hashtable_contains_##TYPE( hashtable *ht, TYPE val )\
        {return hashtable_contains(ht, &val);}

#define HT_ADD( TYPE )\
    bool hashtable_add_##TYPE( hashtable *ht, TYPE val )\
        {return hashtable_add(ht, &val);}

#define HT_DEL( TYPE )\
    bool hashtable_del_##TYPE( hashtable *ht, TYPE val, bool free_elt )\
        {return hashtable_del(ht, &val, free_elt);}


#define HT_ALL_OPS( TYPE )\
HT_CONTAINS(TYPE)\
HT_ADD(TYPE)\
HT_DEL(TYPE)

HT_ALL_OPS(int)
HT_ALL_OPS(size_t)
HT_ALL_OPS(byte_t)



struct _ht_iterator {
    hashtable *src;
    size_t cur;
};

hashtable_iterator *hashtable_get_iterator(hashtable *ht)
{
    hashtable_iterator *it = NEW(hashtable_iterator);
    it->src = ht;
    it->cur = 0;
    return it;
}


void hashtable_iterator_free(hashtable_iterator *htit) {
    FREE(htit);
}

bool ht_iterator_has_next(hashtable_iterator *htit)
{
    while ( htit->cur < htit->src->capacity &&
            ( !bitarr_get_bit(htit->src->occ_slot, htit->cur) ||
              bitarr_get_bit(htit->src->del_slot, htit->cur) ) )
        htit->cur++;
    return htit->cur < htit->src->capacity;
}

const void *ht_iterator_next(hashtable_iterator *htit)
{
    if (ht_iterator_has_next(htit)) {
        void *ret =  htit->src->vals + (htit->cur * htit->src->typesize);
        htit->cur++;
        return ret;
    }
    else 
        return NULL;
}



