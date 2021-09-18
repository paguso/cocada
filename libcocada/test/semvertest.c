#include "CuTest.h"
#include "memdbg.h"
#include "new.h"
#include "semver.h"


void test_semver_from_str(CuTest *tc) {
    memdbg_reset();
    semver ver;
    char *src = "1.0.0-beta+exp.sha.5114f85";
    CuAssert(tc, "Parse error", semver_from_str(&ver, src)==0);
    CuAssertIntEquals(tc, 1, ver.major);
    CuAssertIntEquals(tc, 0, ver.minor);
    CuAssertIntEquals(tc, 0, ver.patch);
    CuAssertStrEquals(tc, "beta", ver.pre_rel);
    CuAssertStrEquals(tc, "exp.sha.5114f85", ver.build);
    FREE(ver.pre_rel);
    FREE(ver.build);
    CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *semver_get_test_suite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_semver_from_str);
    return suite;
}