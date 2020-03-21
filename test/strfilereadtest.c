#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"
#include "strread.h"
#include "strfileread.h"



static char *filename="test_strfileread.txt";

static char *file_content = "acgtacgtacgtacgtacgtacgtacgtacgtacgtacgt";

void test_setup() {
    FILE *file = fopen(filename, "w");
    fprintf(file, "%s", file_content);
    fclose(file);
}

void test_teardown() {
    remove(filename);
}


void test_getc(CuTest *tc) 
{
    test_setup();
    strfileread *sfr = strfileread_open(filename);
    strread *sr = strfileread_strread(sfr);
    size_t n = strlen(file_content);
    char c;
    for (size_t i=0; i<n; i++) {
        c = strread_getc(strfileread_strread(sfr));
        CuAssertCharEquals(tc, file_content[i], c);
    }
    c = strread_getc(strfileread_strread(sfr));
    CuAssertIntEquals(tc, EOF, c);
    test_teardown();
}



CuSuite *strfileread_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_getc);
    return suite;
}




