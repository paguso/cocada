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

#include "CuTest.h"
#include "memdbg.h"
#include "new.h"
#include "semver.h"


void test_semver_from_str(CuTest *tc)
{
	memdbg_reset();
	char *src = "1.0.0-beta+exp.sha.5114f85";
	semver_res result = semver_new_from_str(src);
	CuAssert(tc, "Semver parse error", result.ok);
	semver *ver = result.val;
	CuAssertIntEquals(tc, 1, ver->major);
	CuAssertIntEquals(tc, 0, ver->minor);
	CuAssertIntEquals(tc, 0, ver->patch);
	CuAssertStrEquals(tc, "beta", ver->pre_rel);
	CuAssertStrEquals(tc, "exp.sha.5114f85", ver->build);
	semver_free(ver);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *semver_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_semver_from_str);
	return suite;
}