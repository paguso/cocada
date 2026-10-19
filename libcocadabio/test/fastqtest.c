
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
#include <string.h>

#include "CuTest.h"
#include "fastq.h"
#include "memdbg.h"

static char *filename = "test_fastq.fa";
static size_t nseq = 4;
static char *seq[4] = {
	"aaaaaaaaaa",
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n"
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
	"ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg",

	// "t\n"
	// "tt\n"
	// "tttt\n"
	// "tttttttt\n"
	// "tttttttttttttttt\n"
	// "tttttttttttttttttttttttttttttttt\n"
	// "tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt\n"
	// "tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt\n"
	"tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt"
};


static char *id[4] = {
	"seq0",
	"seq1",
	"seq2",
	"seq3"
};

static char *desc[4] = {
	"",
	"",
	"short description",
	"a very very very very a very very very very a very very very very a very very very very a very very very very a very very very very long sequence"
};

static size_t id_offsets[4] = {-1, -1, -1, -1};
static size_t seq_offsets[4] = {-1, -1, -1, -1};
static size_t qlty_offsets[4] = {-1, -1, -1, -1};

static void test_setup()
{
	FILE *file = fopen(filename, "w");
	size_t offset = 0;
	for (size_t i = 0; i < nseq; i++) {
		id_offsets[i] = offset;
		fputc('@', file);
		offset += 1;
		fputs(id[i], file);
		offset += strlen(id[i]);
		fputc(' ', file);
		offset += 1;
		fputs(desc[i], file);
		offset += strlen(desc[i]);
		fputc('\n', file);
		offset += 1;
		seq_offsets[i] = offset;
		fputs(seq[i], file);
		offset += strlen(seq[i]);
		fputc('\n', file);
		offset += 1;
		fputc('+', file);
		offset += 1;
		fputs(desc[i], file);
		offset += strlen(desc[i]);
		fputc('\n', file);
		offset += 1;
		qlty_offsets[i] = offset;
		fputs(seq[i], file);
		offset += strlen(seq[i]);
		fputc('\n', file);
		offset += 1;
	}
	fclose(file);
}


static void test_teardown()
{
	remove(filename);
}

void test_fastq_next(CuTest *tc)
{
	memdbg_reset();
	test_setup();

	fastq_open_res result = fastq_open(filename);
	CuAssert(tc, "Error opening fasta", result.ok);
	fastq *f = result.val.ok;
	size_t i = 0;
	for (i = 0; fastq_has_next(f); i++) {
		fastq_next_res res = fastq_next(f);
		CuAssert(tc, "fastq_next error", res.ok);
		const fastq_rec *rr = res.val.ok;
		CuAssertSizeTEquals(tc, id_offsets[i], rr->id_offset);
		CuAssertSizeTEquals(tc, seq_offsets[i], rr->seq_offset);
		CuAssertSizeTEquals(tc, qlty_offsets[i], rr->qlty_offset);
		CuAssertStrEquals(tc, id[i], rr->id);
		CuAssertStrEquals(tc, desc[i], rr->descr);
		size_t seq_i_len = strlen(seq[i]);
		size_t k = 0;
		for (size_t j = 0, rl = strlen(rr->seq); j < rl; j++) {
			while ( k < seq_i_len && seq[i][k] == '\n') k++;
			CuAssert(tc, "fasta read error: read too many chars", k < seq_i_len);
			CuAssert(tc, "fasta read error: char mismatch", seq[i][k] == rr->seq[j]);
			k++;
		}
		CuAssert(tc, "fastq read error: premature end of sequence", k == seq_i_len);
	}
	CuAssertSizeTEquals(tc, nseq, i);
	fastq_close(f);

	test_teardown();
	if (!memdbg_is_empty()) {
		memdbg_print_stats(stdout, true);
	}
	CuAssert(tc, "Memory leak!", memdbg_is_empty());
}


CuSuite *fastq_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("fastq");
	SUITE_ADD_TEST(suite, test_fastq_next);
	return suite;
}
