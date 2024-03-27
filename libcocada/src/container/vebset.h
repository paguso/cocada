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

#ifndef vebset_H
#define vebset_H

#include "coretype.h"
#include "new.h"

/**
 * @file vebset.h
 * @brief van Emde Boas (vEB) set of integers.
 * @author Paulo Fonseca
 */


/**
 * @brief vEB set type
 */
typedef struct _vebset vebset;


/**
 * @brief Constructor
 */
vebset *vebset_new();


/**
 * Destructor. Equivalent to DESTROY_FLAT(self, vebset).
 * @see DESTROY_FLAT
 */
void vebset_free(vebset *self);


/**
 * @brief Finaliser. Should be called with an empty finaliser.
 */
void vebset_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Returns the cardinality of the set.
*/
size_t vebset_size(vebset *self);


/**
 * @brief Tests whether the set contains a value @p x.
*/
bool vebset_contains(vebset *self, uint32_t x);


/**
 * @brief Adds value @p x to the set.
 * @return true if @p x was added. false if @p x was already in the
 * set or is an invalid value.
*/
bool vebset_add(vebset *self, uint32_t x);


/**
 * @brief Deletes value @p x from the set.
 * @return true if @p x was deleted. false if @p x was not in the
 * set or is an invalid value.
*/
bool vebset_del(vebset *self, uint32_t x);


/**
 * @brief Returns the smallest value in the set strictly greater than
 * @p x if it exists. Otherwise, returns 2^32.
*/
int64_t vebset_succ(vebset *self, uint32_t x);


/**
 * @brief Returns the greatest value in the set strictly smaller than
 * @p x if it exists. Otherwise, returns -1.
 */
int64_t vebset_pred(vebset *self, uint32_t x);


/**
 * @brief Returns the minimum of the set, if the set is not empty,
 * or 2^32 otherwise.
*/
int64_t vebset_min(vebset *self);


/**
 * @brief Returns the maximum of the set, if the set is not empty,
 * or -1 otherwise.
*/
int64_t vebset_max(vebset *self);


#endif