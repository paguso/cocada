#include "CuTest.h"
#include "memdbg.h"
#include "new.h"
#include "semver.h"


void test_semver_from_str(CuTest *tc) {
    memdbg_reset();
    char *src = "1.0.0-beta+exp.sha.5114f85";
    semver *ver = semver_new_from_str(src);
    CuAssert(tc, "Parse error", ver != NULL);
    CuAssertIntEquals(tc, 1, ver->major);
    CuAssertIntEquals(tc, 0, ver->minor);
    CuAssertIntEquals(tc, 0, ver->patch);
    CuAssertStrEquals(tc, "beta", ver->pre_rel);
    CuAssertStrEquals(tc, "exp.sha.5114f85", ver->build);
    semver_free(ver);
    CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *semver_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_semver_from_str);
    return suite;
}