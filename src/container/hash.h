#ifndef _HASH_H_
#define _HASH_H_


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "bitsandbytes.h"

/**
 * @file hash.h
 * @author Paulo Fonseca
 * @brief General hash function definitions
 */


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



#define IDENT_HASH_DECL( TYPE ) \
uint64_t ident_hash_##TYPE(const void *key);

IDENT_HASH_DECL(uint16_t)
IDENT_HASH_DECL(uint32_t)
IDENT_HASH_DECL(uint64_t)
IDENT_HASH_DECL(size_t)


uint64_t fib_hash(uint64_t key);

/**
 * Simplified 64-bit FNV hashing
 * @see source: http://www.isthe.com/chongo/tech/comp/fnv
 */
uint64_t fnv1a_64bit_hash(void *obj, size_t objsize);


/**
 * @brief djb2 string hash function
 * @see Source: http://www.cse.yorku.ca/~oz/hash.html
 */
size_t djb2_hash(const unsigned char *str);

#endif