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
	memdbg_reset();
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
	//memdbg_print_stats(stdout, true);
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_new_empty()));
	//memdbg_print_stats(stdout, true);
	CuAssert(tc, "memory leak", memdbg_is_empty());
}



void quadtree_do_nothing(quadtree *tree, size_t node, void *data)
{
}


void quadtree_ins_hollow_test(CuTest *tc)
{
	uint depth = 10;
	uint side = 1 << depth;
	memdbg_reset();
	quadtree *tree = quadtree_new(side, side, depth);
	size_t n = side * side;
	for (uint x = 0; x < side; x++) {
		for (uint y = 0; y < side; y++) {
			point2d p = {.x = x, .y = y};
			quadtree_ins(tree, p, NULL, quadtree_do_nothing);
		}
	}
	quadtree_fit(tree);
	//memdbg_print_stats(stdout, false);
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_new_empty()));
	//memdbg_print_stats(stdout, false);
	CuAssert(tc, "memory leak", memdbg_is_empty());
}


void upd_node_ins_pt(quadtree *tree, size_t node, void *pt)
{
	vec *pts = (vec *) quadtree_node_get_data(tree, node);
	if (pts == NULL) {
		pts = vec_new(sizeof(point2d));
		quadtree_node_set_data(tree, node, pts);
	}
	vec_push(pts, pt);
}


void qry_node_qty(quadtree *tree, size_t node, void *dest)
{
	vec *pts = (vec *)quadtree_node_get_data(tree, node); 
	if (pts != NULL) {
		*((size_t *)dest) += vec_len(pts);
	}
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
	//memdbg_print_stats(stdout, false);
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_cons(finaliser_new_ptr(),
	                             FNR(vec))));
	//memdbg_print_stats(stdout, false);
	CuAssert(tc, "memory leak", memdbg_is_empty());
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
	uint nqrys = 1024;
	for (uint i = 0; i < nqrys; i++) {
		uint x = rand_range_uint(0, width - 1);
		uint y = rand_range_uint(0, height - 1);
		uint w = rand_range_uint(0, width - x);
		uint h = rand_range_uint(0, height - y);
		rectangle rect = {.top_left.x = x, .top_left.y = y, .width = w, .height = h};
		size_t qty = 0;
		quadtree_qry(tree, rect, qry_node_qty, &qty, true);
		if (qty != (w * h)) {
			printf("erro\n");
		}
		CuAssertSizeTEquals(tc, (w * h), qty);
	}
	//memdbg_print_stats(stdout, false);
	DESTROY(tree, finaliser_cons(FNR(quadtree), finaliser_cons(finaliser_new_ptr(),
	                             FNR(vec))));
	CuAssert(tc, "memory leak", memdbg_is_empty());
}



CuSuite *quadtree_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, rectangle_snap_to_grid_test);
	SUITE_ADD_TEST(suite, quadtree_ins_hollow_test);
	SUITE_ADD_TEST(suite, quadtree_ins_test);
	SUITE_ADD_TEST(suite, quadtree_qry_test);
	return suite;
}