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
#include <assert.h>

#include "errlog.h"
#include "mathutil.h"
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
	quad_pos dir;
	void *payload;
	quadtree_node *first_chd;
	quadtree_node *next_sibl;
};

#define IS_LEAF(NODE) ((NODE)->first_chd == NULL)

static quadtree_node *quadtree_node_new(quad_pos dir)
{
	quadtree_node *ret = NEW(quadtree_node);
	ret->dir = dir;
	ret->payload = NULL;
	ret->first_chd = NULL;
	ret->next_sibl = NULL;
	return ret;
}


quadtree_node *quadtree_node_get_chd(quadtree_node *node, quad_pos dir)
{
	quadtree_node *cur = node->first_chd;
	while (cur != NULL && cur->dir != dir) {
		cur = cur->next_sibl;
	}
	return cur;
}


static quadtree_node *quadtree_node_get_or_ins_chd(quadtree_node *node,
        quad_pos dir)
{
	quadtree_node *cur = node->first_chd;
	quadtree_node *prev = NULL;
	while (cur != NULL && cur->dir < dir) {
		prev = cur;
		cur = cur->next_sibl;
	}
	if (cur == NULL || dir < cur->dir) {
		if (prev == NULL) { // cur == node->first_chd
			quadtree_node *new_chd = quadtree_node_new(dir);
			new_chd->next_sibl = node->first_chd;
			node->first_chd = new_chd;
			return new_chd;
		}
		else {
			quadtree_node *new_chd = quadtree_node_new(dir);
			new_chd->next_sibl = cur;
			prev->next_sibl = new_chd;
			return new_chd;
		}
	}
	else {
		assert(cur->dir == dir);
		return cur;
	}
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


#define FST_HALF(LEN) ((LEN) / 2)

#define SND_HALF(LEN) ((LEN) - ((LEN) / 2))

static const rectangle EMPTY_REC = {.top_left.x = 0, .top_left.y = 0, .width = 0, .height = 0 };

static rectangle rectangle_clip(rectangle rect, rectangle viewport)
{
	if (rect.top_left.x + rect.width < viewport.top_left.x ||
	        rect.top_left.x >= viewport.top_left.x + viewport.width ||
	        rect.top_left.y + rect.height < viewport.top_left.y ||
	        rect.top_left.y >= viewport.top_left.y + viewport.height) {
		return EMPTY_REC;
	}
	rectangle clr;
	clr.top_left.x = MAX(rect.top_left.x, viewport.top_left.x);
	clr.top_left.y = MAX(rect.top_left.y, viewport.top_left.y);
	clr.width = MIN(rect.top_left.x + rect.width,
	                viewport.top_left.x + viewport.width) - clr.top_left.x;
	clr.height = MIN(rect.top_left.y + rect.height,
	                 viewport.top_left.y + viewport.height) - clr.top_left.y;
	return clr;
}


void quadtree_ins(quadtree *self, point2d p, void *payload,
                  quadtree_node_upd_func upd_func)
{
	WARN_ASSERT(p.x < self->width,
	            "Quadtree insertion ignored (point.x=%u > width=%u)\n", p.x, self->width);
	WARN_ASSERT(p.y < self->height,
	            "Quadtree insertion ignored (point.y=%u > height=%u)\n", p.x, self->height);
	if (p.x > self->width || p.y > self->height) {
		return;
	}
	quadtree_node *cur_node = self->root;
	upd_func(self->root, payload);
	rectangle rect = {.top_left.x = 0, .top_left.y = 0, .width = self->width, .height = self->height};
	point2d centre = {.x = rect.width / 2, .y = rect.height / 2};
	uint cur_depth = 0;
	while (cur_depth < self->depth && (rect.width > 1 || rect.height > 1)) {
		quad_pos dir = (((byte_t)(p.y >= centre.y)) << 1) | ((byte_t)(p.x >= centre.x));
		if (IS_EAST(dir)) {
			rect.top_left.x = centre.x;
			rect.width = SND_HALF(rect.width);
		}
		else {
			rect.width = FST_HALF(rect.width);
		}
		if (IS_SOUTH(dir) ) {
			rect.top_left.y = centre.y;
			rect.height = SND_HALF(rect.height);
		}
		else {
			rect.height = FST_HALF(rect.height);
		}
		centre.x = rect.top_left.x + (rect.width / 2);
		centre.y = rect.top_left.y + (rect.height / 2);
		cur_node = quadtree_node_get_or_ins_chd(cur_node, dir);
		upd_func(cur_node, payload);
		cur_depth++;
	}
}


rectangle rectangle_snap_to_grid(quadtree *self, rectangle rect, snap_t anchor)
{
	point2d bounds[2]; // top_left, bot_right
	bounds[0] = rect.top_left;
	bounds[1].x = rect.top_left.x + rect.width;
	bounds[1].y = rect.top_left.y + rect.height;
	for (uint i=0; i<2; i++) {
		rectangle rect = {.top_left.x = 0, .top_left.y = 0, .width = self->width, .height = self->height};
		point2d centre = {.x = FST_HALF(rect.width), .y = FST_HALF(rect.height)};
		uint cur_depth = 0;
		while (cur_depth < self->depth && (rect.width > 1 || rect.height > 1)) {
			quad_pos dir = (((byte_t)(bounds[i].y >= centre.y)) << 1) | ((byte_t)(
			                   bounds[i].x >= centre.x));
			if (IS_EAST(dir)) {
				rect.top_left.x = centre.x;
				rect.width = SND_HALF(rect.width);
			}
			else {
				rect.width = FST_HALF(rect.width);
			}
			if (IS_SOUTH(dir) ) {
				rect.top_left.y = centre.y;
				rect.height = SND_HALF(rect.height);
			}
			else {
				rect.height = FST_HALF(rect.height);
			}
			centre.x = rect.top_left.x + FST_HALF(rect.width);
			centre.y = rect.top_left.y + FST_HALF(rect.height);
			cur_depth++;
		}
		point2d rectbounds[2];
		rectbounds[0] = rect.top_left;
		rectbounds[1].x = rect.top_left.x + rect.width;
		rectbounds[1].y = rect.top_left.x + rect.height;
		if (bounds[i].x == rectbounds[0].x && bounds[i].y == rectbounds[0].y) {
			bounds[i] = rectbounds[0];
		}
		else if (bounds[i].x == rectbounds[1].x && bounds[i].y == rectbounds[1].y) {
			bounds[i] = rectbounds[1];
		}
		else {
			switch (anchor) {
			case SNAP_IN:
				bounds[i] = rectbounds[1-i];
				break;
			case SNAP_OUT:
				bounds[i] = rectbounds[i];
				break;
			}
		}
	}
	rectangle ret = {.top_left = bounds[0], .width = bounds[1].x - bounds[0].x, .height = bounds[1].y - bounds[0].y};
	return ret;
}


static void quadtree_qry_node(quadtree_node *root, rectangle search_area,
                              rectangle rect, quadtree_node_qry_func qry_func, void *dest, bool backtrack)
{
	if (root == NULL) {
		return;
	}

	if (search_area.top_left.x  == rect.top_left.x
	        && search_area.top_left.y == rect.top_left.y
	        && search_area.width == rect.width
	        && search_area.height == rect.height) {
		qry_func(root, dest);
		if (backtrack) {
			return;
		}
	}
	else if (root->first_chd == NULL) {  // LEAF NODE
		return;
	}

	point2d centre = {.x = search_area.top_left.x + FST_HALF(search_area.width),
	                  .y = search_area.top_left.y + FST_HALF(search_area.height)
	                 };

	if (rect.top_left.x < centre.x && rect.top_left.y < centre.y) {
		rectangle NW_area = {
			.top_left = search_area.top_left,
			.width = FST_HALF(search_area.width),
			.height = FST_HALF(search_area.height)
		};
		rectangle NW_rect = {
			.top_left = rect.top_left,
			.width = MIN(rect.width, centre.x - rect.top_left.x),
			.height = MIN(rect.height,  centre.y - rect.top_left.y)
		};
		quadtree_qry_node(quadtree_node_get_chd(root, NW), NW_area, NW_rect, qry_func,
		                  dest, backtrack );
	}
	if (rect.top_left.x + rect.width >= centre.x
	        && rect.top_left.y < centre.y) {
		rectangle NE_area = {
			.top_left.x = centre.x,
			.top_left.y = search_area.top_left.y,
			.width = SND_HALF(search_area.width),
			.height = FST_HALF(search_area.height)
		};
		rectangle NE_rect = {
			.top_left.x = MAX(rect.top_left.x, centre.x),
			.top_left.y = rect.top_left.y,
			.width = MIN(rect.width, rect.top_left.x + rect.width - centre.x),
			.height = MIN(rect.height, centre.y - rect.top_left.y)
		};
		quadtree_qry_node(quadtree_node_get_chd(root, NE), NE_area, NE_rect, qry_func,
		                  dest, backtrack);
	}
	if (rect.top_left.x < centre.x
	        && rect.top_left.y + rect.height >= centre.y) {
		rectangle SW_area = {
			.top_left.x = search_area.top_left.x,
			.top_left.y = centre.y,
			.width = FST_HALF(search_area.width),
			.height = SND_HALF(search_area.height)
		};
		rectangle SW_rect = {
			.top_left.x = rect.top_left.x,
			.top_left.y = MAX(rect.top_left.y, centre.y),
			.width = MIN(rect.width, centre.x - rect.top_left.x),
			.height = MIN(rect.height, rect.top_left.y + rect.height - centre.y)
		};
		quadtree_qry_node(quadtree_node_get_chd(root, SW), SW_area, SW_rect, qry_func,
		                  dest, backtrack);
	}
	if (rect.top_left.x + rect.width >= centre.x
	        && rect.top_left.y + rect.height >= centre.y) {
		rectangle SE_area = {
			.top_left = centre,
			.width = SND_HALF(search_area.width),
			.height = SND_HALF(search_area.height)
		};
		rectangle SE_rect = {
			.top_left.x = MAX(rect.top_left.x, centre.x),
			.top_left.y = MAX(rect.top_left.y, centre.y),
			.width = MIN(rect.width, rect.top_left.x + rect.width - centre.x),
			.height = MIN(rect.height, rect.top_left.y + rect.height - centre.y)
		};
		quadtree_qry_node(quadtree_node_get_chd(root, SE), SE_area, SE_rect, qry_func,
		                  dest, backtrack);
	}


}


void quadtree_qry(quadtree *self, rectangle rect,
                  quadtree_node_qry_func qry_func, void *dest, bool backtrack)
{
	rectangle search_area = {.top_left.x = 0, .top_left.y = 0, .width = self->width, .height = self->height};
	quadtree_qry_node(self->root, search_area, rect, qry_func, dest, backtrack);

}