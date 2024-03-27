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


CuSuite *cli_get_test_suite();
CuSuite *semver_get_test_suite();


void run_all_tests(void)
{
	CuString *output = CuStringNew();
	CuSuite *suite = CuSuiteNew();

	CuSuiteAddSuite(suite, cli_get_test_suite());
	CuSuiteAddSuite(suite, semver_get_test_suite());

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
