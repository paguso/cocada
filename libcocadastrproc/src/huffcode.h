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

#ifndef HUFFCODE_H
#define HUFFCODE_H

#include <stddef.h>
#include <stdio.h>

#include "alphabet.h"
#include "bitbyte.h"
#include "bitvec.h"
#include "new.h"
#include "strread.h"
#include "xstr.h"
#include "xstrread.h"

/**
 * @file huffcode.h
 * @author Paulo Fonseca
 *
 * @brief Huffman coding data structures and algorithms.
 *
 * A Huffman coding (HC) is a fixed-to-var prefix coding scheme that
 * unambiguously associates every char c of an alphabet to a binary
 * codeword h(c) such that if c!=c' then neither h(c) is a proper
 * prefix of h(c') nor the opposite.
 *
 * The HC is theoretically optimal in the information theoretic sense, meaning
 * that the average number of bits needed to encode a string T of length n
 * is n*H(T) where H(T) denotes the 0-order entropy of T.
 *
 * A HC over an alphabet A of length L is associated with a strictly binary
 * trie - The Huffman Tree (HT) - with L leaves, each corresponding to a
 * different character of A. The codeword of a char c is given by the label
 * of the path from the root of the HT to its corresponding leaf.
 */


/**
 * Huffman code type
 */
typedef struct _huffcode huffcode;


/**
 * Huffman Tree node type
 */
typedef struct _hufftnode hufftnode;


/**
 * @brief Creates a HC for an alphabet with associated letter frequencies.
 * @param ab (no transfer) The base alphabet.
 * @param freqs (no transfer) Individual letter frequencies in lexycographic order.
 */
huffcode *huffcode_new(const alphabet *ab, const size_t *freqs);


/**
 * @brief Creates a HC for an alphabet from a source string.
 * @param ab (no transfer) The base alphabet.
 * @param src (no transfer( Source string from which letter frequencies are to be estimated.
 */
huffcode *huffcode_new_from_str(const alphabet *ab, const char *src);


/**
 * @brief Creates a HC for an alphabet from a source stream.
 * @param (no transfer) ab The base alphabet.
 * @param (no transfer) src Source stream from which letter frequencies are to be estimated.
 */
huffcode *huffcode_new_from_strread(const alphabet *ab, strread *src);


/**
 * @brief Creates a HC for an alphabet from a source string.
 * @param ab (no transfer) The base alphabet.
 * @param src (no transfer( Source string from which letter frequencies are to be estimated.
 */
huffcode *huffcode_new_from_xstr(const alphabet *ab, const xstr *src);


/**
 * @brief Creates a HC for an alphabet from a source stream.
 * @param (no transfer) ab The base alphabet.
 * @param (no transfer) src Source stream from which letter frequencies are to be estimated.
 */
huffcode *huffcode_new_from_xstrread(const alphabet *ab, xstrread *src);



/**
 * @brief Destructor.
 */
void huffcode_free(huffcode *hcode);


/**
 * @brief Prints a string representation of the HC to the std output.
 */
void huffcode_print(FILE *stream, const huffcode *hcode);


/**
 * @brief Encodes a string @p src of length @p len.
 */
bitvec *huffcode_encode(const char *src, size_t len, const huffcode *code);


/**
 * @brief Encodes a string @p src of length @p len to a given destination.
 */
void huffcode_encode_to(bitvec *dest, const char *src, size_t len,
                        const huffcode *hcode);


/**
 * @brief Encodes a string @p src of length @p len.
 */
bitvec *huffcode_encode_xstr(const xstr *src, const huffcode *code);


/**
 * @brief Encodes a string @p src of length @p len to a given destination.
 */
void huffcode_encode_xstr_to(bitvec *dest, const xstr *src,
                             const huffcode *hcode);


/**
 * @brief Encodes a source stream into a binary code.
 */
bitvec *huffcode_encode_strread(strread *src, const huffcode *hcode);


/**
 * @brief Encodes a source stream to a given destination.
 */
void huffcode_encode_strread_to(bitvec *dest, strread *src,
                                const huffcode *hcode);


/**
 * @brief Encodes a source stream into a binary code.
 */
bitvec *huffcode_encode_xstrread(xstrread *src, const huffcode *hcode);


/**
 * @brief Encodes a source stream to a given destination.
 */
void huffcode_encode_xstrread_to(bitvec *dest, xstrread *src,
                                 const huffcode *hcode);


/**
 * @brief Decodes a binary code to a string.
 */
xstr *huffcode_decode(const bitvec *code, const huffcode *hcode);


/**
 * @brief Returns the code of a char
 * @param char_rank The rank of the char w.r.t. the code alphabet
 * @warn  Do NOT destroy of modify the returned bitvector.
 */
const bitvec *huffcode_charcode(const huffcode *hcode, size_t char_rank);


/**
 * @brief Returns the (root of the) Huffman tree corresponding to a given HC.
 * @warning Do NOT modify or destroy the returned value
 */
const hufftnode *huffcode_tree(const huffcode *hcode);


/**
 * @brief Returns the alphabet from a given HC.
 * @warning Do NOT modify or destroy the returned value
 */
const alphabet *huffcode_ab(const huffcode *code);


/**
 * @brief Checks whether a given HT node is a leaf.
 */
bool hufftnode_is_leaf(const hufftnode *node);


/**
 * @brief Returns the left child of a HT node.
 * @warning Do NOT modify or destroy the returned value
 */
const hufftnode *hufftnode_left(const hufftnode *node);


/**
 * @brief Returns the right child of a HT node.
 * @warning Do NOT modify or destroy the returned value
 */
const hufftnode *hufftnode_right(const hufftnode *node);


/**
 * @brief Returns the alphabet mask of a HT @p node. The alphabet mask of a
 *        @p node is a binary mask M[0..l-1] s.t. for each letter of the
 *        alphabet a[j], M[j] = 1 iff the leaf corresponding to letter a[j]
 *        is in the subtree rooted at this @p node.
 *
 * @warning Do NOT modify or destroy the returned value
 */
const byte_t *hufftnode_ab_mask(const hufftnode *node);


/**
 * @brief If the given HT @p node is a leaf, returns the lexicographic rank
 *        of the corresponding letter in the represented alphabet.
 *        If the given HT @p node is not a leaf, returns the alphabet size.
 */
size_t hufftnode_char_rank(const hufftnode *node);

#endif
