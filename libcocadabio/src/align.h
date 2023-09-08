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

#ifndef ALIGN_H
#define ALIGN_H

/**
 * @file align.h
 * @author Paulo Fonseca
 * @brief Sequence alignment algorithms
 */

#include "coretype.h"
#include "arrays.h"
#include "strbuf.h"

/**
 * @brief Uses the Needleman Wunsch dynamic programming algorithm to compute the global
 * alignment of two strings using unit costs for insertion, deletion and mismatch, returning 
 * the cost of the alignment.
 * If a @p cigar buffer is provided, a CIGAR string of the aligment is appended to it. 
 * If multiple alignments are possible, only one is returned, prioritizing mismatches over
 * indels. The CIGAR operations are relative to the query string.
 * 
 * @return int The cost of the alignment.
 */
int simple_global_align(const char *qry, size_t qry_len, const char *tgt,
                        size_t tgt_len, strbuf *cigar);


#endif // ALIGN_H