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
#include "cstrutil.h"
#include "hashmap.h"
#include "mathutil.h"
//#include "marshall.h"
#include "strstream.h"



typedef enum {
	ARRAY = 0,
	FUNC  = 1
} rankmode;


struct _alphabet {
	alphabet_type  type;
	rankmode rank_mode;
	size_t   size;
	char    *letters;
	union {
		size_t        *arr;
		char_rank_func func;
	} ranks;
};


alphabet *alphabet_new(const size_t size, const char *letters)
{
	alphabet *ret;
	ret =  NEW(alphabet);
	ret->type = CHAR_TYPE;
	ret->rank_mode = ARRAY;
	ret->size = size;
	ret->letters = cstr_new(size);
	strncpy(ret->letters, letters, size);
	ret->ranks.arr = ARR_NEW(size_t, UCHAR_MAX);
	ARR_FILL(ret->ranks.arr, 0, UCHAR_MAX, size);
	for (size_t i=0; i<size; i++)
		ret->ranks.arr[(size_t)letters[i]]=i;
	return ret;
}


static inline size_t int_ab_rank(xchar_t c)
{
	return (size_t)c;
}


alphabet *int_alphabet_new(size_t size)
{
	alphabet *ret;
	ret =  NEW(alphabet);
	ret->type = INT_TYPE;
	ret->rank_mode = FUNC;
	ret->size = size;
	ret->letters = NULL;
	ret->ranks.func = int_ab_rank;
	return ret;
}


alphabet *alphabet_clone(const alphabet *src)
{
	switch (src->type) {
	case CHAR_TYPE:
		return alphabet_new(src->size, src->letters);
		break;
	case INT_TYPE:
		return int_alphabet_new(src->size);
		break;
	}
	return NULL;
}


void ab_print(const alphabet *ab)
{
	printf("alphabet@%p\n",(void *)ab);
	printf("  size = %zu\n", ab->size);
	if (ab->type==CHAR_TYPE) {
		printf("  letters = %s\n", ab->letters);
	}
	else if (ab->type==INT_TYPE) {
		printf("  letters = 0..%zu\n",ab->size-1);
	}
}


void alphabet_free(alphabet *ab)
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


void alphabet_finalise(void *ptr, const finaliser *fnr)
{
	alphabet *ab = (alphabet *)ab;
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


alphabet_type ab_type(const alphabet *ab)
{
	return ab->type;
}


size_t ab_size(const alphabet *ab)
{
	return ab->size;
}


bool ab_contains(const alphabet *ab, xchar_t c)
{
	return (ab_rank(ab, c) < ab->size);
}



xchar_t ab_char(const alphabet *ab, size_t index)
{
	switch (ab->type) {
	case CHAR_TYPE:
		return (char)(ab->letters[index]);
		break;
	case INT_TYPE:
		return (xchar_t)index;
		break;
	}
	return 0;
}


size_t ab_rank(const alphabet *ab, xchar_t c)
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


int ab_cmp(const alphabet *ab, xchar_t a, xchar_t b)
{
	size_t ra = ab_rank(ab, a);
	size_t rb = ab_rank(ab, b);
	if (ra==rb) return 0;
	else if (ra<rb) return -1;
	else return +1;
}
