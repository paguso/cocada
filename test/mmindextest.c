#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "CuTest.h"

#include "alphabet.h"
#include "dna.h"
#include "mmindex.h"
#include "fasta.h"


static char *filename = "test_mmindex.fa";
static char *file_contents = 
"\
>seq1\n\
acgtacgtacgtacgtacgtacgtacgtacgtacgtacgt\n\
";

static void test_setup() {
    FILE *f = fopen(filename, "w");
    fputs(file_contents, f);
    fclose(f);
}

static void test_teardown() {
    remove(filename);
}

void test_mmindex_index(CuTest *tc) 
{
    test_setup();

    alphabet *ab = dna_ab_new();
    size_t w[2] = {4, 3};
    size_t k[2] = {4, 3};
    mmindex *idx = mmindex_new(ab, 2, w, k);
    fasta *fa = fasta_open(filename);
    CuAssert(tc, "No FASTA sequence", fasta_has_next(fa));
    const fasta_record_reader *fr = fasta_next_reader(fa);
    
    mmindex_index(idx, fr->seqread);

    const vec *mmpos = mmindex_get(idx, xstring_new_from_arr_cpy("acgt", 4, sizeof(char)));
    size_t exp_mmpos[10] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36}; 
    CuAssertSizeTEquals(tc, 10, vec_len(mmpos));
    for (size_t i=0; i<10; i++) 
        CuAssertSizeTEquals(tc, exp_mmpos[i], vec_get_size_t(mmpos, i));

    mmpos = mmindex_get(idx, xstring_new_from_arr_cpy("acg", 3, sizeof(char)));
    size_t exp_mmpos2[10] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36}; 
    CuAssertSizeTEquals(tc, 10, vec_len(mmpos));
    for (size_t i=0; i<10; i++) 
        CuAssertSizeTEquals(tc, exp_mmpos2[i], vec_get_size_t(mmpos, i));
    
    mmpos = mmindex_get(idx, xstring_new_from_arr_cpy("cgt", 3, sizeof(char)));
    size_t exp_mmpos3[10] = {1, 5, 9, 13, 17, 21, 25, 29, 33, 37}; 
    CuAssertSizeTEquals(tc, 10, vec_len(mmpos));
    for (size_t i=0; i<10; i++) 
        CuAssertSizeTEquals(tc, exp_mmpos3[i], vec_get_size_t(mmpos, i));

    fasta_close(fa);
    test_teardown();
}





CuSuite *mmindex_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_mmindex_index);

    return suite;
}
