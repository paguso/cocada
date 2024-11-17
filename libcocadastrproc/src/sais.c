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

#include <stdlib.h>
#include <stdio.h>

#include "assert.h"
#include "alphabet.h"
#include "arrays.h"
#include "bitvec.h"
#include "coretype.h"
#include "csarray.h"
#include "cstrutil.h"
#include "errlog.h"
#include "strstream.h"


static const byte S = 0;
static const byte L = 1;


#define UNSET SIZE_MAX


static inline llong cstr_char_at(void *str, usize i)
{
	return (llong)(((char *)str)[i]);
}

static inline llong xstr_char_at(void *str, usize i)
{
	return (llong)xstr_get(str, i);
}

static inline llong usize_char_at(void *str, usize i)
{
	return (llong)(((usize *)str)[i]);
}

typedef llong (*CharAtFunc)(void *, usize);


void build_sarr( void *str, usize len, Alphabet *ab, usize *sarr,
                 bool add_sentinel, CharAtFunc char_at );


static void get_bkt_start( usize *bkts, usize *dest, usize l)
{
	for (usize i = 0, sum = 0; i < l; i++) {
		dest[i] = sum;
		sum += bkts[i];
	}
}


static void get_bkt_end( usize *bkts, usize *dest, usize l)
{
	for (usize i = 0, sum = 0; i < l; i++) {
		sum += bkts[i];
		dest[i] = sum;
	}
}


static void init_LS( void *str, usize len, Alphabet *ab,
                     BitVec *lsvec, BitVec *lmsvec, usize *bkts,
                     bool add_sentinel, CharAtFunc char_at)
{
	usize last = 0;
	byte ls, lastls;
	int cmp;
	for (usize i = 1; i < len; i++) {
		cmp = alphabet_cmp(ab, char_at(str, i - 1), char_at(str, i));
		if (cmp == 0) {
			continue;
		}
		ls = (cmp > 0);
		bitvec_push_n(lsvec, i - last, ls);
		bitvec_push(lmsvec, (last > 0 && ls == S && lastls == L));
		bitvec_push_n(lmsvec, (i - last - 1), 0);
		bkts[alphabet_rank(ab, char_at(str, i - 1)) + add_sentinel] += (i - last);
		last = i;
		lastls = ls;
	}
	if (add_sentinel) {
		// last run
		bitvec_push_n(lsvec, len - last, L);
		bitvec_push(lmsvec, (last > 0 && ls == S && lastls == L));
		bitvec_push_n(lmsvec, (len - last - 1), 0);
		bkts[alphabet_rank(ab, char_at(str, len - 1)) + add_sentinel] += (len - last);
	}
	// last run must be the sentinel
	bitvec_push(lsvec, S);
	bitvec_push(lmsvec, true);
	bkts[0] = 1;
}


static void induce_L( void *str, Alphabet *ab,
                      usize *sarr, BitVec *ls,
                      usize *bkts, usize *offsets,
                      bool add_sentinel, CharAtFunc char_at)
{
	usize l = alphabet_size(ab) + add_sentinel;
	get_bkt_start(bkts, offsets, l);
	usize n = bitvec_len(ls);
	for (usize i = 0, j; i < n; i++) {
		j = sarr[i];
		if ( j != UNSET && j > 0 && bitvec_get_bit(ls, j - 1) == L ) {
			j -= 1;
			sarr[offsets[alphabet_rank(ab, char_at(str, j)) + add_sentinel]++] = j;
		}
	}
}


static void induce_S( void *str, Alphabet *ab,
                      usize *sarr, BitVec *ls,
                      usize *bkts, usize *offsets,
                      bool add_sentinel, CharAtFunc char_at)
{
	usize l = alphabet_size(ab) + add_sentinel;
	get_bkt_end(bkts, offsets, l);
	usize n = bitvec_len(ls);
	for (usize i = n - 1, j; i > 0; i--) {
		j = sarr[i];
		if ( j != UNSET && j > 0 && bitvec_get_bit(ls, j - 1) == S ) {
			j -= 1;
			sarr[--offsets[alphabet_rank(ab, char_at(str, j)) + add_sentinel]] = j;
		}
	}
}


