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

#ifndef HASH_H
#define HASH_H


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "bitbyte.h"

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


IDENT_HASH_DECL(byte_t)
IDENT_HASH_DECL(char)
IDENT_HASH_DECL(short)
IDENT_HASH_DECL(int)
IDENT_HASH_DECL(long)
IDENT_HASH_DECL(size_t)
IDENT_HASH_DECL(int8_t)
IDENT_HASH_DECL(int16_t)
IDENT_HASH_DECL(int32_t)
IDENT_HASH_DECL(int64_t)
IDENT_HASH_DECL(uint8_t)
IDENT_HASH_DECL(uint16_t)
IDENT_HASH_DECL(uint32_t)
IDENT_HASH_DECL(uint64_t)


uint64_t fib_hash(uint64_t key);

/**
 * Simplified 64-bit FNV hashing
 * @see source: http://www.isthe.com/chongo/tech/comp/fnv
 */
uint64_t fnv1a_64bit_hash(const void *obj, size_t objsize);


/**
 * @brief djb2 string hash function
 * @see Source: http://www.cse.yorku.ca/~oz/hash.html
 */
size_t djb2_hash(const unsigned char *str);

#endif