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
#include "errlog.h"
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
	int *C = ARR_NEW(int, n);
	int *D = ARR_NEW(int, n);
	
	// NEW_MATRIX_0(CC, int, m, n);
	// NEW_MATRIX_0(DD, int, m, n);
	// NEW_MATRIX_0(II, int, m, n);

	int I_i_j = 0;	
	int C_iminus1_j = 0;
	int C_iminus1_jminus1 = 0;

	int t = gap_open;

	C[0]  = 0;
	//CC[0][0] = 0;
	D[0]  = gap_open;
	for (size_t j = 1; j < n; j++) {
		t += gap_ext;
		C[j] = t;
		//CC[0][j] = t;
		D[j] = t + gap_open;
		//DD[0][j] = t + gap_open;
	}
	t = gap_open;
	for (size_t i = 1; i < m; i++) {
		C_iminus1_j = C[0]; // C_iminus1_j := C[i-1][0]
		t += gap_ext;
		C[0] = t;
		D[0] = t;
		//CC[i][0] = t;
		I_i_j = t + gap_open;
		//II[i][0] = t + gap_open;
		C_iminus1_jminus1 = C_iminus1_j;// has to be set to C[i-1][0] so when it enters j=1 loop, this will be C[i-1][j-1];
		for (size_t j = 1; j < n; j++) {
			int match = subst(qry[i - 1], tgt[j - 1]);
			I_i_j = MIN(C[j-1] + gap_open, I_i_j) + gap_ext; // C[j-1] contains C[i][j-1]
			//II[i][j] = MIN(CC[i][j-1] + gap_open, II[i][j-1]) + gap_ext;
			//assert(I_i_j == II[i][j]);
			D[j] = MIN(C[j] + gap_open, D[j]) + gap_ext; // C[j] contains C[i-1][j] 
			//DD[i][j] = MIN(CC[i-1][j] + gap_open, DD[i-1][j]) + gap_ext;
			//assert(D[j] == DD[i][j]);
			C_iminus1_j = C[j]; // C[j] contains C[i-1][j] so in the next j-iteration C_iminus1_j will contain C[i-1][j-1]
			//assert(C_iminus1_jminus1 == CC[i-1][j-1]);
			C[j] = MIN3(C_iminus1_jminus1 + match, D[j], I_i_j);
			C_iminus1_jminus1 = C_iminus1_j;
			//CC[i][j] = MIN3(CC[i-1][j-1] + match, DD[i][j], II[i][j]);
			//assert (C[j] == CC[i][j]);
		}
	}
	int ret = C[tgt_len];
	FREE(C);
	FREE(D);
	//FREE_MATRIX(CC);
	//FREE_MATRIX(DD);
	//FREE_MATRIX(II);
	return ret;
}






#define FW 1
#define BW -1


static int gotoh_aln(char *qry, size_t qry_len, char *tgt, size_t tgt_len, int dir, int gap_open, int gap_ext, subst_cost_fn subst, int *C, int *D)
{
	/*strbuf *_q = strbuf_new();
	strbuf_nappend(_q, qry, qry_len);
	strbuf *_t = strbuf_new();
	strbuf_nappend(_t, tgt, tgt_len);
	DEBUG("aligning qry=%s tgt=%s\n", strbuf_as_str(_q), strbuf_as_str(_t));
	strbuf_free(_q);
	strbuf_free(_t);*/

	size_t m = qry_len + 1;
	size_t n = tgt_len + 1;

	int I_i_j = 0;	
	int C_iminus1_j = 0;
	int C_iminus1_jminus1 = 0;

	int t = gap_open;

	C[0]  = 0;
	D[0] = gap_open; 
	for (size_t j = 1; j < n; j++) {
		t += gap_ext;
		C[j] = t;
		D[j] = t + gap_open;
	}
	//DEBUG("C[%zu]: ",0);
	//ARR_PRINT(C, C, %d, 0, n, n, "");
	//DEBUG("D[%zu]: ",0);
	//ARR_PRINT(D, D, %d, 0, n, n, "");
	t = gap_open;
	for (size_t i = 1; i < m; i++) {
		//C_iminus1_j = C[0]; // C_iminus1_j := C[i-1][0]
		C_iminus1_jminus1 = C[0];//C_iminus1_j;// has to be set to C[i-1][0] so when it enters j=1 loop, this will be C[i-1][j-1];
		t += gap_ext;
		C[0] = t;
		D[0] = t;
		I_i_j = t + gap_open;
		for (size_t j = 1; j < n; j++) {
			int match = (dir==FW) ? subst(qry[i - 1], tgt[j - 1])  : subst(qry[qry_len - i], tgt[tgt_len - j]);
			I_i_j = MIN(C[j-1] + gap_open, I_i_j) + gap_ext; // C[j-1] contains C[i][j-1]
			D[j] = MIN(C[j] + gap_open, D[j]) + gap_ext; // C[j] contains C[i-1][j] 
			C_iminus1_j = C[j]; // C[j] contains C[i-1][j] so in the next j-iteration C_iminus1_j will contain C[i-1][j-1]
			C[j] = MIN3(C_iminus1_jminus1 + match, D[j], I_i_j);
			C_iminus1_jminus1 = C_iminus1_j;
		}
	//DEBUG("C[%zu]: ",i);
	//ARR_PRINT(C, C, %d, 0, n, n, "");
	//DEBUG("D[%zu]: ",i);
	//ARR_PRINT(D, D, %d, 0, n, n, "");
	}
	int ret = C[tgt_len];
	return ret;
}

