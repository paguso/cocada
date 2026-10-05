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
 * @file cdsymtest.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <string.h>

#include "CuTest.h"
#include "cdfile.h"
#include "cdsym.h"
#include "memdbg.h"
#include "new.h"


static const char *SRC_X =
    "/**\n * @file x.h\n * @author X\n * @brief X.\n */\n"
    "#if WIDTH == 1\n"
    "typedef char xchar_t;\n"
    "#else\n"
    "typedef int xchar_t;\n"
    "#endif\n"
    "typedef struct {\n\tint len;\n\tchar *s;\n} xstr;\n"
    "enum colour {\n\tRED,\n\tGREEN\n};\n"
    "void f(void);\n";

static const char *SRC_Y =
    "/**\n * @file y.h\n * @author Y\n * @brief Y.\n */\n"
    "void f(void);\n";


void test_cdsym_table(CuTest *tc)
{
	memdbg_reset();
	cdfile *x = cdfile_new_from_str("lib/x.h", SRC_X, strlen(SRC_X));
	cdfile *y = cdfile_new_from_str("y.h", SRC_Y, strlen(SRC_Y));
	vec *files = vec_new(sizeof(cdfile *));
	vec_push_rawptr(files, x);
	vec_push_rawptr(files, y);
	cdsymtab *tab = cdsymtab_new(files);

	size_t n;
	const cdsym *s = cdsymtab_resolve(tab, "x.h", NULL, &n);
	CuAssertTrue(tc, s && s->kind == CDD_FILE && s->file == x && n == 1);
	CuAssertTrue(tc, s->decl && s->decl->kind == CDD_FILE);

	// alternative declarations in one file: one file, not ambiguous
	s = cdsymtab_resolve(tab, "xchar_t", NULL, &n);
	CuAssertTrue(tc, s && s->kind == CDD_TYPEDEF && n == 1);

	s = cdsymtab_resolve(tab, "xstr.len", NULL, &n);
	CuAssertTrue(tc, s && s->kind == CDD_MEMBER && n == 1);
	CuAssertStrEquals(tc, "xstr", s->parent->name);
	CuAssertTrue(tc, cdsymtab_resolve(tab, "len", NULL, NULL) == NULL);

	// enum constants: qualified and plain
	CuAssertTrue(tc, cdsymtab_resolve(tab, "colour.RED", NULL, NULL) != NULL);
	s = cdsymtab_resolve(tab, "GREEN", NULL, NULL);
	CuAssertTrue(tc, s && s->kind == CDD_MEMBER);

	// declared in two files: prefer the referring one
	s = cdsymtab_resolve(tab, "f", y, &n);
	CuAssertTrue(tc, s && s->file == y && n == 2);
	s = cdsymtab_resolve(tab, "f", x, NULL);
	CuAssertTrue(tc, s->file == x);

	CuAssertTrue(tc, cdsymtab_resolve(tab, "nothing", NULL, &n) == NULL && n == 0);

	cdsymtab_free(tab);
	DESTROY_FLAT(files, vec);
	cdfile_free(x);
	cdfile_free(y);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdsym_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdsym");
	SUITE_ADD_TEST(suite, test_cdsym_table);
	return suite;
}
