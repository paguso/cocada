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

#ifndef QUADTREE_H
#define QUADTREE_H

#include "coretype.h"

/**
 * @file quadtree.h
 * @brief Quadtree
 * @author Paulo Fonseca
 *
 * A Quadtree is a 4-way tree representing an hierarchical partition
 * of a rectangle in four quadrants (top-left, top-right,
 * bottom-left, and bottom-right). The **depth** of the quadtree
 * corresponds to the number of subdivision levels. Depth=0 means
 * no subdivision, depth=1 means one subdivision, and so forth.
 * The diagram below illustrates a complete quadtree with depth=2.
 *
 * ```
 *  +-------+-------+-------+-------+
 *  |       |       |       |       |
 *  |   F   |   G   |   J   |   K   |
 *  |       |       |       |       |
 *  + ----- B ----- + ----- C ----- +
 *  |       |       |       |       |
 *  |   H   |   I   |   L   |   M   |
 *  |       |               |       |
 *  + ----- + ----  A  ---- + ----- +
 *  |       |               |       |
 *  |   N   |   O   |   R   |   S   |
 *  |       |       |       |       |
 *  + ----- D ----- + ----- E ----- +
 *  |       |       |       |       |
 *  |   P   |   Q   |   T   |   U   |
 *  |       |       |       |       |
 *  +-------+-------+-------+-------+
 *
 *
 *         __________  A  ___________
 *       /         __/   \__         \
 *      /         /         \         \
 *   _ B_      _ C_        _ D_      _ E_
 *  / /\ \    / /\ \      / /\ \    / /\ \
 * F G  H I  J K  L M    N O  P Q  R S  T U
 *
 * ```
 *
 * ## Discretisation
 *
 * This quadtree is disctretised. It represents a rectangle with
 * top-left point `(tl.x, tl.y) = (0,0)`, and given width `w` and
 * height `h`, both integers.
 * Thus, the x-coordinates of points inside the rectangle lie
 * in the range `0..w-1`, from left to right, whereas y-coordinates
 * lie in `0..h-1` from top to bottom. Subdivision in quadtrants
 * follow the scheme below.
 *
 *
 * ```
 *       0               w/2               w
 *       +----------------+----------------+
 *       |                |                |
 *       |    NW = 00     |     NE = 01    |
 *       |                |                |
 *   h/2 +--------------- C ---------------+
 *       |                |                |
 *       |    SW = 10     |     SE = 11    |
 *       |                |                |
 *     h +----------------+----------------+
 *
 *		C = (w/2, h/2)
 *
 * ```
 *
 * Subdivision occurs if the maximum depth has not been reached **and**
 * at least one dimension is greater than one. Notice that it may happen
 * that one dimension reaches 1 sooner than the other. However,
 * the diagram above remains valid. For instance, if `w=3`
 * and `h=1` we'd have
 *
 * ```
 *       0             w/2 = 1                              w=3
 *       +--------------- C ---------------------------------+
 *       |                |                                  |
 *       |                |                                  |
 *       |                |                                  |
 *       |    SW = 10     |            SE = 11               |
 *       |                |                                  |
 *       |                |                                  |
 *       |                |                                  |
 *   h=1 +----------------+----------------------------------+
 *
 *		C = (w/2 = 1, h/2 = 0)
 *
 * ```
 *
 * ## Node data (payload)
 *
 * Each node has associated data accessible via the public interface methods
 * ::quadtree_node_get_data and ::quadtree_node_set_data. Whether this
 * data is owned or not is left to the user. If the data is owned,
 * the finaliser should be defined accordingly with a nested finaliser
 * to a pointer to the owned type. For instance, if the data contains
 * a (pointer to) a vec:
 *
 * ```C
 * DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_cons(finaliser_new_ptr(), FNR(vec))));
 * ```
 */


/**
 * @brief Quadrant position type
 */
typedef enum {
	NW, /**< Northwest = top-left */
	NE, /**< Northeast = top-right */
	SW, /**< Southwest = bottom-left */
	SE  /**< Southeast = bottom-right */
} quad_pos;


