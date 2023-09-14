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

/**
 * @file sort.h
 * @author Paulo Fonseca
 * @brief Sorting algorithms.
 */

#include "coretype.h"
#include "order.h"

/**
 * @brief Sort an array in place using Quicksort using the median of three pivot choice heuristic.
 *
 * @param arr The array to be sorted.
 * @param n The number of elements in the array.
 * @param typesize The size of each element in the array.
 * @param cmp The comparison function.
 */
void quicksort(void *arr, size_t n, size_t typesize, cmp_func cmp);


/**
 * @brief Sort the indexes of an array via Quicksort using the median of three pivot choice heuristic.
 *
 * @param arr The array to be sorted.
 * @param n The number of elements in the array.
 * @param typesize The size of each element in the array.
 * @param cmp The comparison function.
 * @return The array `I=(I[0]...I[n-1])` of sorted indexes of the original array s.t. for all `0 <= q < n`, `arr[I[q]] <= arr[I[q+1]]` according to @p cmp.
 */
size_t *index_quicksort(void *arr, size_t n, size_t typesize, cmp_func cmp);


/**
 * @brief Finds the index of the first element in a sorted array that is greater or equal to a given value.
 *
 * @param sorted_arr The sorted array
 * @param n The number of elements in the array
 * @param typesize The size of each element in the array in bytes
 * @param cmp The comparison function
 * @param val The value to be compared
 * @return size_t The index of the first element in the array that is greater or equal to @p val.
 */
size_t succ(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp,
            void *val);


/**
 * @brief Finds the index of the first element in a sorted array that is strictly greater than a given value.
 *
 * @param sorted_arr The sorted array
 * @param n The number of elements in the array
 * @param typesize The size of each element in the array in bytes
 * @param cmp The comparison function
 * @param val The value to be compared
 * @return size_t The index of the first element in the array that is strictly greater than @p val.
 */
size_t strict_succ(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp,
                   void *val);


/**
 * @brief Finds the index of the last element in a sorted array that is smaller or equal to a given value.
 *
 * @param sorted_arr The sorted array
 * @param n The number of elements in the array
 * @param typesize The size of each element in the array in bytes
 * @param cmp The comparison function
 * @param val The value to be compared
 * @return size_t The index of the last element in the array that is smaller or equal to @p val.
 */
size_t pred(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp,
            void *val);


/**
 * @brief Finds the index of the last element in a sorted array that is strictly smaller than a given value.
 *
 * @param sorted_arr The sorted array
 * @param n The number of elements in the array
 * @param typesize The size of each element in the array in bytes
 * @param cmp The comparison function
 * @param val The value to be compared
 * @return size_t The index of the last element in the array that is strictly smaller than @p val.
 */
size_t strict_pred(void *sorted_arr, size_t n, size_t typesize, cmp_func cmp,
                   void *val);

#endif