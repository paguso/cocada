#include<stddef.h>
#include<stdint.h>

#include "hash.h"

#define IDENT_HASH( TYPE ) \
uint64_t ident_hash_##TYPE(const void *key) {\
    return (uint64_t)(*((TYPE *)key));\
}

IDENT_HASH(uint16_t)
IDENT_HASH(uint32_t)
IDENT_HASH(uint64_t)
IDENT_HASH(size_t)


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
