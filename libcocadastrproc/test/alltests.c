
#include <stdio.h>
#include <stdlib.h>

#include "CuTest.h"


CuSuite *alphabet_get_test_suite();


void run_all_tests(void)
{
	CuString *output = CuStringNew();
	CuSuite *suite = CuSuiteNew();

	CuSuiteAddSuite(suite, alphabet_get_test_suite());

	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}


void print_count() ;


int main(void)
{
	run_all_tests();
	return 0;
}
