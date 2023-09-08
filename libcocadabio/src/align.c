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
