/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

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



size_t djb2_hash(const unsigned char *str)
{
	size_t hash = 5381;
	int c;

	while ( (c = *(str++)) )
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}