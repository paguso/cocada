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

#include <coretype.h>

typedef struct _vebset vebset;

vebset *vebset_new();

void vebset_free(vebset *self);

size_t vebset_size(vebset *self);

bool vebset_contains(vebset *self, uint32_t x);

bool vebset_add(vebset *self, uint32_t x);

int64_t vebset_succ(vebset *self, uint32_t x);

int64_t vebset_pred(vebset *self, uint32_t x);

int64_t vebset_min(vebset *self);

int64_t vebset_max(vebset *self);

bool vebset_del(vebset *self, uint32_t x);


#endif