#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "CuTest.h"

#include "alphabet.h"
#include "dna.h"
#include "mmindex.h"


void test_mmindex_index(CuTest *tc) 
{
    alphabet *ab = dna_ab_new();
    xstring *s = xstring_new_from_arr_cpy("acgtacgtacgtacgtacgtacgtacgtacgtacgtacgt", 40, sizeof(char));
    CuAssertSizeTEquals(tc, 40, xstr_len(s));
    size_t w[] = {4};
    size_t k[] = {4};
    mmindex *idx = mmindex_new(ab, 1, w, k);
    strstream *str = strstream_open_xstr(s);
    mmindex_index(idx, str);
}





CuSuite *mmindex_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_mmindex_index);

    return suite;
}
