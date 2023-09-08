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

#include "align.h"
#include "arrays.h"
#include "coretype.h"
#include "mathutil.h"
#include "strbuf.h"


int subst_uniform_cost(char a, char b)
{
    return (a == b) ? 0 : 1;
}


void compress_cigar(strbuf *cigar)
{
	size_t n = strbuf_len(cigar);
	if (n == 0) return;
	size_t i = 0;
	char last_c = strbuf_get(cigar, i++);
	int run_len = 1;
	while (i < n) {
		char c = strbuf_get(cigar, i++);
		if (c == last_c) {
			run_len++;
		}
		else {
			sbprintf(cigar, "%d%c", run_len, last_c);
			last_c = c;
			run_len = 1;
		}
	}
	sbprintf(cigar, "%d%c", run_len, last_c);
	strbuf_clip(cigar, n, strbuf_len(cigar));
}


int simple_global_align(const char *qry, size_t qry_len, const char *tgt,
                        size_t tgt_len, strbuf *cigar)
{
	int ret = 0;
	NEW_MATRIX(C, int, qry_len+1, tgt_len+1);
	for (size_t i = 0; i <= qry_len; i++) {
		C[i][0] = i;
	}
	for (size_t j = 0; j <= tgt_len; j++) {
		C[0][j] = j;
	}
	for (size_t i = 1; i <= qry_len; i++) {
		for (size_t j = 1; j <= tgt_len; j++) {
			int match = (qry[i - 1] == tgt[j - 1]) ? 0 : 1;
			C[i][j] = MIN3(C[i-1][j-1] + match, C[i-1][j] + 1, C[i][j-1] + 1);
		}
	}
	ret = C[qry_len][tgt_len];
	// recover the cigar string
	if (cigar) {
		strbuf *aln = strbuf_new();
		size_t i = qry_len, j = tgt_len;
		while (i || j) {
			if (j == 0) {
				strbuf_append_char(aln, 'D');
				i--;
			}
			else if (i == 0) {
				strbuf_append_char(aln, 'I');
				j--;
			}
			else if (C[i][j] == C[i-1][j-1] + (qry[i - 1] == tgt[j - 1] ? 0 : 1)) {
				strbuf_append_char(aln, qry[i - 1] == tgt[j - 1] ? 'M' : 'S');
				i--;
				j--;
			}
			else if (C[i][j] == C[i-1][j] + 1) {
				strbuf_append_char(aln, 'D');
				i--;
			}
			else { //(C[i][j] == C[i][j-1] + 1) {
				strbuf_append_char(aln, 'I');
				j--;
			}
		}
		strbuf_reverse(aln);
		compress_cigar(aln);
		strbuf_cat(cigar, aln);
		strbuf_free(aln);
	}
	FREE_MATRIX(C);
	return ret;
}


int gotoh(char *qry, size_t qry_len, char *tgt, size_t tgt_len, int gap_open, int gap_ext, subst_cost_fn subst, strbuf *cigar)
{
	size_t m = qry_len + 1;
	size_t n = tgt_len + 1;
	//NEW_MATRIX_0(C, int, m, n);
	int *C = ARR_NEW(int, n);
	//NEW_MATRIX_0(D, int, m, n);
	int *D = ARR_NEW(int, n);
	//NEW_MATRIX_0(I, int, m, n);/

	int I_i_j = 0;	
	int C_iminus1_j = 0;
	int C_iminus1_jminus1 = 0;

	int t = gap_open;

	C[0]  = 0;
	for (size_t j = 1; j < n; j++) {
		t += gap_ext;
		C[j] = t;
		//D[0][j] = t + gap_open;
		D[j] = t + gap_open;
	}
	t = gap_open;
	for (size_t i = 1; i < m; i++) {
		t += gap_ext;
		C[0] = t;
		//I[i][0] = t + gap_open;
		I_i_j = t + gap_open;
		for (size_t j = 1; j < n; j++) {
			int match = subst(qry[i - 1], tgt[j - 1]);
			//I[i][j] = MIN(C[i][j-1] + gap_open, I[i][j-1]) + gap_ext;
			I_i_j = MIN(C[j-1] + gap_open, I_i_j) + gap_ext; // C[j-1] contains C[i][j-1]
			//D[i][j] = MIN(C[i-1][j] + gap_open, D[i-1][j]) + gap_ext;
			D[j] = MIN(C[j] + gap_open, D[j]) + gap_ext; // C[j] contains C[i-1][j] 
			C_iminus1_j = C[j]; // C[j] contains C[i-1][j] so in the next j-iteration C_iminus1_j will contain C[i-1][j-1]
			//C[i][j] = MIN3(C[i-1][j-1] + match, D[i][j], I[i][j]);
			C[j] = MIN3(C_iminus1_jminus1 + match, D[j], I_i_j);
			C_iminus1_jminus1 = C_iminus1_j;
		}
	}
	int ret = C[tgt_len];
	FREE_MATRIX(C);
	FREE(D);
	//FREE_MATRIX(I);
	return ret;
}