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


void upd_node_ins_pt(quadtree_node *node, void *pt)
{
	vec *pts = (vec *) quadtree_node_get_data(node);
	if (pts == NULL) {
		pts = vec_new(sizeof(point2d));
		quadtree_node_set_data(node, pts);
	}
	vec_push(pts, pt);
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




CuSuite *quadtree_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, quadtree_ins_test);
	return suite;
}