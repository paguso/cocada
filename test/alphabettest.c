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

#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "alphabet.h"


void test_ab_new(CuTest *tc)
{
	size_t size=16;
	char *letters = "0123456789ABCDEF";
	alphabet *ab;
	ab = alphabet_new(size, letters);
	for (size_t i=0; i<size; i++) {
		size_t rk = ab_rank(ab, letters[i]);
		CuAssertSizeTEquals(tc, i, rk);
	}
	for (size_t i=size; i<2*size; i++) {
		size_t rk = ab_rank(ab, letters[i]);
		CuAssertSizeTEquals(tc, size, rk);
	}
	alphabet_free(ab);
}

/*
void test_ab_new_with_rank_map(CuTest *tc)
{
    size_t size=16;
    char *letters = "0123456789ABCDEF";
    alphabet *ab;
    ab = alphabet_new_with_rank_map(size, letters);
    for (size_t i=0; i<size; i++) {
        size_t rk = ab_rank(ab, letters[i]);
        CuAssertSizeTEquals(tc, i, rk);
    }
    for (size_t i=size; i<2*size; i++) {
        size_t rk = ab_rank(ab, letters[i]);
        CuAssertSizeTEquals(tc, size, rk);
    }
    alphabet_free(ab);
}
*/

extern size_t rankfn(char c)
{
	char *letters = "0123456789ABCDEF";
	size_t p;
	for (p=0; p<strlen(letters) && letters[p]!=c; p++);
	return p;
}

/*
void test_ab_new_with_rank_func(CuTest *tc)
{
    size_t size=16;
    char *letters = "0123456789ABCDEF";
    alphabet *ab;
    ab = alphabet_new_with_rank_func(size, letters, rankfn);
    for (size_t i=0; i<size; i++) {
        size_t rk = ab_rank(ab, letters[i]);
        CuAssertSizeTEquals(tc, i, rk);
    }
    for (size_t i=size; i<2*size; i++) {
        size_t rk = ab_rank(ab, letters[i]);
        CuAssertSizeTEquals(tc, size, rk);
    }
    alphabet_free(ab);
}
*/


/*
void test_ab_marshall(CuTest *tc)
{
    char *filename = "test_ab_marshall.bin";
    size_t size=16;
    char *letters = "0123456789ABCDEF";
    alphabet *abp, *abm, *abf;
    abp = alphabet_new(size, letters);
    abm = alphabet_new_with_rank_map(size, letters);
    REGISTER_FUNCTION(rankfn);
    abf = alphabet_new_with_rank_func(size, letters, rankfn);
    marshallctx *ctx = marshallctx_new(filename, MARSHALL);
    ab_marshall(ctx, abp);
    ab_marshall(ctx, abm);
    ab_marshall(ctx, abf);
    ab_marshall(ctx, abp);
    ab_marshall(ctx, abm);
    ab_marshall(ctx, abf);
    alphabet_free(abp);
    alphabet_free(abm);
    alphabet_free(abf);
    marshallctx_free(ctx);

    alphabet *abp2, *abm2, *abf2;
    ctx = marshallctx_new(filename, UNMARSHALL);
    abp = ab_unmarshall(ctx);
    abm = ab_unmarshall(ctx);
    abf = ab_unmarshall(ctx);
    abp2 = ab_unmarshall(ctx);
    abm2 = ab_unmarshall(ctx);
    abf2 = ab_unmarshall(ctx);
    marshallctx_free(ctx);

    CuAssertPtrEquals(tc, abp, abp2);
    CuAssertPtrEquals(tc, abf, abf2);
    CuAssertPtrEquals(tc, abm, abm2);

    for (size_t i=0; i<size; i++) {
        size_t rkp = ab_rank(abp, letters[i]);
        size_t rkm = ab_rank(abm, letters[i]);
        size_t rkf = ab_rank(abf, letters[i]);
        CuAssertSizeTEquals(tc, i, rkp);
        CuAssertSizeTEquals(tc, i, rkm);
        CuAssertSizeTEquals(tc, i, rkf);
    }
    for (size_t i=size; i<2*size; i++) {
        size_t rkp = ab_rank(abp, letters[i]);
        size_t rkm = ab_rank(abm, letters[i]);
        size_t rkf = ab_rank(abf, letters[i]);
        CuAssertSizeTEquals(tc, size, rkp);
        CuAssertSizeTEquals(tc, size, rkm);
        CuAssertSizeTEquals(tc, size, rkf);
    }
    alphabet_free(abp);
    alphabet_free(abm);
    alphabet_free(abf);
}

*/

CuSuite *alphabet_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_ab_new);
	//SUITE_ADD_TEST(suite, test_ab_new_with_rank_func);
	//SUITE_ADD_TEST(suite, test_ab_new_with_rank_map);
	//SUITE_ADD_TEST(suite, test_ab_marshall);
	return suite;
}