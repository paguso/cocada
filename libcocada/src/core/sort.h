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

#ifndef SORT_H
#define SORT_H

#include "coretype.h"
#include "order.h"

/**
 * @brief Sort an array in place using the Quicksort algorithm.
 * 
 * @param arr The array to be sorted.
 * @param n The number of elements in the array.
 * @param typesize The size of each element in the array.
 * @param cmp The comparison function.
 */
void quicksort(void *arr, size_t n, size_t typesize, cmp_func *cmp);


/**
 * @brief Sort the indexes of an array Quicksort algorithm.
 * 
 * @param arr The array to be sorted.
 * @param n The number of elements in the array.
 * @param typesize The size of each element in the array.
 * @param cmp The comparison function.
 * @return The array `I=(I[0]...I[n-1])` of sorted indexes of the original array s.t. for all `0 <= q < n`, `arr[I[q]] <= arr[I[q+1]]` according to @p cmp.
 */
size_t *index_quicksort(void *arr, size_t n, size_t typesize, cmp_func *cmp);



#endif