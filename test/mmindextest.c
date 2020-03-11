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
    //xstring *str = xstring_from("acgt", sizeof(char));
}





CuSuite *mmindex_get_test_suite()
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_mmindex_index);

    return suite;
}
