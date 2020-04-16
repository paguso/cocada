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

#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#include "coretype.h"
#include "new.h"
#include "vec.h"

/**
 * @file stack.h
 * @author Paulo Fonseca
 *
 * @brief Stack: dynamic linear collection with LIFO access policy.
 */


/**
 * Stack type
 */
typedef vec stack;


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements in bytes.
 */
stack *stack_new(size_t typesize);


/**
 * @brief Destructor.
 */
void stack_dtor(void *ptr, const dtor *dt);


/**
 * @brief Checks whether the stack is empty.
 */
bool stack_empty(const stack *s);


/**
 * @brief Pushes an element onto the top of a stack.
 */
void stack_push(stack *s, const void *elt);


/**
 * @brief Pops an element from the top of a nonempty stack.
 */
void stack_pop(stack *s, void *dest);



#define STACK_PUSH_DECL( TYPE ) \
    void stack_push_##TYPE(stack *s, TYPE val);

#define STACK_POP_DECL( TYPE ) \
    TYPE stack_pop_##TYPE(stack *q);

#define STACK_ALL_DECL( TYPE ) \
STACK_PUSH_DECL(TYPE) \
STACK_POP_DECL(TYPE)

STACK_ALL_DECL(char)
STACK_ALL_DECL(uchar)
STACK_ALL_DECL(short)
STACK_ALL_DECL(ushort)
STACK_ALL_DECL(int)
STACK_ALL_DECL(uint)
STACK_ALL_DECL(long)
STACK_ALL_DECL(ulong)
STACK_ALL_DECL(llong)
STACK_ALL_DECL(ullong)
STACK_ALL_DECL(float)
STACK_ALL_DECL(double)
STACK_ALL_DECL(ldouble)
STACK_ALL_DECL(bool)
STACK_ALL_DECL(size_t)
STACK_ALL_DECL(int8_t)
STACK_ALL_DECL(uint8_t)
STACK_ALL_DECL(int16_t)
STACK_ALL_DECL(uint16_t)
STACK_ALL_DECL(int32_t)
STACK_ALL_DECL(uint32_t)
STACK_ALL_DECL(int64_t)
STACK_ALL_DECL(uint64_t)
STACK_ALL_DECL(byte_t)
STACK_ALL_DECL(rawptr)
STACK_ALL_DECL(cstr)


#endif