/**
 * @brief Quadtree opaque type.
 */
typedef struct _quadtree quadtree;


/**
 * @brief Returns the child of a node.
 */
size_t quadtree_node_get_chd(quadtree *tree, size_t node, quad_pos pos);


/**
 * @brief Gets the node data (payload).
 */
const void *quadtree_node_get_data(quadtree *tree, size_t node);


/**
 * @brief Sets the node data (payload).
 */
void quadtree_node_set_data(quadtree *tree, size_t node, void *data);


/**
 * @brief Node update callback function type
 */
typedef void (*quadtree_node_upd_func)(quadtree *tree, size_t node, void *data);


/**
 * @brief Node query callback function type
 */
typedef void (*quadtree_node_qry_func)(quadtree *tree, size_t node, void *dest);


/**
 * @brief Discretised point type
 */
typedef struct {
	uint x;
	uint y;
} point2d;


/**
 * @brief Discretised rectangle type.
 */
typedef struct {
	point2d top_left;
	uint width;
	uint height;
} rectangle;


/**
 * @brief Anchor type to be used with the ::rectangle_snap_to_grid method.
 */
typedef enum {
	SNAP_IN, /**< Snap to inner rectangle **/
	SNAP_OUT /**< Snap to outer grid rectangle **/
} snap_t;


/**
 * @brief Snaps the rectangle to the grid implied by the quadtree.
 * If @p anchor is ::SNAP_OUT, returns the smallest rectangle with corners
 * in the grid containing @p rect. If @p anchor is ::SNAP_IN, returns
 * the largest rectangle with corners in the grid contained by @p rect.
 *
 * ## Example
 * Consider the diagram below. If this method is called with the
 * highligthed rectangle (*) and @p anchor = ::SNAP_IN, the
 * rectangle corresponding to `BJL` is returned, where `B=FGHI`.
 * If @p anchor = ::SNAP_OUT, then `BCNORS` area is returned.
 *
 * ```
 *  ****************************- - +
 *  *       .       .       .  *    .
 *  *   F   .   G   .   J   .  * K  .
 *  *       .       .       .  *    .
 *  * - - - B - - - + - - - C -*- - +
 *  *       .       .       .  *    .
 *  *   H   .   I   .   L   .  * M  .
 *  *       .               .  *    .
 *  * - - - + - -   A   - - + -*- - +
 *  *       .               .  *    .
 *  ****************************    .
 *  .   N   .   O   .   R   .   S   .
 *  + - - - D - - - + - - - E - - - +
 *  .       .       .       .       .
 *  .   P   .   Q   .   T   .   U   .
 *  .       .       .       .       .
 *  + - - - + - - - + - - - + - - - +
 * ```
 */
rectangle rectangle_snap_to_grid(quadtree *tree, rectangle rect, snap_t anchor);


/**
 * @brief Creates a new quadtree. The parameter @p depth specifies the
 * maximum depth. However this may not be achieved if rectangles with
 * side 1
 */
quadtree *quadtree_new(uint width, uint height, uint depth);


/**
 * @brief Finaliser.
 * If the node data is **not owned**, the provided
 * finaliser should be
 * ```C
 * finaliser_cons(FNR(quadtree), finaliser_new_empty()))
 * ```
 * If nodes contain owned references to (non-primitive) objects of type
 * ```C
 * finaliser_cons(FNR(quadtree), finaliser_cons(finaliser_new_ptr(), FNR(T)))
 * ```
 * Most likely this will be called via the ::DESTROY or ::FINALISE macros
 *
 * @see new.h
 */
void quadtree_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Adjusts internal representation trimming off unused memory
 */
void quadtree_fit(quadtree *tree);


