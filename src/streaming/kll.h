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

#ifndef KLL_H
#define KLL_H

#include <stdio.h>

#include "coretype.h"
#include "order.h"

typedef struct __kllsumm kllsumm;


kllsumm *kll_new(size_t typesize, cmp_func cmp, double eps);

kllsumm *kll_new_with_cap(size_t typesize, cmp_func cmp, double eps, size_t cap);

void kll_upd(kllsumm *self, void *val);

size_t kll_rank(kllsumm *self, void *val);

void kll_print(kllsumm *self, FILE *stream, void (*print_val)(FILE *, const void *));


#endif