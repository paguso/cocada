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
typedef struct _FASTAIndex FASTAIndex;


/**
 * @brief
 */
FASTAIndex *fasta_index_new(const char *src_path);


/**
 * @brief Finaliser
 */
void fasta_index_finalise(void *ptr, const Finaliser *fnr);


/**
 * @brief Destructor
 */
void fasta_index_free(FASTAIndex *self);


/**
 * @brief
 */
usize fasta_index_size(FASTAIndex *self);


/**
 * @brief
 */
void fasta_index_add(FASTAIndex *self, usize descr_offset, usize seq_offset);


/**
 * @brief FASTA record offset.
 */
typedef struct {
	usize descr_offset;	/**< Record description offset. */
	usize seq_offset;	/**< Record sequence offset. */
} FASTARecOffsets;


/**
 * @brief Returns the offsets (description, sequence) of the record #@p rec_no.
 */
FASTARecOffsets fasta_index_get(FASTAIndex *self, usize rec_no);

#endif
