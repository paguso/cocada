/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "CuTest.h"


CuSuite *cdlexer_get_test_suite();
CuSuite *cddecl_get_test_suite();
CuSuite *cddoc_get_test_suite();
CuSuite *cdlint_get_test_suite();
CuSuite *cdsym_get_test_suite();
CuSuite *cdmd_get_test_suite();
CuSuite *cdconfig_get_test_suite();


CuSuite* create_test_suite()
{
    CuSuite *suite = CuSuiteNew("all");
    CuSuiteAddSuite(suite, cdlexer_get_test_suite());
    CuSuiteAddSuite(suite, cddecl_get_test_suite());
    CuSuiteAddSuite(suite, cddoc_get_test_suite());
    CuSuiteAddSuite(suite, cdlint_get_test_suite());
    CuSuiteAddSuite(suite, cdsym_get_test_suite());
    CuSuiteAddSuite(suite, cdmd_get_test_suite());
    CuSuiteAddSuite(suite, cdconfig_get_test_suite());

    CuSuitePrintTests(suite);
    return suite;
}

void run_all_tests(CuSuite *rootSuite)
{
    CuString *output = CuStringNew();
    CuSuiteRun(rootSuite);
    CuSuiteSummary(rootSuite, output);
    CuSuiteDetails(rootSuite, output);
    printf("%s\n", output->buffer);
}

void run_suite_tests(CuSuite *rootSuite, int nsuites, char **suiteNames)
{
    CuString *output = CuStringNew();
    for (int i = 0; i < nsuites; i++) {
        fprintf(stdout, "Running suite %s tests...\n", suiteNames[i]);
        CuSuiteRunSuite(rootSuite, suiteNames[i]);
    }
    CuSuiteSummary(rootSuite, output);
    CuSuiteDetails(rootSuite, output);
    printf("%s\n", output->buffer);
}

int main(int argc, char **argv)
{
    CuSuite *rootSuite = create_test_suite();
    if (argc == 1) {
        run_all_tests(rootSuite);
    } else {
        run_suite_tests(rootSuite, argc - 1, (argc > 1) ? &argv[1]: NULL);
    }
    return 0;
}
