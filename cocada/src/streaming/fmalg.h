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

#ifndef FMALG_H
#define FMALG_H

#include <inttypes.h>


typedef struct _fmalg fmalg;


fmalg *fmalg_init_single(uint64_t maxval);


fmalg *fmalg_init(uint64_t maxval, size_t navg, size_t mmedian);


void fmalg_free(fmalg *fm);


void fmalg_reset(fmalg *fm);


void fmalg_process(fmalg *fm, uint64_t val);


uint64_t fmalg_query(fmalg *fm);


#endif