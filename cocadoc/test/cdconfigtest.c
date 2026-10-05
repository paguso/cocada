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
#include <string.h>
#include <sys/stat.h>

#include "CuTest.h"
#include "cdconfig.h"
#include "memdbg.h"
#include "new.h"


void test_cdpath_join(CuTest *tc)
{
	memdbg_reset();
	const char *cases[][3] = {
		{".", "./doc/markdown", "doc/markdown"},
		{"cfg", "./doc", "cfg/doc"},
		{"cfg/", "doc/", "cfg/doc"},
		{"cfg", "./", "cfg"},
		{".", "./", "."},
		{"cfg", "/abs/path", "/abs/path"},
		{"", "a", "a"},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		char *p = cdpath_join(cases[i][0], cases[i][1]);
		CuAssertStrEquals(tc, cases[i][2], p);
		FREE(p);
	}
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


static void write_text(const char *path, const char *text)
{
	FILE *f = fopen(path, "w");
	fputs(text, f);
	fclose(f);
}


void test_cdconfig_load(CuTest *tc)
{
	memdbg_reset();
	char root[] = "/tmp/cocadoc-test-XXXXXX";
	CuAssertTrue(tc, mkdtemp(root) != NULL);
	char path[512], cfgpath[512];
	const char *dirs[] = {"cfg", "base", "base/modA", "base/modA/src", "base/modA/src/sub",
	                      "base/modA/src/thrdpty", "base/modA/src/sub/thrdpty",
	                      "base/modB", "base/modB/src"
	                     };
	for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		snprintf(path, sizeof(path), "%s/%s", root, dirs[i]);
		mkdir(path, 0755);
	}
	const char *files[] = {"base/modA/src/x.h", "base/modA/src/sub/y.h",
	                       "base/modA/src/x.c", "base/modA/src/thrdpty/t.h",
	                       "base/modA/src/sub/thrdpty/u.h", "base/modA/src/old_deprecated.h",
	                       "base/modB/src/w.h"
	                      };
	for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
		snprintf(path, sizeof(path), "%s/%s", root, files[i]);
		write_text(path, "\n");
	}
	snprintf(cfgpath, sizeof(cfgpath), "%s/cfg/cocadoc.config", root);
	write_text(cfgpath,
	           "// a comment\n"
	           "PROJECT_NAME = Test   // trailing comment\n"
	           "VERSION =\n"
	           "\n"
	           "OUTPUT_DIRECTORY = ./out\n"
	           "BASE_DIR = ../base\n"
	           "MODULES = modB  modA\n"
	           "EXCLUDE_DIRS = thrdpty\n"
	           "EXCLUDE_FILES = *deprecated.h *temporary.h\n");

	char err[256] = "";
	cdconfig *cfg = cdconfig_load(cfgpath, err, sizeof(err));
	CuAssert(tc, err, cfg != NULL);
	CuAssertStrEquals(tc, "Test", cfg->project_name);
	CuAssertTrue(tc, cfg->version == NULL);
	CuAssertStrEquals(tc, "markdown", cfg->output_type);
	snprintf(path, sizeof(path), "%s/cfg/out", root);
	CuAssertStrEquals(tc, path, cfg->output_dir);
	CuAssertSizeTEquals(tc, 2, vec_len(cfg->modules));

	vec *headers = cdconfig_find_headers(cfg);
	CuAssertSizeTEquals(tc, 3, vec_len(headers));
	const char *expected[] = {"base/modB/src/w.h", "base/modA/src/sub/y.h", "base/modA/src/x.h"};
	for (size_t i = 0; i < 3 && i < vec_len(headers); i++) {
		snprintf(path, sizeof(path), "%s/cfg/../%s", root, expected[i]);
		CuAssertStrEquals(tc, path, (char *)vec_get_rawptr(headers, i));
	}
	DESTROY(headers, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	cdconfig_free(cfg);

	// errors
	const char *bad[][2] = {
		{"FOO = 1\n", "unknown key FOO"},
		{"MODULES\n", "expected KEY = value"},
		{"OUTPUT_TYPE = html\n", "unsupported OUTPUT_TYPE html"},
	};
	for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
		write_text(cfgpath, bad[i][0]);
		CuAssertTrue(tc, cdconfig_load(cfgpath, err, sizeof(err)) == NULL);
		CuAssert(tc, err, strstr(err, bad[i][1]) != NULL);
	}
	snprintf(path, sizeof(path), "%s/none.config", root);
	CuAssertTrue(tc, cdconfig_load(path, err, sizeof(err)) == NULL);
	CuAssert(tc, err, strstr(err, "cannot read") != NULL);

	char cmd[600];
	snprintf(cmd, sizeof(cmd), "rm -rf %s", root);
	CuAssertIntEquals(tc, 0, system(cmd));
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdconfig_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdconfig");
	SUITE_ADD_TEST(suite, test_cdpath_join);
	SUITE_ADD_TEST(suite, test_cdconfig_load);
	return suite;
}