/**
 * @brief Inserts a point @p p in the quadtree with associated @p payload data.
 * The algorithm follows a path from the root to the corresponding leaf
 * containing the point according to the subdivision process described in
 * the header documentation. The provided callback update function @p upd_func
 * is called on each node in this path from the root to the leaf. This may be used to
 * update the node data via the public interface methods
 * ::quadtree_node_get_data and ::quadtree_node_set_data.
 *
 * ## Example
 *
 * ```
 *  + - - - + - - - + - - - + - - - +
 *  .       .       .       .       .
 *  .   F   .   G   .   J   .   K   .
 *  .       .       .       .       .
 *  + - - - B - - - + - - - C - - - +
 *  .       .       .       .       .
 *  .   H   .   I   .   L   .   M   .
 *  .       .       .       .       .
 *  + - - - + - - - A - - - + - - - +
 *  .       .       .       .       .
 *  .   N   .   O   .   R   .   S   .
 *  .       .       .       .       .
 *  + - - - D - - - + - - - E - - - +
 *  .       .       .       .       .
 *  .   P   .   Q   .   T   .   U   .
 *  .       .       .  (*)  .       .
 *  + - - - + - - - + - ^ - + - - - +
 *                      |
 *                      |
 *    insert here -----´
 *
 *
 *  Nodes visited are indicated by an asterisk *:
 *
 *         __________  A* ___________
 *       /         __/   \__         \
 *      /         /         \         \
 *   _ B_      _ C_        _ D_      _E*_
 *  / /\ \    / /\ \      / /\ \    / /\ \
 * F G  H I  J K  L M    N O  P Q  R S T* U
 *
 * ```
 */
void quadtree_ins(quadtree *tree, point2d pt, void *payload,
                  quadtree_node_upd_func upd_func);


/**
 * @brief Queries a rectangular area of the quadtree. The algorithm
 * recursively decomposes the rectangle from top to bottom following
 * the quadree nodes, and collects information to update the query result
 * from all nodes completely contained in the selected area.
 *
 * ## Example
 * In the diagram below, the highlighted area
 * (inside the *) encompasses all the `B` top-left quadrant, plus
 * sub-quadrants `J` and `L`, and partially overlaps with `K`, `M`, `N`,
 * `O`, `R`, `S`.
 *
 * ```
 *  ****************************- - +
 *  *       .       .       .  *    .
 *  *   F   .   G   .   J   .  * K  .
 *  *       .       .       .  *    .
 *  * - - - B - - - + - - - C -*- - +
 *  *       .       .       .  *    .
 *  *   H   .   I   .   L   .  * M  .
 *  *       .               .  *    .
 *  * - - - + - -   A   - - + -*- - +
 *  *       .               .  *    .
 *  ****************************    .
 *  .   N   .   O   .   R   .   S   .
 *  + - - - D - - - + - - - E - - - +
 *  .       .       .       .       .
 *  .   P   .   Q   .   T   .   U   .
 *  .       .       .       .       .
 *  + - - - + - - - + - - - + - - - +
 *
 * Quadtree:
 *         __________  A  ___________
 *       /         __/   \__         \
 *      /         /         \         \
 *   _*B_      _ C_        _ D_      _ E_
 *  / /\ \    / /\ \      / /\ \    / /\ \
 * F G  H I  J K  L M    N O  P Q  R S  T U
 * * *  * *  *    *
 *
 * The subdivisions inside the selected area are indicated with
 * an asterisk (*).
 *
 * ```
 * The query algorithm recursively traverses the quadtree
 * from higher to lower levels (top-to-bottom)
 * following child quadrants in the NW, NE, SW, SW (left-to-right) order.
 * Each time a node corresponding to a region completely contained
 * in the queried area, the query update function @p qry_func is
 * called from that node to update the query result @p dest.
 * In the example, this would correspond to nodes `B` ,`F` ,`G` ,`H` ,
 * `I` ,`J` ,`L`, in that order. This exhaustive decomposition occurs when
 * the @p backtrack parameter is set to `false`.
 *
 * However, depending on the information associated with each node, this may
 * be unecessary. For example, when getting to node `B`, the whole region
 * corresponding to nodes `F`, `G`, `H`, `I` is covered. If `B` already
 * contains the information to update the query result at this level,
 * we can signal the algorithm to @p backtrack at this point by setting
 * this parameter to `true`.
 *
 *
 */
void quadtree_qry(quadtree *tree, rectangle area,
                  quadtree_node_qry_func qry_func, void *dest, bool backtrack);

#endif
