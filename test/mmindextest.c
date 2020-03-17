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
    size_t w[2] = {4, 3};
    size_t k[2] = {4, 3};
    mmindex *idx = mmindex_new(ab, 2, w, k);
    strstream *str = strstream_open_xstr(s);
    mmindex_index(idx, str);

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

}





CuSuite *mmindex_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_mmindex_index);

    return suite;
}
