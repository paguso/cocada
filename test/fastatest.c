#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"
#include "fasta.h"

static char *filename = "test_fasta.fa";
static size_t nseq = 4;
static char *seq[4] ={ 
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


static void test_setup() 
{
    FILE *file = fopen(filename, "w");
    for (size_t i=0; i<nseq; i++) {
        fputc('>', file);
        fputs(desc[i], file);        
        fputc('\n', file);
        fputs(seq[i], file);        
        fputc('\n', file);
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
        const fasta_record *rr = fasta_next(f);
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
        fasta_record_reader *rr = fasta_next_reader(f);
        CuAssertStrEquals(tc, desc[i], rr->descr);
        size_t k = 0;
        xchar_t c;
        size_t l = strlen(seq[i]);
        while((c=strread_getc(rr->seqread)) != EOF) {
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
