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

#include <string.h>

#include "CuTest.h"
#include "align.h"
#include "memdbg.h"
#include "strbuf.h"

void test_simple_global_align(CuTest *tc)
{

	memdbg_reset();

	char *qry = "";
	char *tgt = "aaa";
	strbuf *cigar = strbuf_new();
	int cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), cigar);
	CuAssertIntEquals(tc, 3, cost);
	CuAssertStrEquals(tc, "3I", strbuf_as_str(cigar));
	strbuf_clear(cigar);

    qry = "aaa";
    tgt = "";
    cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), cigar);
    CuAssertIntEquals(tc, 3, cost);
    CuAssertStrEquals(tc, "3D", strbuf_as_str(cigar));
    strbuf_clear(cigar);
    
    qry = "aaa";
    tgt = "bbb";
    cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), cigar);
    CuAssertIntEquals(tc, 3, cost);
    CuAssertStrEquals(tc, "3S", strbuf_as_str(cigar));
    strbuf_clear(cigar);

    qry = "aaa";
    tgt = "bbbaaa";
    cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), cigar);
    CuAssertIntEquals(tc, 3, cost);
    CuAssertStrEquals(tc, "3I3M", strbuf_as_str(cigar));
    strbuf_clear(cigar);
    
    qry = "aaabbb";
    tgt = "aaaa";
    cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), cigar);
    CuAssertIntEquals(tc, 3, cost);
    CuAssertStrEquals(tc, "3M2D1S", strbuf_as_str(cigar));
    strbuf_clear(cigar);

    
    

	strbuf_free(cigar);
	if (!memdbg_is_empty()) {
		memdbg_print_stats(stderr, true);
	}
	CuAssert(tc, "Memory leak detected", memdbg_is_empty());
}


CuSuite *align_get_test_suite(void)
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_simple_global_align);
	return suite;
}