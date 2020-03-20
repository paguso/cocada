#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "CuTest.h"
#include "strread.h"
#include "strfileread.h"


void test_getc(CuTest *tc) 
{

}



CuSuite *strfileread_get_test_suite() 
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_getc);
    return suite;
}




