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

#ifndef  WAVTREE_H
#define  WAVTREE_H

#include <stddef.h>

#include "alphabet.h"

/**
 * @file wavtree.h
 * @author Paulo Fonseca, Israel Batista
 *
 * @brief Wavelet tree ADT.
 *
 * The Wavelet Tree is a succinct data structure to store strings in
 * compressed space. It generalizes the rank and select
 * operations defined on bitvectors to arbitrary alphabets.
 * (https://en.wikipedia.org/wiki/Wavelet_Tree)
 *
 * This library currently supports two WT shapes:
 * - Balanced: at each node the alphabet is split into halves leading
 *             to a balanced digital search tree
 * - Huffman: the tree is shaped after the Huffman code tree of the
 *            represented string.
 *
 */


/**
 * @brief Wavelet tree shape
 */
typedef enum {
	WT_BALANCED = 0, /**< Balanced, i.e. at each node the alphabet is split in halves. */
	WT_HUFFMAN = 1   /**< Shaped after a Huffman code tree */
}
wtshape;

typedef struct _wavtree wavtree;

/**
 * @brief Creates a wavelet tree representing a string @p str over an alphabet
 *        @p ab.
 * @param ab The base alphabet.
 * @param str The source string.
 * @param len The length of the indexed string.
 * @param shape The WT shape.
 */
wavtree *wavtree_new(alphabet *ab, char *str, size_t len, wtshape shape);


/**
 * @brief Creates a wavelet tree representing a string @p str over an alphabet
 *        @p ab.
 * @param ab The base alphabet.
 * @param str The source string.
 * @param len The length of the indexed string.
 * @param shape The WT shape.
 */
wavtree *wavtree_new_from_xstr(alphabet *ab, xstr *xstr, wtshape shape);


/**
 * @brief Creates a wavelet tree from a stream with known alphabet @p ab.
 * @param ab The base alphabet.
 * @param sst The source stream.
 * @param shape The WT shape.
 * @param layout The WT memory layout.
 */
wavtree *wavtree_new_from_stream(alphabet *ab, strstream *sst, wtshape shape);


/**
 * @brief Create a balanced wavelet tree from a stream with unknown alphabet.
 * @warn  Only CHAR_TYPE streams currently supported
 */
wavtree *wavtree_new_online( strstream *sst );


/**
 * @brief Destructor
 */
void wavtree_free(wavtree *wt);


/**
 * @brief Returns the lenght of the string represented by the WT.
 */
size_t wavtree_len(wavtree *wt);


/**
 * @brief Computes the rank of a given position @p pos, defined as
 *        rank[pos] = # of positions 0<=j<pos s.t. str[j]==str[pos], where
 *        str is string represented by the WT.
 *        @p pos must be in the range [0, length(str))
 */
size_t wavtree_rank_pos(wavtree *wt, size_t pos);


/**
 * @brief Computes the @pos c-rank of a given position @p pos, defined as
 *        the # of positions 0<=j<@p pos s.t. str[j]==@p c, where
 *        str is string represented by the WT.
 */
size_t wavtree_rank(wavtree *wt, size_t pos, xchar_t c);


/**
 * @brief Computes the position of the occurence of a char @p c with
 *        a given @p rank in the string represented by the WT.
 *        select(c,r) = j s.t. str[j]==c and rank[j]==r.
 *        If no such position exists, returns the length of the represented
 *        string.
 */
size_t wavtree_select(wavtree *wt, xchar_t c, size_t rank);


/**
 * @brief Computes the position of the rightmost @p c, strictly to the
 *        left of @p pos, i.e. max{j<pos | @p wt.char(j)==c}.
 *        If no such position exists, returns the length of the represented
 *        string.
 */
size_t wavtree_pred(wavtree *wt, size_t pos, xchar_t c);


/**
 * @brief Computes the position of the leftmost @p c, strictly to the
 *        right of @p pos, i.e. min{j>pos | @p wt.char(j)==c}.
 *        If no such position exists, returns the length of the  represented
 *        string.
 */
size_t wavtree_succ(wavtree *wt, size_t pos, xchar_t c);


/**
 * @brief Returns the char at position @p pos in the string represented by
 *        the WT. Notice that the WT does <b>not</b> explicitly store
 *        the string.
 */
xchar_t wavtree_char(wavtree *wt, size_t pos);


/**
 * @brief Prints a representation of the WT to standard output.
 */
void wavtree_print(wavtree *wt);

#endif
