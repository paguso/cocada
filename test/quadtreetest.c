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

#include "coretype.h"
#include "CuTest.h"
#include "memdbg.h"
#include "vec.h"
#include "quadtree.h"
#include "randutil.h"


void rectangle_snap_to_grid_test(CuTest *tc)
{
	uint w = 1024;
	uint h = 1024;
	uint d = 2;
	quadtree *tree = quadtree_new(w, h, d);
	rectangle r = {.top_left.x = 0, .top_left.y = 0, .width = 1024, .height = 1024};
	rectangle b = rectangle_snap_to_grid(tree, r, SNAP_OUT);
	CuAssertIntEquals(tc, r.top_left.x, b.top_left.x);
	CuAssertIntEquals(tc, r.top_left.y, b.top_left.y);
	CuAssertIntEquals(tc, w, b.width);
	CuAssertIntEquals(tc, h, b.height);
	b = rectangle_snap_to_grid(tree, r, SNAP_IN);
	CuAssertIntEquals(tc, r.top_left.x, b.top_left.x);
	CuAssertIntEquals(tc, r.top_left.y, b.top_left.y);
	CuAssertIntEquals(tc, w, b.width);
	CuAssertIntEquals(tc, h, b.height);
	r.width = w / 2;
	r.height = h / 2;
	b = rectangle_snap_to_grid(tree, r, SNAP_IN);
	CuAssertIntEquals(tc, r.top_left.x, b.top_left.x);
	CuAssertIntEquals(tc, r.top_left.y, b.top_left.y);
	CuAssertIntEquals(tc, w / 2, b.width);
	CuAssertIntEquals(tc, h / 2, b.height);
	b = rectangle_snap_to_grid(tree, r, SNAP_OUT);
	CuAssertIntEquals(tc, r.top_left.x, b.top_left.x);
	CuAssertIntEquals(tc, r.top_left.y, b.top_left.y);
	CuAssertIntEquals(tc, w / 2, b.width);
	CuAssertIntEquals(tc, h / 2, b.height);
	r.width = 300;
	r.height = 300;
	b = rectangle_snap_to_grid(tree, r, SNAP_IN);
	CuAssertIntEquals(tc, r.top_left.x, b.top_left.x);
	CuAssertIntEquals(tc, r.top_left.y, b.top_left.y);
	CuAssertIntEquals(tc, w / 4, b.width);
	CuAssertIntEquals(tc, h / 4, b.height);
	b = rectangle_snap_to_grid(tree, r, SNAP_OUT);
	CuAssertIntEquals(tc, r.top_left.x, b.top_left.x);
	CuAssertIntEquals(tc, r.top_left.y, b.top_left.y);
	CuAssertIntEquals(tc, w / 2, b.width);
	CuAssertIntEquals(tc, h / 2, b.height);
	r.top_left.x = 468;
	r.top_left.y = 468;
	r.width = 300;
	r.height = 300;
	b = rectangle_snap_to_grid(tree, r, SNAP_IN);
	CuAssertIntEquals(tc, 512, b.top_left.x);
	CuAssertIntEquals(tc, 512, b.top_left.y);
	CuAssertIntEquals(tc, 256, b.width);
	CuAssertIntEquals(tc, 256, b.height);
	b = rectangle_snap_to_grid(tree, r, SNAP_OUT);
	CuAssertIntEquals(tc, 256, b.top_left.x);
	CuAssertIntEquals(tc, 256, b.top_left.y);
	CuAssertIntEquals(tc, 512, b.width);
	CuAssertIntEquals(tc, 512, b.height);
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_new_empty()));
}


void upd_node_ins_pt(quadtree_node *node, void *pt)
{
	vec *pts = (vec *) quadtree_node_get_data(node);
	if (pts == NULL) {
		pts = vec_new(sizeof(point2d));
		quadtree_node_set_data(node, pts);
	}
	vec_push(pts, pt);
}


void qry_node_qty(quadtree_node *node, void *dest)
{
	*((size_t *)dest) += vec_len((vec *)quadtree_node_get_data(node));
}


void quadtree_ins_test(CuTest *tc)
{
	uint width = 1024;
	uint height = 1024;
	uint depth = 7;
	uint npts = 500;
	memdbg_reset();
	quadtree *tree = quadtree_new(width, height, depth);
	for (uint i=0; i<npts; i++) {
		point2d pt = {.x = rand_range_uint(0, width), .y = rand_range_uint(0, height) };
		quadtree_ins(tree, pt, &pt, upd_node_ins_pt);
	}
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_cons(finaliser_new_ptr(),
	                             FNR(vec))));
	memdbg_print_stats(stdout);
}

void quadtree_qry_test(CuTest *tc)
{
	uint width = 1024;
	uint height = 1024;
	uint depth = 10;
	memdbg_reset();
	quadtree *tree = quadtree_new(width, height, depth);
	for (uint x = 0; x < width; x++) {
		for (uint y = 0; y < height; y++) {
			point2d pt = {.x = x, .y = y };
			quadtree_ins(tree, pt, &pt, upd_node_ins_pt);
		}
	}
	for (uint x = 0; x < width; x++) {
		for (uint y = 0; y < height; y++) {
			rectangle rect = {.top_left.x = x, .top_left.y = y, .width = 1, .height = 1};
			size_t qty = 0;
			quadtree_qry(tree, rect, qry_node_qty, &qty, false);
			CuAssertSizeTEquals(tc, qty, 1);
		}
	}
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_cons(finaliser_new_ptr(),
	                             FNR(vec))));
	memdbg_print_stats(stdout);
}




CuSuite *quadtree_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, rectangle_snap_to_grid_test);
	//SUITE_ADD_TEST(suite, quadtree_ins_test);
	//SUITE_ADD_TEST(suite, quadtree_qry_test);
	return suite;
}