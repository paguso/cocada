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
#include "fasta.h"

static char *filename = "test_fasta.fa";
static size_t nseq = 4;
static char *seq[4] = {
	"aaaaaaaaaa",
	"ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n\
cccccccccccccccccccccccccccccccccccccccc",
	"ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg",
	"t\n\
tt\n\
tttt\n\
tttttttt\n\
tttttttttttttttt\n\
tttttttttttttttttttttttttttttttt\n\
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt\n\
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt\n\
tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt"
};
static char *desc[4] = {
	"",
	"seq1",
	"seq2 short description",
	"seq3 a very very very very a very very very very a very very very very a very very very very a very very very very a very very very very long sequence"
};
static size_t desc_offsets[4] = {-1, -1, -1, -1};
static size_t seq_offsets[4] = {-1, -1, -1, -1};

static void test_setup()
{
	FILE *file = fopen(filename, "w");
	size_t offset = 0;
	for (size_t i=0; i<nseq; i++) {
		desc_offsets[i] = offset;
		fputc('>', file);
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
	}
	fclose(file);
}


static void test_teardown()
{
	remove(filename);
}


void test_fasta_next(CuTest *tc)
{
	test_setup();

	fasta *f = fasta_open(filename);
	size_t i=0;
	for(i=0; fasta_has_next(f); i++) {
		const fasta_rec *rr = fasta_next(f);
		CuAssertSizeTEquals(tc, desc_offsets[i], rr->descr_offset);
		CuAssertSizeTEquals(tc, seq_offsets[i], rr->seq_offset);
		CuAssertStrEquals(tc, desc[i], rr->descr);
		size_t seq_i_len = strlen(seq[i]);
		size_t k = 0;
		for(size_t j=0, rl=strlen(rr->seq); j < rl; j++) {
			while( k<seq_i_len && seq[i][k]=='\n') k++;
			CuAssert(tc, "fasta read error: read too many chars", k<seq_i_len);
			CuAssert(tc, "fasta read error: char mismatch", seq[i][k]==rr->seq[j]);
			k++;
		}
		CuAssert(tc, "fasta read error: premature end of sequence", k==seq_i_len);
	}
	CuAssertSizeTEquals(tc, nseq, i);

	test_teardown();
}


void test_fasta_next_read(CuTest *tc)
{
	test_setup();

	fasta *f = fasta_open(filename);
	size_t i=0;
	for(i=0; fasta_has_next(f); i++) {
		const fasta_rec_rdr *rr = fasta_next_reader(f);
		CuAssertSizeTEquals(tc, desc_offsets[i], rr->descr_offset);
		CuAssertSizeTEquals(tc, seq_offsets[i], rr->seq_offset);
		CuAssertStrEquals(tc, desc[i], rr->descr);
		size_t k = 0;
		char c;
		size_t l = strlen(seq[i]);
		while((c=strread_getc(rr->seqrdr)) != EOF) {
			while( k<l && seq[i][k]=='\n') k++;
			CuAssert(tc, "fasta read error: read too many chars", k<l);
			CuAssert(tc, "fasta read error: char mismatch", seq[i][k]==c);
			k++;
		}
		CuAssert(tc, "fasta read error: premature end of sequence", k==l);
	}
	CuAssertSizeTEquals(tc, nseq, i);

	test_teardown();
}



CuSuite *fasta_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_fasta_next);
	SUITE_ADD_TEST(suite, test_fasta_next_read);
	return suite;

}
