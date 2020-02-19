#ifndef HASH_H
#define HASH_H


#include <stdint.h>

#include "bitsandbytes.h"



/**
 * @brief The hash function is used by the hash table to obtain a numeric key 
 *        for an element. This key is then used internally to determine the 
 *        location in which the element will be stored.
 */
typedef uint64_t (*hash_func)(const void *);


/**
 * @brief The equality function is used by the hashtable to compare elements. 
 *        It is related to the hash function in which elements that compare as
 *        equal *must* have the same hash key and, conversely, if two elements 
 *        have different hash values, then they must compare as different.
 */
typedef bool (*equals_func)(const void *, const void *);



uint64_t fib_hash(uint64_t key)
{
    return (key * 11400714819323198485llu);
}

uint64_t fnv_hash(byte_t key[], size_t keylen)
{
    const uint64_t p = 16777619;
    uint64_t hash = 2166136261;
    for (size_t i=0; i<keylen; ++i)
        hash = (hash ^ key[i]) * p;
    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    return hash;
}



#endif