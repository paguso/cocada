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



/*
 * Simplified 64-bit FNV hashing
 * source: http://www.isthe.com/chongo/tech/comp/fnv
 */
uint64_t fnv1a_64bit_hash(void *obj, size_t objsize)
{
    uint64_t hval = 0;

    //FNV-1 hash each octet of the buffer
    for ( byte_t *bp = (byte_t *)obj, 
          *end = ((byte_t *)obj) + objsize; 
          bp < end; bp++ ) {
    	//xor the bottom with the current octet 
    	hval ^= (uint64_t)*bp;
	    // multiply by the 64 bit FNV magic prime mod 2^64 
#ifdef NO_FNV_GCC_OPTIMIZATION
	    hval *= ((uint64_t)0x100000001b3ULL);
#else
    	hval += (hval << 1) + (hval << 4) + (hval << 5) +
                (hval << 7) + (hval << 8) + (hval << 40);
#endif
    }
    return hval;
}

