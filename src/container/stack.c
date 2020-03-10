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
#include "vec.h"


typedef vec stack;


stack *stack_new(size_t typesize)
{
	return vec_new(typesize);
}


void stack_free(stack *s, bool free_elements)
{
	vec_free(s, free_elements);
}


void stack_dispose(void *ptr, const dtor *dt)
{
	vec_dispose(ptr, dt);
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


#define STACK_PUSH( TYPE ) \
    void stack_push_##TYPE(stack *s, TYPE val)\
    { stack_push(s, &val); }


#define STACK_POP( TYPE ) \
    TYPE stack_pop_##TYPE(stack *q)\
    { TYPE val; stack_pop(q, &val); return val;}


#define STACK_ALL_OPS( TYPE ) \
STACK_PUSH(TYPE) \
STACK_POP(TYPE)


STACK_ALL_OPS(int)
STACK_ALL_OPS(size_t)

