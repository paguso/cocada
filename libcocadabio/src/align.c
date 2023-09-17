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


int unit_subst(char a, char b)
{
    return (a == b) ? 0 : 1;
}


static int read_number_(char *cigar, size_t pos, int *nb) 
{
	//const char *s = strbuf_as_str(cigar);
	//s = &s[pos];
	*nb = 0;
	int p = pos;
	while ('0' <= cigar[p] && cigar[p] <= '9') {
		*nb = (*nb) * 10;
		*nb = (*nb) + (cigar[p] - '0');
	  	p++;
	}
	return p-pos;	
}


static void compress_cigar(strbuf *cigar)
{
	size_t n = strbuf_len(cigar);
	//char *cigar_ = strbuf_as_str(cigar);
	if (n == 0) return;
	size_t i = 0;
	int count;
	i += read_number_(strbuf_as_str(cigar), i, &count);
	char last_c = strbuf_get(cigar, i++);
	int run_len = count;
	while (i < n) {
		i += read_number_(strbuf_as_str(cigar), i, &count);
		char c = strbuf_get(cigar, i++);
		if (c == last_c) {
			run_len += count;
		}
		else {
			sbprintf(cigar, "%d%c", run_len, last_c);
			last_c = c;
			run_len = count;
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
				strbuf_append(aln, "D1");
				i--;
			}
			else if (i == 0) {
				strbuf_append(aln, "I1");
				j--;
			}
			else if (C[i][j] == C[i-1][j-1] + (qry[i - 1] == tgt[j - 1] ? 0 : 1)) {
				strbuf_append(aln, qry[i - 1] == tgt[j - 1] ? "M1" : "S1");
				i--;
				j--;
			}
			else if (C[i][j] == C[i-1][j] + 1) {
				strbuf_append(aln, "D1");
				i--;
			}
			else { //(C[i][j] == C[i][j-1] + 1) {
				strbuf_append(aln, "I1");
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


static int gotoh_affine_cost(const char *qry, size_t qry_len, const char *tgt, size_t tgt_len, int gap_open, int gap_ext, subst_cost_fn subst)
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



typedef enum {
	FW = 1,
	BW = -1
} dir_t;

static int aff_slice_cost(const char *qry, size_t qry_len, const char *tgt, size_t tgt_len, dir_t dir, int gap_open, int initial_del_gap_open, int gap_ext, subst_cost_fn subst, int *C, int *D)
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
	//t = gap_open;
	t = initial_del_gap_open;
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
	CAT_JOIN = 0,
	DEL_JOIN = 1
} join_t;

static inline int gap(size_t len, int gap_open, int gap_ext) 
{
	return (len) ? (gap_open + (len * gap_ext)) : 0;
}

static int aff_slice_aln(const char *qry, size_t from_qry, size_t to_qry, const char *tgt, size_t from_tgt, size_t to_tgt, int gap_open, int gap_open_begin, int gap_open_end, int gap_ext, subst_cost_fn subst, strbuf *cigar, int *C1, int *D1, int *C2, int *D2) { 
	size_t m = to_qry - from_qry;
	size_t n = to_tgt - from_tgt;

	if (n == 0) {
		if (m > 0) {
			sbprintf(cigar, "%dD", m);
			return gap(m, MIN(gap_open_begin, gap_open_end), gap_ext);
		} else {
			return 0;
		}
	} else if (m == 0) {
		sbprintf(cigar, "%dI", n);
		return gap(n, MIN(gap_open, gap_open_end), gap_ext);
	} else if (m == 1) {
		int min_cost = INT_MAX;
		size_t j_min = n;
		// insert tgt[:j], (mis)match qry[0]=tgt[j], and insert tgt[j+1:]
		for (size_t j = 0; j < n; j++) {
			int match = subst(qry[from_qry], tgt[from_tgt+j]);
			int gap_begin = gap(j, gap_open, gap_ext);
			int gap_end = gap(n - j - 1, gap_open, gap_ext);
			if ( (gap_begin + match + gap_end) < min_cost) {
				min_cost = gap_begin + match + gap_end;
				j_min = j;
			}
		}
		assert(j_min < n);
		// delete qry[0] and insert tgt[:] - starts with deletion
		if (gap(1, gap_open_begin, gap_ext) + gap(n, gap_open, gap_ext) < min_cost) { 
			min_cost = gap(1, gap_open_begin, gap_ext) + gap(n, gap_open, gap_ext);
			j_min = n; 
		}
		// insert tgt[:] and delete qry[0] - ends with deletion
		if (gap(n, gap_open, gap_ext) + gap(1, gap_open_end, gap_ext) < min_cost) { 
			min_cost = gap(n, gap_open, gap_ext) + gap(1, gap_open_end, gap_ext);
			j_min = n+1; 
		}
		if (j_min < n) {
			if (j_min > 0) {
				sbprintf(cigar, "%zuI", j_min);
			}
			bool match = qry[from_qry]==tgt[from_tgt+j_min];
			sbprintf(cigar, "1%c", (match?'M':'S'));
			if ((n - j_min - 1) > 0) {
				sbprintf(cigar, "%zuI", (n - j_min -1));
			}
		} else if (j_min == n){
			sbprintf(cigar, "1D%zuI", n);
		} else {
			assert (j_min == n+1);
			sbprintf(cigar, "%zuI1D", n);
		}
		return min_cost;
	} else {
		size_t mid_qry = (from_qry + to_qry) / 2;
		size_t top_len = mid_qry - from_qry;
		size_t bot_len = to_qry - mid_qry;
		size_t tgt_len = to_tgt - from_tgt;

		aff_slice_cost(&qry[from_qry], top_len, &tgt[from_tgt], tgt_len, FW, gap_open, gap_open_begin, gap_ext, subst, C1, D1);
		aff_slice_cost(&qry[mid_qry], bot_len, &tgt[from_tgt], tgt_len, BW, gap_open, gap_open_end, gap_ext, subst, C2, D2);

		int min_cost = INT_MAX;
		size_t mid_tgt = 0;
		join_t join = CAT_JOIN;
		for (size_t j = 0; j <=n; j++) {
			if ((C1[j] + C2[n-j]) < min_cost) {
				mid_tgt = from_tgt + j;
				min_cost = C1[j] + C2[n-j];
				join = CAT_JOIN;
			}
			if ((D1[j] + D2[n-j] - gap_open) < min_cost) {
				mid_tgt = from_tgt + j;
				min_cost = D1[j] + D2[n-j] - gap_open;
				join = DEL_JOIN;
			}
		}
		if (join == CAT_JOIN) {
			int tc = aff_slice_aln(qry, from_qry, mid_qry, tgt, from_tgt, mid_tgt, gap_open, gap_open_begin, gap_open, gap_ext, subst, cigar, C1, D1, C2, D2);
			int bc = aff_slice_aln(qry, mid_qry, to_qry, tgt, mid_tgt, to_tgt, gap_open, gap_open, gap_open_end, gap_ext, subst, cigar, C1, D1, C2, D2);
			assert (min_cost == tc + bc);
		} else if (join == DEL_JOIN) {
			int tc = aff_slice_aln(qry, from_qry, mid_qry-1, tgt, from_tgt, mid_tgt, gap_open, gap_open_begin, 0, gap_ext, subst, cigar, C1, D1, C2, D2);
			sbprintf(cigar, "2D");
			int bc = aff_slice_aln(qry, mid_qry+1, to_qry, tgt, mid_tgt, to_tgt, gap_open, 0, gap_open_end, gap_ext, subst, cigar, C1, D1, C2, D2);
			assert (min_cost >= tc + bc); // exact expression ???
		} else {
			assert(false);
		}
		return min_cost;
	}

}

int affine_global_align(const char *qry, size_t qry_len, const char *tgt, size_t tgt_len, 
	int gap_open, int gap_ext, subst_cost_fn subst, strbuf *cigar) { 
	if (cigar) {
		int *C1, *D1, *C2, *D2;
		C1 = ARR_NEW(int, tgt_len + 1);
		D1 = ARR_NEW(int, tgt_len + 1);
		C2 = ARR_NEW(int, tgt_len + 1);
		D2 = ARR_NEW(int, tgt_len + 1);

		strbuf *cigar_ = strbuf_new();

		int ret = aff_slice_aln(qry, 0, qry_len, tgt, 0, tgt_len, gap_open, gap_open, gap_open, gap_ext, subst, cigar_, C1, D1, C2, D2);

		compress_cigar(cigar_);
		strbuf_cat(cigar, cigar_);
		strbuf_free(cigar_);

		FREE(C1);
		FREE(C2);
		FREE(D1);
		FREE(D2);
		return ret;
	} else {
		return gotoh_affine_cost(qry, qry_len, tgt, tgt_len, gap_open, gap_ext, subst);
	}
}

void fprintf_alignment(FILE *out, const char *qry, size_t qry_from, size_t qry_to, const char *tgt, size_t tgt_from, size_t tgt_to, const char *cigar, size_t cigar_len)
{
	size_t qry_len = qry_to - qry_from;
	size_t tgt_len = tgt_to - tgt_from;
	qry = &qry[qry_from];
	tgt = &tgt[tgt_from];
	strbuf *qry_line = strbuf_new();
	strbuf *tgt_line = strbuf_new();
	strbuf *cig_line = strbuf_new();
	size_t tpos = 0, qpos = 0, cpos = 0, prev_cpos = 0;
	int pos_len = (int)log10(MAX(qry_from, tgt_from)) + 1;
	sbprintf(tgt_line, "T[%*d] ", pos_len, tgt_from);
	sbprintf(qry_line, "Q[%*d] ", pos_len, qry_from);
	sbprintf(cig_line, "%*s", pos_len+4, "");
	while (cpos < cigar_len) { 
		int count;
		cpos += read_number_(cigar, cpos, &count);
		char op = cigar[cpos++];
		int clen = cpos - prev_cpos;
		int print_len = 1 + MAX(clen, count); // width of the aligned fragment

		switch (op) {
			case 'M':
			case 'S':
				sbprintf(tgt_line, "%.*s", count, &tgt[tpos]);
				sbprintf(tgt_line, "%*s", (print_len - count), " ");
				tpos += count;
				sbprintf(qry_line, "%.*s", count, &qry[qpos]);
				sbprintf(qry_line, "%*s", (print_len - count), " ");
				qpos += count;
				sbprintf(cig_line, "%.*s", clen, &cigar[prev_cpos]);
				sbprintf(cig_line, "%*s", (print_len - clen), " ");
				prev_cpos = cpos;
				break;
			case 'I':
				sbprintf(tgt_line, "%.*s", count, &tgt[tpos]);
				sbprintf(tgt_line, "%*s", (print_len - count), " ");
				tpos += count;
				for (int i=0; i < count; i++) {
					sbprintf(qry_line, "-");
				}
				//sbprintf(qry_line, "%.*s", count, &qry[qpos]);
				sbprintf(qry_line, "%*s", (print_len - count), " ");
				//qpos += count;
				sbprintf(cig_line, "%.*s", clen, &cigar[prev_cpos]);
				sbprintf(cig_line, "%*s", (print_len - clen), " ");
				prev_cpos = cpos;
				break;
			case 'D':
				for (int i=0; i < count; i++) {
					sbprintf(tgt_line, "-");
				}
				//sbprintf(tgt_line, "%.*s", count, &tgt[tpos]);
				sbprintf(tgt_line, "%*s", (print_len - count), " ");
				//tpos += count;
				sbprintf(qry_line, "%.*s", count, &qry[qpos]);
				sbprintf(qry_line, "%*s", (print_len - count), " ");
				qpos += count;
				sbprintf(cig_line, "%.*s", clen, &cigar[prev_cpos]);
				sbprintf(cig_line, "%*s", (print_len - clen), " ");
				prev_cpos = cpos;
				break;
			default:
				assert(false);
		}
	}
	assert (tpos == tgt_len && qpos == qry_len);
	pos_len = (int)log10(MAX(qry_to, tgt_to)) + 1;
	sbprintf(tgt_line, "[%*d]", pos_len, tgt_to);
	sbprintf(qry_line, "[%*d]", pos_len, qry_to);
	sbprintf(cig_line, "%*s", pos_len+2, "");
	fprintf(out, "%s\n%s\n%s\n", strbuf_as_str(tgt_line),  strbuf_as_str(qry_line),  strbuf_as_str(cig_line));
	strbuf_free(tgt_line);
	strbuf_free(qry_line);
	strbuf_free(cig_line);
}
