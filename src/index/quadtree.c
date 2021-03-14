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
#include "vec.h"

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

#define IS_EAST(QUAD) ( QUAD & 0x01)
#define IS_SOUTH(QUAD) ( QUAD >> 1 )


struct _quadtree {
	uint width;
	uint height;
	uint depth;
	vec *nodes;
};


typedef struct _quadtree_node {
	void *payload;
	size_t first_chd;
} quadtree_node;


static quadtree_node DUMMY_NODE = {.payload = NULL, .first_chd = 0};


static size_t quadtree_node_new(quadtree *tree)
{
	size_t new_node_pos = vec_len(tree->nodes);
	vec_push(tree->nodes, &DUMMY_NODE);
	vec_push(tree->nodes, &DUMMY_NODE);
	vec_push(tree->nodes, &DUMMY_NODE);
	vec_push(tree->nodes, &DUMMY_NODE);
	return new_node_pos;
}


bool quadtree_node_is_leaf(quadtree *tree, size_t node)
{
	return (((quadtree_node *) vec_get(tree->nodes, node))->first_chd) == 0;
}


size_t quadtree_node_get_chd(quadtree *tree, size_t node, quad_pos pos)
{
	size_t fch = ((quadtree_node *) vec_get(tree->nodes, node))->first_chd;
	return (fch == 0) ? fch : fch + pos;
}


static size_t quadtree_node_get_or_ins_chd(quadtree *tree, size_t node,
        quad_pos pos)
{
	size_t fchd = ((const quadtree_node *)vec_get(tree->nodes, node))->first_chd;
	if ( fchd == 0 ) {
		fchd = quadtree_node_new(tree);
	}
	((quadtree_node *)vec_get_mut(tree->nodes, node))->first_chd = fchd;
	return fchd + pos;
}


const void *quadtree_node_get_data(quadtree *tree, size_t node)
{
	return (const void *)(((const quadtree_node *)vec_get(tree->nodes,
	                       node))->payload);
}


void quadtree_node_set_data(quadtree *tree, size_t node, void *data)
{
	((quadtree_node *)vec_get_mut(tree->nodes, node))->payload = data;
}


#define ROOT 3

quadtree *quadtree_new(uint width, uint height, uint depth)
{
	quadtree *ret = NEW(quadtree);
	ret->width = width;
	ret->height = height;
	ret->depth = depth;
	ret->nodes = vec_new_with_capacity(sizeof(quadtree_node), 4);
	quadtree_node_new(ret);
	return ret;
}


void quadtree_fit(quadtree *tree)
{
	vec_fit(tree->nodes);
}



void quadtree_finalise(void *ptr, const finaliser *fnr)
{
	quadtree *self = (quadtree *)ptr;
	finaliser *vec_fnr = finaliser_cons(FNR(vec), finaliser_clone(finaliser_chd(fnr,
	                                    0)));
	DESTROY(self->nodes, vec_fnr);
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


void quadtree_ins(quadtree *tree, point2d p, void *payload,
                  quadtree_node_upd_func upd_func)
{
	WARN_ASSERT(p.x < tree->width,
	            "Quadtree insertion ignored (point.x=%u > width=%u)\n", p.x, tree->width);
	WARN_ASSERT(p.y < tree->height,
	            "Quadtree insertion ignored (point.y=%u > height=%u)\n", p.x, tree->height);
	if (p.x > tree->width || p.y > tree->height) {
		return;
	}
	size_t cur_node = ROOT;
	upd_func(tree, cur_node, payload);
	rectangle rect = {.top_left.x = 0, .top_left.y = 0, .width = tree->width, .height = tree->height};
	point2d centre = {.x = rect.width / 2, .y = rect.height / 2};
	uint cur_depth = 0;
	while (cur_depth < tree->depth && (rect.width > 1 || rect.height > 1)) {
		quad_pos pos = (((byte_t)(p.y >= centre.y)) << 1) | ((byte_t)(p.x >= centre.x));
		if (IS_EAST(pos)) {
			rect.top_left.x = centre.x;
			rect.width = SND_HALF(rect.width);
		}
		else {
			rect.width = FST_HALF(rect.width);
		}
		if (IS_SOUTH(pos) ) {
			rect.top_left.y = centre.y;
			rect.height = SND_HALF(rect.height);
		}
		else {
			rect.height = FST_HALF(rect.height);
		}
		centre.x = rect.top_left.x + FST_HALF(rect.width);
		centre.y = rect.top_left.y + FST_HALF(rect.height);
		cur_node = quadtree_node_get_or_ins_chd(tree, cur_node, pos);
		upd_func(tree, cur_node, payload);
		cur_depth++;
	}
}


rectangle rectangle_snap_to_grid(quadtree *tree, rectangle rect, snap_t anchor)
{
	point2d bounds[2]; // top_left, bot_right
	bounds[0] = rect.top_left;
	bounds[1].x = rect.top_left.x + rect.width;
	bounds[1].y = rect.top_left.y + rect.height;
	for (uint i=0; i<2; i++) {
		rectangle rect = {.top_left.x = 0, .top_left.y = 0, .width = tree->width, .height = tree->height};
		point2d centre = {.x = FST_HALF(rect.width), .y = FST_HALF(rect.height)};
		uint cur_depth = 0;
		while (cur_depth < tree->depth && (rect.width > 1 || rect.height > 1)) {
			quad_pos pos = (((byte_t)(bounds[i].y >= centre.y)) << 1) | 
							((byte_t)(bounds[i].x >= centre.x));
			if (IS_EAST(pos)) {
				rect.top_left.x = centre.x;
				rect.width = SND_HALF(rect.width);
			}
			else {
				rect.width = FST_HALF(rect.width);
			}
			if (IS_SOUTH(pos) ) {
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


static void quadtree_qry_node(quadtree *tree, size_t node,
                              rectangle search_area,
                              rectangle rect, quadtree_node_qry_func qry_func, void *dest, bool backtrack)
{
	if (node == 0) {
		return;
	}

	if (search_area.top_left.x  == rect.top_left.x
	        && search_area.top_left.y == rect.top_left.y
	        && search_area.width == rect.width
	        && search_area.height == rect.height) {
		qry_func(tree, node, dest);
		if (backtrack) {
			return;
		}
	}
	else if (quadtree_node_is_leaf(tree, node)) {
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
		quadtree_qry_node(tree, quadtree_node_get_chd(tree, node, NW), NW_area, NW_rect,
		                  qry_func, dest, backtrack );
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
		quadtree_qry_node(tree, quadtree_node_get_chd(tree, node, NE), NE_area, NE_rect,
		                  qry_func, dest, backtrack);
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
		quadtree_qry_node(tree, quadtree_node_get_chd(tree, node, SW), SW_area, SW_rect,
		                  qry_func, dest, backtrack);
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
		quadtree_qry_node(tree, quadtree_node_get_chd(tree, node, SE), SE_area, SE_rect,
		                  qry_func, dest, backtrack);
	}
}


void quadtree_qry(quadtree *tree, rectangle rect,
                  quadtree_node_qry_func qry_func, void *dest, bool backtrack)
{
	rectangle search_area = {.top_left.x = 0, .top_left.y = 0, .width = tree->width, .height = tree->height};
	quadtree_qry_node(tree, ROOT, search_area, rect, qry_func, dest, backtrack);

}