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


#ifndef ORDER_H
#define ORDER_H

/**
 * @file order.h
 * @author Paulo Fonseca
 * @brief Comparison functions (order, equality)
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Total order comparison function prototype.
 *
 * @return Returns an integer `C` such that
 * - `C == 0` iff data pointed by @p left and @p right compare as
 * **equals**
 * - `C < 0` iff data pointed by @p left is **less than** data pointed
 * by **right**
 * - `C > 0` iff data pointed by @p left is **greater than** data pointed
 * by **right**
 *
 * @see cmp_`type` for type-specific comparison
 */
typedef int (*cmp_func)(const void *left, const void *right);


/**
 * @brief Equality comparison function prototype.
 *
 * @return Returns a boolean `E` such that
 * `E == true` iff data pointed by @p left and @p right compare as
 * **equals**. Otherwise `E==false`.
 *
 * @see eq_`type` for type-specific comparison
 */
typedef bool (*eq_func)(const void *left, const void *right);

/**
 * @brief Provides type-specific order comparison functions,
 * e.g. cmp_float, cmp_uin64_t, etc.
 * @p leff and @p right are assumed to point to memory locations
 * containing values of the intended type, which are compared
 * in the usual (compiler-defined) sense.
 */
#define CMP_DECL( TYPE )\
int cmp_##TYPE(const void *left, const void *right);

/**
 * @brief Provides type-specific equality comparison functions,
 * e.g. eq_float, eq_uin64_t, etc.
 * @p leff and @p right are assumed to point to memory locations
 * containing values of the intended type, which are compared
 * in the usual (compiler-defined) sense.
 */
#define EQ_DECL( TYPE )\
bool eq_##TYPE(const void *left, const void *right);

///@cond
#define CMPEQ_DECL( TYPE )\
CMP_DECL( TYPE )\
EQ_DECL( TYPE )

CMPEQ_DECL(int)
CMPEQ_DECL(long)
CMPEQ_DECL(float)
CMPEQ_DECL(double)
CMPEQ_DECL(size_t)
CMPEQ_DECL(char)
CMPEQ_DECL(int8_t)
CMPEQ_DECL(int16_t)
CMPEQ_DECL(int32_t)
CMPEQ_DECL(int64_t)
CMPEQ_DECL(uint8_t)
CMPEQ_DECL(uint16_t)
CMPEQ_DECL(uint32_t)
CMPEQ_DECL(uint64_t)
////@endcond



#endif