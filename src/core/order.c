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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "order.h"

#define CMP_IMPL( TYPE )\
int cmp_##TYPE(const void *pl, const void *pr) {\
    TYPE l = *((TYPE *)pl);\
    TYPE r = *((TYPE *)pr);\
    if (l == r) return 0;\
    else if (l < r) return -1;\
    else return +1;}

#define EQ_IMPL( TYPE )\
bool eq_##TYPE(const void *pl, const void *pr)\
    { return   *((TYPE *)pl) == *((TYPE *)pr); }


#define CMPEQ_IMPL( TYPE , ...)\
CMP_IMPL( TYPE )\
EQ_IMPL( TYPE )

XX_PRIMITIVES(CMPEQ_IMPL)