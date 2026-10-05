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

#include <string.h>

#include "CuTest.h"
#include "cdlint.h"
#include "memdbg.h"
#include "new.h"


// Whether there is a warning at line with the given rule, name and message part
static bool has_warn(const vec *warns, size_t line, const char *rule, const char *name,
                     const char *substr)
{
	for (size_t i = 0; i < vec_len(warns); i++) {
		const cdwarn *w = vec_get(warns, i);
		if (w->line == line && strcmp(w->rule, rule) == 0 && strcmp(w->name, name) == 0
		        && strstr(w->msg, substr)) {
			return true;
		}
	}
	return false;
}


#define ASSERT_WARN(LINE, RULE, NAME, SUBSTR) \
	CuAssert(tc, "missing warning " RULE " " NAME ": " SUBSTR, \
	         has_warn(warns, LINE, RULE, NAME, SUBSTR))


static const char *SRC =
    "#ifndef T_H\n"                              // 1
    "#define T_H\n"                              // 2
    "/**\n"                                      // 3
    " * @file t.h\n"                             // 4
    " * @author A\n"                             // 5
    " * @brief Test.\n"                          // 6
    " */\n"                                      // 7
    "\n"                                         // 8
    "/**\n"                                      // 9
    " * @brief Adds.\n"                          // 10
    " * @param a The a.\n"                       // 11
    " * @param c Not a parameter.\n"             // 12
    " */\n"                                      // 13
    "int add(int a, int b);\n"                   // 14
    "\n"                                         // 15
    "void undoc(void);\n"                        // 16
    "\n"                                         // 17
    "#define MAX(A, B) ((A) > (B) ? (A) : (B))\n" // 18
    "\n"                                         // 19
    "/**\n"                                      // 20
    " * @brief Sets.\n"                          // 21
    " * @param @move v The vector.\n"            // 22
    " * @param n The size.\n"                    // 23
    " * @return Nothing.\n"                      // 24
    " */\n"                                      // 25
    "void set(size_t n, vec *v);\n"              // 26
    "\n"                                         // 27
    "/**\n"                                      // 28
    " * @brief Gets.\n"                          // 29
    " * @param v The vector.\n"                  // 30
    " * @return The value.\n"                    // 31
    " */\n"                                      // 32
    "const void *get(const vec *v);\n"           // 33
    "\n"                                         // 34
    "typedef enum {\n"                           // 35
    "\tX, /**< The x */\n"                       // 36
    "\tY\n"                                      // 37
    "} e_t;\n"                                   // 38
    "\n"                                         // 39
    "struct _private;\n"                         // 40
    "#endif\n";


void test_cdlint_decls(CuTest *tc)
{
	memdbg_reset();
	vec *warns = cdlint("dir/t.h", SRC, strlen(SRC));
	ASSERT_WARN(12, "DC8", "add", "@param c is not a parameter of add");
	ASSERT_WARN(9, "DC8", "add", "parameter b is not documented");
	ASSERT_WARN(9, "DC8", "add", "return value is not documented");
	ASSERT_WARN(16, "DC4", "undoc", "undocumented function");
	ASSERT_WARN(18, "DC4", "MAX", "undocumented macro");
	ASSERT_WARN(20, "DC8", "set", "@params are not in the order of the declaration");
	ASSERT_WARN(20, "DC8", "set", "@return for a function returning void");
	ASSERT_WARN(35, "DC4", "e_t", "undocumented type");
	ASSERT_WARN(37, "DC4", "e_t.Y", "undocumented member");
	// get, X, the file comment and _private are fine
	CuAssertSizeTEquals(tc, 9, vec_len(warns));
	cdwarn_vec_free(warns);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cdlint_file(CuTest *tc)
{
	memdbg_reset();
	vec *warns = cdlint("other.h", SRC, strlen(SRC));
	ASSERT_WARN(3, "DC3", "other.h", "@file t.h does not match the file name other.h");
	cdwarn_vec_free(warns);

	const char *src = "/**\n * @brief F.\n */\nvoid f(void);\n";
	warns = cdlint("f.h", src, strlen(src));
	ASSERT_WARN(1, "DC3", "f.h", "no file comment");
	CuAssertSizeTEquals(tc, 1, vec_len(warns));
	cdwarn_vec_free(warns);

	src = "/**\n * @file g.h\n * @brief G.\n */\n";
	warns = cdlint("g.h", src, strlen(src));
	ASSERT_WARN(1, "DC3", "g.h", "no @author");
	CuAssertSizeTEquals(tc, 1, vec_len(warns));
	cdwarn_vec_free(warns);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdlint_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdlint");
	SUITE_ADD_TEST(suite, test_cdlint_decls);
	SUITE_ADD_TEST(suite, test_cdlint_file);
	return suite;
}
