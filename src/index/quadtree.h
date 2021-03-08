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

typedef enum {
	NW,
	NE,
	SW,
	SE
} quad_dir;

typedef struct _quadtree_node quadtree_node;

quadtree_node *quadtree_node_get_chd(quadtree_node *node, quad_dir dir);

void *quadtree_node_get_data(quadtree_node *node);

void quadtree_node_set_data(quadtree_node *node, void *data);

typedef void (*quadtree_node_upd_func)(quadtree_node *node, void *data);



typedef struct _quadtree quadtree;

typedef struct {
	uint x;
	uint y;
} point2d;

typedef struct {
	point2d top_left;
	uint width;
	uint height;
} rectangle;

quadtree *quadtree_new(uint width, uint height, uint depth);

void quadtree_finalise(void *ptr, const finaliser *fnr);

bool quadtree_ins(quadtree *self, point2d p, void *payload,
                  quadtree_node_upd_func upd_func);


#endif
