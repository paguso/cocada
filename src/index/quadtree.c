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

#include "errlog.h"
#include "new.h"
#include "quadtree.h"

/*
Quadrant encoding

+----------------+----------------+
|                |                |
|    NW = 00     |     NE = 01    |
|                |                |
+----------------+----------------+
|                |                |
|    SW = 10     |     SE = 11    |
|                |                |
+----------------+----------------+
*/

#define IS_EAST(DIR) ( DIR & 0x01)
#define IS_WEST(DIR) ( 0x01 - IS_EAST(DIR) )
#define IS_SOUTH(DIR) ( DIR >> 1 )
#define IS_NORTH(DIR) ( 0x01 - IS_SOUTH(DIR) )


struct _quadtree_node {
	quad_dir dir;
	void *payload;
	quadtree_node *first_chd;
	quadtree_node *next_sibl;
};

#define IS_LEAF(NODE) ((NODE)->first_chd == NULL)

static quadtree_node *quadtree_node_new(quad_dir dir)
{
	quadtree_node *ret = NEW(quadtree_node);
	ret->dir = dir;
	ret->payload = NULL;
	ret->first_chd = NULL;
	ret->next_sibl = NULL;
	return ret;
}


quadtree_node *quadtree_node_get_chd(quadtree_node *node, quad_dir dir)
{
	quadtree_node *cur = node->first_chd;
	while (cur != NULL && cur->dir != dir) {
		cur = cur->next_sibl;
	}
	return cur;
}


void *quadtree_node_get_data(quadtree_node *node)
{
	return node->payload;
}


void quadtree_node_set_data(quadtree_node *node, void *data)
{
	node->payload = data;
}



struct _quadtree {
	quadtree_node *root;
	uint width;
	uint height;
	uint depth;
};


quadtree *quadtree_new(uint width, uint height, uint depth)
{
	quadtree *ret = NEW(quadtree);
	ret->width = width;
	ret->height = height;
	ret->depth = depth;
	ret->root = quadtree_node_new(NE);
	return ret;
}


static void destroy_tree(quadtree_node *root, const finaliser *fnr)
{
	for (quadtree_node *chd = root->first_chd; chd != NULL;) {
		quadtree_node *next = chd->next_sibl;
		destroy_tree(chd, fnr);
		chd = next;
	}
	FINALISE(&(root->payload), fnr);
	FREE(root);
}


void quadtree_finalise(void *ptr, const finaliser *fnr)
{
	quadtree *self = (quadtree *)ptr;
	const finaliser *node_fnr = (finaliser_nchd(fnr) > 0) ? finaliser_chd(fnr,
	                            0) : NULL;
	destroy_tree(self->root, node_fnr);
}



bool quadtree_ins(quadtree *self, point2d p, void *payload,
                  quadtree_node_upd_func upd_func)
{
	WARN_ASSERT(p.x < self->width,
	            "Quadtree insertion ignored (point.x=%u > width=%u)\n", p.x, self->width);
	WARN_ASSERT(p.y < self->height,
	            "Quadtree insertion ignored (point.y=%u > height=%u)\n", p.x, self->height);
	if (p.x > self->width || p.y > self->height) {
		return false;
	}
	quadtree_node *cur_node = self->root;
	upd_func(self->root, payload);
	rectangle rect = {.top_left.x = 0, .top_left.y = 0, .width = self->width, .height = self->height};
	point2d centre = {.x = rect.width / 2, .y = rect.height / 2};
	uint cur_depth = 1;
	while (cur_depth < self->depth && (rect.width > 1 || rect.height > 1)) {
		quad_dir dir = (((byte_t)(p.y >= centre.y)) << 1) | ((byte_t)(p.x >= centre.x));
		if (IS_EAST(dir)) {
			rect.top_left.x = centre.x;
			rect.width -= (rect.width / 2);
		}
		else {
			rect.width = (rect.width / 2);
		}
		if (IS_SOUTH(dir) ) {
			rect.top_left.y = centre.y;
			rect.height -= (rect.height / 2);
		}
		else {
			rect.height = (rect.height / 2);
		}
		centre.x = rect.top_left.x + (rect.width / 2);
		centre.y = rect.top_left.y + (rect.height / 2);
		if (cur_node->first_chd == NULL) {
			cur_node->first_chd = quadtree_node_new(dir);
			cur_node = cur_node->first_chd;
		}
		else if (dir < cur_node->first_chd->dir) {
			quadtree_node *new_first_chd = quadtree_node_new(dir);
			new_first_chd->next_sibl = cur_node->first_chd;
			cur_node->first_chd = new_first_chd;
			cur_node = new_first_chd;
		}
		else {
			quadtree_node *cur_sibl = cur_node->first_chd;
			while (cur_sibl->next_sibl != NULL && cur_sibl->next_sibl->dir < dir) {
				cur_sibl = cur_sibl->next_sibl;
			}
			if (cur_sibl->next_sibl != NULL && cur_sibl->next_sibl->dir == dir) {
				cur_node = cur_sibl->next_sibl;
			}
			else {
				quadtree_node *new_sibl = quadtree_node_new(dir);
				new_sibl->next_sibl = cur_sibl->next_sibl;
				cur_sibl->next_sibl = new_sibl;
				cur_node = new_sibl;
			}
		}
		upd_func(cur_node, payload);
		cur_depth++;
	}
	return true;
}
