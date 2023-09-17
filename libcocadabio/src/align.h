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
 * @brief Substitution cost function type
 */
typedef int (*subst_cost_fn)(char a, char b);


/**
 * @brief Returns the cost of substituting @p a by @p b defined as
 * 1 if @p a != @p b and 0 otherwise.
 * 
 * @param a The first character
 * @param b The second character
 * @return int The cost of substituting @p a by @p b
 */
int unit_subst(char a, char b);


/**
 * @brief Uses the Needleman Wunsch dynamic programming algorithm to compute the global
 * alignment of two strings using unit costs for insertion, deletion and mismatch, returning 
 * the cost of the alignment.
 * If a @p cigar buffer is provided, a CIGAR string of the aligment is appended to it. 
 * If multiple alignments are possible, only one is returned, prioritizing mismatches over
 * indels. The CIGAR operations are relative to the query string.
 * 
 * @note The algorithm runs in O(nm) time and O(nm) space, where m = @p qry_len and n= @p tgt_len.
 * @return int The cost of the alignment.
 */
int simple_global_align(const char *qry, size_t qry_len, const char *tgt, size_t tgt_len, strbuf *cigar);


/** 
 * @brief Uses the Myers&Miller algorithm to compute the global affine cost of the global
 * alignment of two strings.
 * If a @p cigar buffer is provided, a CIGAR string of the aligment is appended to it. 
 * The CIGAR operations are relative to the query string.
 * If multiple alignments are possible, only one is returned as described in 
 * "E.W. Myers, W. Miller, Optimal Alignments in Linear Space, Bioinformatics 4:1 (1988)"
 * 
 * @note The algorithm runs in O(nm) time and O(n) space, where m = @p qry_len and n= @p tgt_len.
 * @param qry The query string 
 * @param qry_len The length of the query string
 * @param tgt The target string
 * @param tgt_len The length of the target string
 * @param gap_open The cost of opening a gap
 * @param gap_ext The cost of extending a gap
 * @param subst The substitution cost function
 * @param cigar The string buffer on which to append the CIGAR string of the alignment
 * @return int The cost of the alignment.
 * @see "E.W. Myers, W. Miller, Optimal Alignments in Linear Space, Bioinformatics, Volume 4,
 * Issue 1, March 1988, Pages 11–17, https://doi.org/10.1093/bioinformatics/4.1.11"
 */
int affine_global_align(const char *qry, size_t qry_len, const char *tgt, size_t tgt_len, 
    int gap_open, int gap_ext, subst_cost_fn subst, strbuf *cigar);

/**
 * @brief Prints the alignment of two strings to the given file stream.
 * 
 * @param out Output file stream
 * @param qry The query string
 * @param qry_from The start position of the query string
 * @param qry_to The end position of the query string
 * @param tgt The target string
 * @param tgt_from The start position of the target string
 * @param tgt_to The end position of the target string
 * @param cigar The CIGAR string of the alignment
 * @param cigar_len The length of the CIGAR string
 */
void fprintf_alignment(FILE *out, const char *qry, size_t qry_from, size_t qry_to, const char *tgt, size_t tgt_from, size_t tgt_to, const char *cigar, size_t cigar_len);


#endif // ALIGN_H