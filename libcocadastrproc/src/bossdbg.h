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

#ifndef DEBRUIJNGRAPH_H
#define DEBRUIJNGRAPH_H

#include "alphabet.h"
#include "strstream.h"
#include "xchar.h"

/**
 * @file bossdbg.h
 * @author Paulo Fonseca
 *
 * @brief String de Bruijn Graph (dBG) ADT.
 *
 * A de Bruijn Graph of order k is herein defined as a digraph
 * whose nodes correspond to distinct string k-mers and such that for every
 * pair of nodes u[0]..u[k-1], v[0]...v[k-1] s.t. u[1..k-1]=v[0..k-2]
 * there is an edge labeled u[k-1].
 */


/**
 * String dBG type.
 */
typedef struct _dbgraph dbgraph;


/**
 * @brief Creates a dBG from a source string. The string is padded to the
 *        left with @par k sentinel chars ($), and to the right with a
 *        finishing $. The sentinel is considered to be lexicographically
 *        smaller than all the other symbols of the input alphabet.
 * @param ab Input alphabet.
 * @param txt Input string. The noded will correspond to all the k-mers of
 *        this string after padding.
 * @param k A stictly positive order.
 */
dbgraph *bossbossdbg_new_from_str(alphabet *ab, char *txt, size_t k, bool multigraph);


/**
 * @brief Creates a dBG from a source stream.
 * @see bossbossdbg_new_from_string
 */
dbgraph *bossbossdbg_new_from_stream( alphabet *ab, strstream *sst, size_t k,
                              bool multigraph );


/**
 * @brief Destructor.
 */
void bossdbg_free(dbgraph *g);


/**
 * @brief Returns the sentinel character of a given deBruijn graph.
 */
char bossdbg_sentinel(dbgraph *g);


/**
 * @brief Returns the input alphabet
 */
alphabet *bossdbg_ab(dbgraph *g);


/**
 * @brief Returns the extended alphabet, including the sentinel
 * character of a given de Bruijn graph.
 */
alphabet *bossdbg_ext_ab(dbgraph *g);


/**
 * @brief Returns the number of distinct nodes of the dBG.
 */
size_t bossdbg_nnodes(dbgraph *g);


/**
 * @brief Returns the number of distinct edges of the dBG.
 */
size_t bossdbg_nedges(dbgraph *g);


/**
 * @brief Returns the order of the dBG.
 */
size_t bossdbg_k(dbgraph *g);


/**
 * @brief Returns whether the a node may have multiple outgoing edges with the
 *        same label.
 */
bool bossdbg_is_multigraph(dbgraph *g);


/**
 * @brief Every distinct node of the dBG has a unique integer id, which is
 *        normally used for navigating through the graph.
 *        This function returns the id of a node given its (also unique)
 *        rank in the range 0..nnodes-1.
 * @param nrk The node rank. If nrk>=nnodes, the result is undefined.
 * @see bossdbg_node_rank
 */
size_t bossdbg_node_id(dbgraph *g, size_t nrk);


/**
 * @brief The dBG has nnodes distinct nodes, and so every node
 *        has a rank in the range 0..nnodes-1.
 *        This function returns the rank of a node given its id.
 * @param nid The node id. If a nonexistant id is given, the result is
 *            undefined.
 * @see bossdbg_node_rank
 */
size_t bossdbg_node_rank(dbgraph *g, size_t nid);


/**
 * @brief Computes the label of the node into the string @p dest.
 * @param nid The node id. If a nonexistant id is given, the result is
 *            the empty string.
 */
void bossdbg_node_lbl(dbgraph *g, size_t nid, xstr *dest);


/**
 * @brief Returns the outdegree of a node given its id.
 *        If @p nid is invalid, the result is undefined.
 */
size_t bossdbg_outdeg(dbgraph *g, size_t nid);


/**
 * @brief Returns the number of outgoing edges of a node with a given
 *        edge label @p c.
 *        If @p nid is invalid, the result is undefined.
 */
size_t bossdbg_lbl_outdeg(dbgraph *g, size_t nid, xchar_t c);


/**
 * @brief Returns the child of the node with id @p nid, following an edge
 *        with label @p c.
 *        If @p nid is invalid, the result is undefined.
 *        If @p nid is valid but no such edge exists, an invalid node id is
 *        returned.
 * @param nid The id of the parent node.
 * @param c The outgoing edge label.
 */
size_t bossdbg_child(dbgraph *g, size_t nid, xchar_t c);


/**
 * @brief Returns the smallest node id corresponding to a node w with
 *        an outgoing edge pointing to the node with the given id.
 * @param nid The node id of the child node. If a nonexistant id is given,
 *        the result is undefined.
 */
size_t bossdbg_parent(dbgraph *g, size_t nid);


/**
 * @brief Prints the dBG @p g to std output.
 */
void  bossdbg_print(dbgraph *g);

#endif