typedef enum {
	CAT_CAT = 0,
	DEL_DEL = 1
} join_t;


int aln(char *qry, size_t from_qry, size_t to_qry, char *tgt, size_t tgt_len, int dir, int gap_open, int gap_ext, subst_cost_fn subst, strbuf *cigar, int *C1, int *D1, int *C2, int *D2) { 
	size_t m = to_qry - from_qry;
	size_t n = tgt_len;

	if (n == 0) {
		if (m > 0) {
			sbprintf(cigar, "%dD", m);
			C1[0] = C2[0] = D1[0] = D2[0] = gap_open + (m * gap_ext);
			return C1[0];
		} else {
			return 0;
		}
	} else if (m == 0) {
		//sbprintf(cigar, "%dI", n);
		C1[0] = C2[0] = 0;
		int t = gap_open;
		for (size_t j = 1; j <= n; j++) {
			t += gap_ext;
			C1[j] = C2[j] = t;
			D1[j] = D2[j] = t + gap_open;
		}
		return C1[n];
	} else if (m == 1) {
		int *C, *D; 
		C = (dir > 0) ? C1 : C2;
		D = (dir > 0) ? D1 : D2;
		gotoh_aln(&qry[from_qry], 1, tgt, tgt_len, dir, gap_open, gap_ext, subst, C, D);
		return C[n];
	} else {
		size_t mid_qry = (from_qry + to_qry) / 2;
		size_t top_len = mid_qry - from_qry;
		size_t bot_len = to_qry - mid_qry;
		gotoh_aln(&qry[from_qry], top_len, tgt, tgt_len, FW, gap_open, gap_ext, subst, C1, D1);
		/*
		strbuf *q = strbuf_new();
		strbuf_nappend(q, &qry[from_qry], top_len);
		DEBUG("qry_top='%s' tgt_top='%s'\n", strbuf_as_str(q), tgt);
		ARR_PRINT(C1, C1, %d, 0, n+1, n+1, "");
		ARR_PRINT(D1, D1, %d, 0, n+1, n+1, "");
		strbuf_clear(q);
		strbuf_nappend(q, &qry[mid_qry], bot_len);
		DEBUG("qry_bot='%s' tgt_bot='%s'\n", strbuf_as_str(q), tgt);
		*/
		gotoh_aln(&qry[mid_qry], bot_len, tgt, tgt_len, BW, gap_open, gap_ext, subst, C2, D2);
		/*
		ARR_PRINT(C2, C2, %d, 0, n+1, n+1, "");
		ARR_PRINT(D2, D2, %d, 0, n+1, n+1, "");
		strbuf_free(q);
		*/
		int min_cost = INT_MAX;
		size_t mid_tgt = 0;
		join_t join = 2;
		for (size_t j = 0; j <=n; j++) {
			if ((C1[j] + C2[n-j]) < min_cost) {
				mid_tgt = j;
				min_cost = C1[j] + C2[n-j];
				join = CAT_CAT;
			}
			if ((D1[j] + D2[n-j] - gap_open) < min_cost) {
				mid_tgt = j;
				min_cost = D1[j] + D2[n-j] - gap_open;
				join = DEL_DEL;
			}
		}
		return min_cost;
	}

}

int align(char *qry, size_t qry_len, char *tgt, size_t tgt_len, int gap_open, int gap_ext, subst_cost_fn subst, strbuf *cigar) { 

	int *C1, *D1, *C2, *D2;
	C1 = ARR_NEW(int, tgt_len + 1);
	D1 = ARR_NEW(int, tgt_len + 1);
	C2 = ARR_NEW(int, tgt_len + 1);
	D2 = ARR_NEW(int, tgt_len + 1);


	int ret = aln(qry, 0, qry_len, tgt, tgt_len, FW, gap_open, gap_ext, subst, cigar, C1, D1, C2, D2);


	FREE(C1);
	FREE(C2);
	FREE(D1);
	FREE(D2);
	return ret;
}