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

/**
 * @file alltests.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"


CuSuite *cdlexer_get_test_suite();
CuSuite *cddecl_get_test_suite();
CuSuite *cddoc_get_test_suite();
CuSuite *cdlint_get_test_suite();
CuSuite *cdsym_get_test_suite();
CuSuite *cdmd_get_test_suite();
CuSuite *cdconfig_get_test_suite();
CuSuite *cdmacro_get_test_suite();


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
	CuSuiteAddSuite(suite, cdmacro_get_test_suite());
	return suite;
}


int main(int argc, char **argv)
{
	CuSuite *rootSuite = create_test_suite();
	bool allOk = true;
	if (argc > 1 && !strcmp(argv[1], "-l")) {
		fprintf(stdout, "Available tests\n");
		CuSuitePrint(rootSuite);
		return 0;
	}
	if (argc == 1) {
		allOk = allOk && CuSuiteRun(rootSuite);
	} else {
		for (int i = 1; i < argc; i++) {
			fprintf(stdout, "Running suite %s tests...\n", argv[i]);
			allOk = allOk && CuSuiteRunSuite(rootSuite, argv[i]);
		}
	}
	CuSuitePrintResults(rootSuite, stdout);
	CuSuiteDelete(rootSuite);
	return allOk?0:1;
}
