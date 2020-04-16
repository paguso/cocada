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


void stack_dtor(void *ptr, const dtor *dt)
{
	vec_dtor(ptr, dt);
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


#define STACK_PUSH_IMPL( TYPE ) \
    void stack_push_##TYPE(stack *s, TYPE val)\
    { stack_push(s, &val); }


#define STACK_POP_IMPL( TYPE ) \
    TYPE stack_pop_##TYPE(stack *q)\
    { TYPE val; stack_pop(q, &val); return val;}


#define STACK_ALL_IMPL( TYPE ) \
STACK_PUSH_IMPL(TYPE) \
STACK_POP_IMPL(TYPE)


STACK_ALL_IMPL(char)
STACK_ALL_IMPL(uchar)
STACK_ALL_IMPL(short)
STACK_ALL_IMPL(ushort)
STACK_ALL_IMPL(int)
STACK_ALL_IMPL(uint)
STACK_ALL_IMPL(long)
STACK_ALL_IMPL(ulong)
STACK_ALL_IMPL(llong)
STACK_ALL_IMPL(ullong)
STACK_ALL_IMPL(float)
STACK_ALL_IMPL(double)
STACK_ALL_IMPL(ldouble)
STACK_ALL_IMPL(bool)
STACK_ALL_IMPL(size_t)
STACK_ALL_IMPL(int8_t)
STACK_ALL_IMPL(uint8_t)
STACK_ALL_IMPL(int16_t)
STACK_ALL_IMPL(uint16_t)
STACK_ALL_IMPL(int32_t)
STACK_ALL_IMPL(uint32_t)
STACK_ALL_IMPL(int64_t)
STACK_ALL_IMPL(uint64_t)
STACK_ALL_IMPL(byte_t)
STACK_ALL_IMPL(rawptr)
STACK_ALL_IMPL(cstr)
