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

#include <stddef.h>
#include <stdlib.h>

#include "new.h"
#include "stack.h"
#include "vec.h"



stack *stack_new(size_t typesize)
{
	return vec_new(typesize);
}


void stack_finalise(void *ptr, const finaliser *fnr)
{
	vec_finalise(ptr, fnr);
}


bool stack_empty(const stack *s)
{
	return (vec_len(s)==0);
}


void stack_push(stack *s, const void *elt)
{
	vec_push(s, elt);
}


void stack_pop(stack *s, void *dest)
{
	return vec_pop(s, vec_len(s)-1, dest);
}


void stack_peek(stack *s, void *dest)
{
	return vec_get_cpy(s, vec_len(s)-1, dest);
}


#define STACK_PUSH_IMPL( TYPE ) \
	void stack_push_##TYPE(stack *s, TYPE val)\
	{ stack_push(s, &val); }


#define STACK_POP_IMPL( TYPE ) \
	TYPE stack_pop_##TYPE(stack *q)\
	{ TYPE val; stack_pop(q, &val); return val;}


#define STACK_PEEK_IMPL( TYPE ) \
	TYPE stack_peek_##TYPE(stack *q)\
	{ TYPE val; stack_peek(q, &val); return val;}


#define STACK_ALL_IMPL( TYPE , ...) \
	STACK_PUSH_IMPL(TYPE) \
	STACK_POP_IMPL(TYPE) \
	STACK_PEEK_IMPL(TYPE)

XX_CORETYPES(STACK_ALL_IMPL)
