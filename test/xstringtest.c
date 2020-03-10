#include <math.h>
#include <stdlib.h>
#include <stdio.h>


#include "CuTest.h"

#include "bitsandbytes.h"
#include "mathutil.h"
#include "xstring.h"


void test_xstring_new(CuTest *tc)
{
	for (size_t len=0; len<1000; len++) {
		xstring *xs = xstring_new_with_len(len, (len>0)?(size_t)multceil(log2(len), BYTESIZE):0);
		xstring_free(xs);
	}
}


void test_xstring_get_set(CuTest *tc)
{
	for (size_t len=0; len<1000; len++) {
		xstring *xs = xstring_new_with_len(len, (len>0)?(size_t)multceil(log2(len), BYTESIZE):0);

		for (size_t i=0; i<len; i++) {
			xstr_set(xs, i, i);
		}

		//xstr_print(xs);

		for (size_t i=0; i<len; i++) {
			//printf("xstr[%zu]=%zu\n", i, xstr_get(xs,i));
			CuAssert(tc, "assertion failed", i==xstr_get(xs, i));
		}

		xstring_free(xs);
	}
}

void test_xstring_to_string(CuTest *tc)
{
	size_t l = 1000;
	xstring *xs = xstring_new(2);
	for (xchar_t i=0; i<l; i++) {
		xstr_push(xs, i);
	}
	dynstr *ds = dynstr_new();
	xstr_to_string(xs, ds);
	printf("%s\n",dstr_as_str(ds));
}

CuSuite *xstring_get_test_suite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_xstring_new);
	SUITE_ADD_TEST(suite, test_xstring_get_set);
	//SUITE_ADD_TEST(suite, test_xstring_to_string);
	return suite;
}