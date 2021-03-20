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

/**
 * @file fastaidx.h
 * @brief FASTA file index
 * @author Paulo Fonseca
 */

/**
 * @brief 
 */
typedef struct _fastaidx fastaidx;


/**
 * @brief 
 */
fastaidx *fastaidx_new(const char *src_path);


/**
 * @brief Finaliser
 */
void fastaidx_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief
 */
size_t fastaidx_size(fastaidx *self);


/**
 * @brief
 */
void fastaidx_add(fastaidx *self, size_t descr_offset, size_t seq_offset);


/**
 * @brief FASTA record offset.
 */
typedef struct {
	size_t descr_off;	/**< Record description offset. */
	size_t seq_off;	/**< Record sequence offset. */
} fasta_rec_offs;


/**
 * @brief Returns the offsets (description, sequence) of the record #@p rec_no.
 */
fasta_rec_offs fastaidx_get(fastaidx *self, size_t rec_no);

#endif