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
 * The diagram below illustrates a quadtree with depth=2.
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
 *```
 * 
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
 * @brief Quadtree node opaque type.
 */
typedef struct _quadtree_node quadtree_node;

/**
 * @brief Returns the child of a node.
 */
quadtree_node *quadtree_node_get_chd(quadtree_node *node, quad_pos dir);


/**
 * @brief Gets the node data (payload).
 */
void *quadtree_node_get_data(quadtree_node *node);


/**
 * @brief Sets the node data (payload).
 */
void quadtree_node_set_data(quadtree_node *node, void *data);


/**
 * @brief Node update callback function type
 */
typedef void (*quadtree_node_upd_func)(quadtree_node *node, void *data);


/** 
 * @brief Node query callback function type
 */
typedef void (*quadtree_node_qry_func)(quadtree_node *node, void *dest);


/**
 * @brief Quadtree opaque type.
 */
typedef struct _quadtree quadtree;


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



typedef enum {
	SNAP_IN,
	SNAP_OUT
} snap_t;


rectangle rectangle_snap_to_grid(quadtree *self, rectangle rect, snap_t anchor);


quadtree *quadtree_new(uint width, uint height, uint depth);


void quadtree_finalise(void *ptr, const finaliser *fnr);


bool quadtree_ins(quadtree *self, point2d p, void *payload,
                  quadtree_node_upd_func upd_func);


void quadtree_qry(quadtree *self, rectangle area,
                  quadtree_node_qry_func qry_func, void *dest, bool backtrack);

#endif