static void sort_LMS( void *str, Alphabet *ab,
                      usize *sarr, BitVec *ls, BitVec *lms,
                      usize *bkts, usize *offsets,
                      bool add_sentinel, CharAtFunc char_at)
{
	//assert(char_at != usize_char_at || add_sentinel == false);
	usize n = bitvec_len(ls);
	usize l = alphabet_size(ab) + add_sentinel;

	// 1. Sort LMS segments
	get_bkt_end(bkts, offsets, l);
	sarr[0] = n - 1;
	for (usize i = 0; i < n - 1; i++) {
		if (!bitvec_get_bit(lms, i)) {
			continue;
		}
		sarr[--offsets[alphabet_rank(ab, char_at(str, i)) + add_sentinel]] = i;
	}
	//printf("1)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");
	induce_L(str, ab, sarr, ls, bkts, offsets, add_sentinel, char_at);
	//printf("2)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");
	induce_S(str, ab, sarr, ls, bkts, offsets, add_sentinel, char_at);
	//printf("3)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");

	// 2. Reduce the problem
	// 2.1 move sorted LMS segments to 1st half of the SA
	usize nlms = 0;
	for (usize i = 0; i < n; i++) {
		if (bitvec_get_bit(lms, sarr[i])) {
			sarr[nlms++] = sarr[i];
		}
	}
	ARR_FILL(sarr, nlms, n, UNSET);
	//printf("4)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");

	// 2.2 compute the # of different LMS segments
	//     and rename each of them as different macro int character
	usize ndifflms = 0;
	usize cur_lms = 0, prev_lms = 0, pos = 0;
	bool diff = false;

	//printf("ls=");
	//bitvec_print(stdout, ls, 4);
	//printf("lms=");
	//bitvec_print(stdout, lms, 4);

	for (usize i = 0; i < nlms; i++) {
		diff = false;
		cur_lms = sarr[i];
		for ( usize k = 0; ; k++ ) {
			//printf("comparing positions prev_lms+k=%zu to cur_lms+k=%zu \n", prev_lms+k, cur_lms+k);
			if (i == 0 ||
			        prev_lms + k + add_sentinel == n || cur_lms + k + add_sentinel == n ||
			        char_at(str, prev_lms + k) != char_at(str, cur_lms + k) ||
			        bitvec_get_bit(ls, prev_lms + k) != bitvec_get_bit(ls, cur_lms + k)) {
				diff = true;
				//printf("break diff=true >\n");
				break;
			}
			else if (k > 0 && ( bitvec_get_bit(lms, prev_lms + k)
			                    || bitvec_get_bit(lms, cur_lms + k) )) {
				break;
				//printf("break diff==false>>\n");
			}
			//printf("prev_lms=%zu cur_lms=%zu k=%zu diff=%d\n", prev_lms, cur_lms, k, diff);
		}
		ndifflms += diff; // if(diff) ndifflms++;

		// since more than half of the SA is free
		// (each LMS requires a preceding L)
		// use the second half to store the macro characters
		// initially at scatered positions
		pos = ((cur_lms % 2) == 0) ? (cur_lms / 2) : ((cur_lms - 1) / 2);
		sarr[nlms + pos] = ndifflms - 1;
		prev_lms = cur_lms;
	}

	//printf("5)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");
	// and finally push all the macro chars towards the end of the SA
	for (usize i = n - 1, j = n - 1; i >= nlms; i--)
		if (sarr[i] != UNSET) {
			sarr[j] = sarr[i];
			if (i != j) {
				sarr[i] = UNSET;
			}
			j--;
		}

	//printf("6)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");

	// 3. use the reduced string to sort the LMS suffixes
	usize *red_sarr = sarr;
	usize *red_str  = sarr + n - nlms;

	// 3.1 if all (sorted) macro chars are distinct,
	//     the LMS suffixes are already sorted
	if (nlms == ndifflms) {
		for (usize i = 0; i < nlms; i++) {
			red_sarr[red_str[i]] = i;
		}
	}
	// 3.2 otherwise, solve the reduced problem
	else {
		//printf("reducing\n");
		//ARR_PRINT(red_str, red_str, %zu, 0, nlms, 10, "");
		Alphabet *red_ab = alphabet_new_int_ab(ndifflms);
		build_sarr(red_str, nlms, red_ab, red_sarr, 0, usize_char_at);
		alphabet_free(red_ab);
	}
	// now the first positions of sarr correspond to the number of
	// LMS suffixes (not their original positions) in lexicographic order
	// (sarr[0] = ndifflms-1)

	// printf("7)\n");
	// ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");

	// 4. finally put the starting positions of the sorted LMS suffixes
	//    at their correct place in the SA
	get_bkt_end(bkts, offsets, l);
	for (usize i = 0, j = 0; i < n; i++) {
		if (bitvec_get_bit(lms, i)) {
			red_str[j++] = i;
		}
	}

	//printf("8)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");
	for (usize i = 0; i < nlms; i++) {
		red_sarr[i] = red_str[red_sarr[i]];
	}
	ARR_FILL(sarr, nlms, n, UNSET);
	//printf("9)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");
	for (usize i = nlms - 1, j; i > 0; i--) {
		j = sarr[i];
		sarr[i] = UNSET;
		sarr[--offsets[alphabet_rank(ab, char_at(str, j)) + add_sentinel]] = j;
	}
	// Done. All LMS suffixes are sorted and correctly placed in the SA
	//printf("10)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, n, 10, "");
}

