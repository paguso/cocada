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

#ifndef FASTAIDX_H
#define FASTAIDX_H

#include <stddef.h>

#include "new.h"

typedef struct _fastaidx fastaidx;

fastaidx *fastaidx_new(const char *src_path);

void fastaidx_finalise(void *ptr, const finaliser *fnr);

size_t fastaidx_size(fastaidx *self);

void fastaidx_add(fastaidx *self, size_t descr_offset, size_t seq_offset);

typedef struct {
	size_t descr_off;
	size_t seq_off;
} fasta_rec_offs;

fasta_rec_offs fastaidx_get(fastaidx *self, size_t rec_no);

#endif