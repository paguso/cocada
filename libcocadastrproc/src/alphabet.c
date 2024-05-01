/*
 *
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

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alphabet.h"
#include "arrays.h"
#include "arrays.h"
#include "new.h"
#include "order.h"
#include "coretype.h"
#include "errlog.h"
#include "cstrutil.h"
#include "hashmap.h"
#include "mathutil.h"


#define UCHAR_RANGE (UCHAR_MAX+1)

typedef enum {
	ARRAY = 0,
	FUNC  = 1
} rankmode;


struct _Alphabet {
	AlphabetType  type;
	rankmode rank_mode;
	size_t   size;
	char    *letters;
	union {
		size_t        *arr;
		char_rank_func func;
	} ranks;
};


Alphabet *alphabet_new(size_t size, const char *letters)
{
	Alphabet *ret;
	ret =  NEW(Alphabet);
	ret->type = CHAR_TYPE;
	ret->rank_mode = ARRAY;
	ret->letters = ARR_NEW(char, size);
	ret->ranks.arr = ARR_NEW(size_t, UCHAR_RANGE);
	ARR_FILL(ret->ranks.arr, 0, UCHAR_RANGE, size);
	ret->size = 0;
	for (size_t i = 0; i < size; i++) {
		uchar c = letters[i];
		if (ret->ranks.arr[c] == size) {
			ret->ranks.arr[c] = ret->size;
			ret->letters[ret->size] = c;
			ret->size++;
		}
		else {
			WARN("Character '%c' has already been assigned rank %zu. Ignoring.\n", (char)c,
			     ret->ranks.arr[c]);
		}
	}
	for (size_t i = 0; i < UCHAR_RANGE; i++) {
		ret->ranks.arr[i] = MIN(ret->ranks.arr[i], ret->size);
	}
	ret->letters = cstr_resize(ret->letters, ret->size);
	return ret;
}


Alphabet *alphabet_new_with_equivs(size_t size, char **letters)
{
	Alphabet *ret = NEW(Alphabet);
	ret->type = CHAR_TYPE;
	ret->letters = cstr_new(size);
	ret->size = 0;
	ret->ranks.arr = ARR_NEW(size_t, UCHAR_RANGE);
	ret->rank_mode = ARRAY;
	ARR_FILL(ret->ranks.arr, 0, UCHAR_RANGE, size);

	for (size_t i = 0; i < size; i++) {
		bool i_used = false;
		for (size_t j = 0, l = strlen(letters[i]); j < l; j++) {
			uchar c = letters[i][j];
			if (ret->ranks.arr[c] == size) {
				ret->ranks.arr[c] = ret->size;
				if (!i_used) {
					ret->letters[ret->size] = c;
				}
				i_used = true;
			}
			else {
				WARN("Character '%c' has already been assigned rank %zu. Ignoring.\n", (char)c,
				     ret->ranks.arr[c]);
			}
		}
		if (i_used) {
			ret->size++;
		}
	}
	for (size_t i = 0; i < UCHAR_RANGE; i++) {
		ret->ranks.arr[i] = MIN(ret->ranks.arr[i], ret->size);
	}
	ret->letters = cstr_resize(ret->letters, ret->size);
	return ret;
}


static inline size_t int_ab_rank(xchar c)
{
	return (size_t)c;
}


Alphabet *alphabet_new_int_ab(size_t size)
{
	Alphabet *ret;
	ret =  NEW(Alphabet);
	ret->type = INT_TYPE;
	ret->rank_mode = FUNC;
	ret->size = size;
	ret->letters = NULL;
	ret->ranks.func = int_ab_rank;
	return ret;
}


Alphabet *alphabet_clone(const Alphabet *src)
{
	switch (src->type) {
	case CHAR_TYPE:
		return alphabet_new(src->size, src->letters);
		break;
	case INT_TYPE:
		return alphabet_new_int_ab(src->size);
		break;
	}
	return NULL;
}


void alphabet_free(Alphabet *ab)
{
	if (ab == NULL) return;
	switch (ab->rank_mode) {
	case ARRAY:
		FREE(ab->ranks.arr);
		break;
	default:
		break;
	}
	FREE(ab->letters);
	FREE(ab);
}


void alphabet_finalise(void *ptr, const Finaliser *fnr)
{
	Alphabet *ab = (Alphabet *)ptr;
	if (ab == NULL) return;
	switch (ab->rank_mode) {
	case ARRAY:
		FREE(ab->ranks.arr);
		break;
	default:
		break;
	}
	FREE(ab->letters);
}


AlphabetType alphabet_type(const Alphabet *ab)
{
	return ab->type;
}


size_t alphabet_size(const Alphabet *ab)
{
	return ab->size;
}


bool alphabet_contains(const Alphabet *ab, xchar c)
{
	return (alphabet_rank(ab, c) < ab->size);
}


xchar alphabet_char(const Alphabet *ab, size_t index)
{
	switch (ab->type) {
	case CHAR_TYPE:
		return (char)(ab->letters[index]);
		break;
	case INT_TYPE:
		return (xchar)index;
		break;
	}
	return 0;
}


size_t alphabet_rank(const Alphabet *ab, xchar c)
{
	switch (ab->type) {
	case CHAR_TYPE:
		return ab->ranks.arr[(unsigned char)c];
		break;
	case INT_TYPE:
		return MIN(ab->ranks.func(c), ab->size);
		break;
	}
	return ab->size;
}


int alphabet_cmp(const Alphabet *ab, xchar a, xchar b)
{
	size_t ra = alphabet_rank(ab, a);
	size_t rb = alphabet_rank(ab, b);
	if (ra == rb) return 0;
	else if (ra < rb) return -1;
	else return +1;
}
