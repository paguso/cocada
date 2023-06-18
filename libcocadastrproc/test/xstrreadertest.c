#include "CuTest.h"
#include "memdbg.h"
#include "xstr.h"
#include "xstrread.h"
#include "xstrreader.h"


void test_xstrreader(CuTest *tc)
{
    memdbg_reset();
    xstr *s = xstr_new_from_arr_cpy("0123456789012345678901234567890123456789", 40, 1);
    xstrreader *xrdr = xstrreader_open(s);
    xchar_wt c;
    c = xstrread_getc(xstrreader_as_xstrread(xrdr));
    CuAssertCharEquals(tc, '0', c);
    size_t nread = xstrread_read_until(xstrreader_as_xstrread(xrdr), NULL, '0');
    CuAssertSizeTEquals(tc, 9, nread);
    c = xstrread_getc(xstrreader_as_xstrread(xrdr));
    CuAssertCharEquals(tc, '0', c);
    xstr *dest = xstr_new(1);
    nread = xstrread_read(xstrreader_as_xstrread(xrdr), dest, 19);
    CuAssertSizeTEquals(tc, 19, xstr_len(dest));
    xstr_clear(dest);
    c = xstrread_getc(xstrreader_as_xstrread(xrdr));
    CuAssertCharEquals(tc, '0', c);
    nread = xstrread_read(xstrreader_as_xstrread(xrdr), dest, 19);
    CuAssertSizeTEquals(tc, 9, xstr_len(dest));
    xstr_free(dest);
    xstrreader_close(xrdr);
    xstr_free(s);
    if (!memdbg_is_empty()) {
        memdbg_print_stats(stderr, true);
    }
    CuAssert(tc, "Memory leak!", memdbg_is_empty());

}



CuSuite* xstrreader_get_test_suite(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_xstrreader);
    return suite;
}