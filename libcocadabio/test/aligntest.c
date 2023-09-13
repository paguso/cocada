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
#include "cstrutil.h"
#include "align.h"
#include "errlog.h"
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


char *random_str(size_t n) {
    char *ret = cstr_new(n);
    for (size_t i = 0; i < n; i++) {
        ret[i] = 'a' + rand()%10;
    }
    return ret;
}

void test_gotoh(CuTest *tc) {
    memdbg_reset();

    char *qry = "aaabbbcccddd";
    char *tgt = "aabbbxxcccyyddd";
    strbuf *cigar = strbuf_new();
    int simple_cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), cigar);
    int cost = gotoh(qry, strlen(qry), tgt, strlen(tgt), 0, 1, subst_uniform_cost, cigar);
    CuAssertIntEquals(tc, simple_cost, cost);

    for (int gap_open = 0; gap_open < 2; gap_open++) {
        for (int gap_ext = 1; gap_ext < 3; gap_ext++) {
            for (size_t m = 0; m < 20; m++) {
                for (size_t n = 0; n < 20; n++) {
                    qry = random_str(m);
                    tgt = random_str(n);
                    cost = gotoh(qry, strlen(qry), tgt, strlen(tgt), 0, 1, subst_uniform_cost, cigar);
                    DEBUG("gotoh(%s,%s,%d,%d)=%d", qry, tgt, gap_open, gap_ext, cost);
                    free(qry);
                    free(tgt);
                }
            }
        }
    }
   
    strbuf_free(cigar);

    CuAssert(tc, "Memory leak detected", memdbg_is_empty());
}


void test_align(CuTest *tc) {
    memdbg_reset();

    char *qry = "aaaaccccccccbbbbaaaaccccccccbbbb";
    char *tgt = "aaaabbbbaaaabbbb";
    strbuf *cigar = strbuf_new();

    int gap_open = 1;
    int gap_ext = 1;
    int align_cost = align(qry, strlen(qry), tgt, strlen(tgt), gap_open, gap_ext, subst_uniform_cost, cigar);
    int gotoh_cost = gotoh(qry, strlen(qry), tgt, strlen(tgt), gap_open, gap_ext, subst_uniform_cost, cigar);
    CuAssertIntEquals(tc, align_cost, gotoh_cost);

    //int cost = simple_global_align(qry, strlen(qry), tgt, strlen(tgt), gap_open, gap_ext, subst_uniform_cost, cigar);

    //CuAssertIntEquals(tc, gotoh_cost, align_cost);
    for (int gap_open = 0; gap_open < 2; gap_open++) {
        for (int gap_ext = 1; gap_ext < 3; gap_ext++) {
            for (size_t m = 0; m < 20; m++) {
                for (size_t n = 0; n < 20; n++) {
                    qry = random_str(m);
                    tgt = random_str(n);
                    int align_cost = align(qry, strlen(qry), tgt, strlen(tgt), gap_open, gap_ext, subst_uniform_cost, cigar);
                    int gotoh_cost = gotoh(qry, strlen(qry), tgt, strlen(tgt), gap_open, gap_ext, subst_uniform_cost, cigar);
                    CuAssertIntEquals(tc, align_cost, gotoh_cost);
                    free(qry);
                    free(tgt);
                }
            }
        }
    }
   

    strbuf_free(cigar);
    CuAssert(tc, "Memory leak!", memdbg_is_empty());
}

CuSuite *align_get_test_suite(void)
{
	CuSuite *suite = CuSuiteNew();
	//SUITE_ADD_TEST(suite, test_simple_global_align);
    //SUITE_ADD_TEST(suite, test_gotoh);
    SUITE_ADD_TEST(suite, test_align);
	return suite;
}