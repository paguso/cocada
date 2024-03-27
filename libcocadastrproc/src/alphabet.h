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

#ifndef ALPHABET_H
#define ALPHABET_H

#include <stddef.h>
#include <stdlib.h>

#include "new.h"
#include "xchar.h"

/**
 * @file alphabet.h
 * @author Paulo Fonseca
 *
 * @brief Finite ordered alphabet ADT.
 *
 * An alphabet is a finite ordered set of letters
 * A=(a[0], a[1], ..., a[l-1]) s.t. rank(a[j])==j, and so
 * a[0] < a[1] < ... < a[l-1] where < stands for the lexicographic order.
 *
 * Included is support for alphabets of two types:
 * 1. CHAR alphabets, whose characters form a subset of standard C char type,
 * 2. INT alphabets, whose characters are integers in the range 0..size-1
 *
 * For this reason, the characters taken and returned by functions
 * are of type xchar_t (eXtended char type), which is actually an
 * unsigned integer type large enough to hold both types of supported chars.
 */


/**
 * @brief Alphabet types returned by the ab_type() function.
 * @see ab_type()
 */
typedef enum {
	CHAR_TYPE = 0, /**< Character alphabet type */
	INT_TYPE  = 1  /**< Integer alphabet type */
} alphabet_type;


/**
 * @brief alphabet type
 */
typedef struct _alphabet alphabet;


/**
 * @brief char rank function type
 */
typedef size_t (*char_rank_func)(xchar_t c);


/**
 * @brief Creates a char alphabet from a string with letters in
 *        lexicographic order.
 * @param size Number of letters.
 * @param letters (no transfer) String with letters in lexicographic order.
 */
alphabet *alphabet_new(size_t size, const char *letters);

/**
 * @brief Creates a char alphabet with the possibility of multiple
 * chars being equivalente, e.g. lowercase/uppercase.
 *
 * For example, a DNA alphabet with equivalent lowercase/uppercase
 * could be defined as
 *
 * ```C
 * char *nucl[4] = {"Aa", "Cc", "Gg", "Tt"};
 * alphabet *dna = alphabet_new_with_equivs(4, nucl);
 * ```
 *
 * The parameter @p size indicates the size of the alphabet, i.e. the
 * number of distinct ranks, from 0 to @p size-1.
 * The parameter @p letters gives combinations of equivalent letter per rank
 * s.t. `letters[i]` contains a string with the equivalent letters
 * to be assigned rank `i`. Once a char in @p letters[i] is assigned a rank,
 * further appearences of the same char in @p letters[j] with `j>i` will be
 * ignored. If multiple equivalent chars are assigned to rank `r`,
 * calling ::ab_char(ab, `r`) will return the first of them.
 *
 *
 * @param size The size of the alphabet
 * @param letters The combinations of equivalent letters per rank
 * s.t. `letters[i]` contains a string with the equivalent letters
 * to be assigned rank `i`
 */
alphabet *alphabet_new_with_equivs(size_t size, char **letters);


/**
 * @brief Creates an integer alphabet with characters ranging from
 *        0 to @p size-1
 * @param size The alphabet size.
 */
alphabet *int_alphabet_new(size_t size);


/**
 * @brief Clones an alphabet
 */
alphabet *alphabet_clone(const alphabet *src);


/**
 * @brief Destructor.
 */
void alphabet_free(alphabet *ab);


/**
 * @brief Destructor
 */
void alphabet_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Return the base char type of the alphabet
 */
alphabet_type ab_type(const alphabet *ab);


/**
 * @brief Returns the number of letters.
 */
size_t ab_size(const alphabet *ab);


/**
 * @brief Indicates whether alphabet @p ab contains the character @c.
 */
bool ab_contains(const alphabet *ab, xchar_t c);


/**
 * @brief Returns the letter of given rank. If rank >= alphabet size, the
 * behaviour is undefined.
 */
xchar_t ab_char(const alphabet *ab, size_t rank);


/**
 * @brief Returns the rank of a given character @p c.
 * For any char @p c in @p ab, we must have: 0 <= rank(@p c) < size(@p ab).
 * For any char @p c not in @p ab, we must have rank(@p c)==size(@p ab).
 * This means that all chars not in the alphabet are lexicographically
 * identical and strictly greater than any char in the alphabet.
 */
size_t ab_rank(const alphabet *ab, xchar_t c);


/**
 * @brief Lexicographicaly compares two chars @p a and @p b;
 * @return -1 if rank(@p a)<rank(@p b); 0 if rank(@p a)==rank(@p b);
 *         or +1 if rank(@p a)==rank(@p b);
 * @see ab_rank
 */
int ab_cmp(const alphabet *ab, xchar_t a, xchar_t b);


#endif