/*
 * The alphabet MUST ALREADY contain the sentinel
 * If add_sentinel is true, a sentinel char is virtually appended to the string.
 * If add_sentinel is false, the smallest char in the alphabet must occur
 * exactly once at the end of the string.
 */
void build_sarr( void *str, usize len, Alphabet *ab, usize *sarr,
                 bool add_sentinel, CharAtFunc char_at)
{
	usize ab_sz = alphabet_size(ab);

	BitVec *ls   = bitvec_new_with_capacity(len + add_sentinel);
	BitVec *lms  = bitvec_new_with_capacity(len + add_sentinel);
	usize *bkts = ARR_OF_0_NEW(usize, ab_sz + add_sentinel);

	init_LS(str, len, ab, ls, lms, bkts, add_sentinel, char_at);

	// printf("-------------------------------------------------------------\n");
	//printf("build_sarr\n");
	//printf("ls\n");
	//bitvec_print(stdout, ls, 4);
	//printf("lms\n");
	//bitvec_print(stdout, lms, 4);
	//ARR_PRINT(bkts, bkts, %zu, 0, ab_sz + add_sentinel, 10, "");

	ARR_FILL(sarr, 0, len + 1, UNSET);
	usize *offsets = ARR_NEW(usize, ab_sz + add_sentinel);

	sort_LMS(str, ab, sarr, ls, lms, bkts, offsets, add_sentinel, char_at);
	//printf("a)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, len+1, 10, "");
	induce_L(str, ab, sarr, ls, bkts, offsets, add_sentinel, char_at);
	//printf("b)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, len+1, 10, "");
	induce_S(str, ab, sarr, ls, bkts, offsets, add_sentinel, char_at);
	//printf("c)\n");
	//ARR_PRINT(sarr, SA, %zu, 0, len+1, 10, "");

	// clean up
	bitvec_free(ls);
	bitvec_free(lms);
	FREE(bkts);
	FREE(offsets);
}


usize *sais(char *str, usize len, Alphabet *ab)
{
	ERROR_ASSERT(alphabet_type(ab) == CHAR_TYPE,
	             "Incompatible alphabet type for SA-IS");
	if (len < 2) {
		usize *sarr = ARR_NEW(usize, len + 1);
		for (usize i = 0; i <= len; i++) {
			sarr[i] = len - i;
		}
		return sarr;
	}
	usize *sarr = ARR_NEW(usize, len + 1);
	build_sarr(str, len, ab, sarr, true, cstr_char_at);
	return sarr;
}


usize *sais_xstr(xstr *str, Alphabet *ab)
{
	usize len = xstr_len(str);
	if (len < 2) {
		usize *sarr = ARR_NEW(usize, len + 1);
		for (usize i = 0; i <= len; i++) {
			sarr[i] = len - i;
		}
		return sarr;
	}
	usize *sarr = ARR_NEW(usize, len + 1);
	build_sarr(str, len, ab, sarr, true, xstr_char_at);
	return sarr;
}
