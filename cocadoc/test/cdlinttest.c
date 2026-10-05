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
 * @file cdlinttest.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <string.h>

#include "CuTest.h"
#include "cdfile.h"
#include "cdlint.h"
#include "cdsym.h"
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
	cdfile *f = cdfile_new_from_str("dir/t.h", SRC, strlen(SRC));
	vec *warns = cdlint(f, NULL);
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
	cdfile_free(f);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cdlint_file(CuTest *tc)
{
	memdbg_reset();
	cdfile *f = cdfile_new_from_str("other.h", SRC, strlen(SRC));
	vec *warns = cdlint(f, NULL);
	ASSERT_WARN(3, "DC3", "other.h", "@file t.h does not match the file name other.h");
	cdwarn_vec_free(warns);
	cdfile_free(f);

	const char *src = "/**\n * @brief F.\n */\nvoid f(void);\n";
	f = cdfile_new_from_str("f.h", src, strlen(src));
	warns = cdlint(f, NULL);
	ASSERT_WARN(1, "DC3", "f.h", "no file comment");
	CuAssertSizeTEquals(tc, 1, vec_len(warns));
	cdwarn_vec_free(warns);
	cdfile_free(f);

	src = "/**\n * @file g.h\n * @brief G.\n */\n";
	f = cdfile_new_from_str("g.h", src, strlen(src));
	warns = cdlint(f, NULL);
	ASSERT_WARN(1, "DC3", "g.h", "no @author");
	CuAssertSizeTEquals(tc, 1, vec_len(warns));
	cdwarn_vec_free(warns);
	cdfile_free(f);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


static const char *SRC_A =
    "/**\n"                                      // 1
    " * @file a.h\n"                             // 2
    " * @author A\n"                             // 3
    " * @brief A.\n"                             // 4
    " *\n"                                       // 5
    " * Uses #b_func, #b_t.x, #B_ONE and #missing.\n" // 6
    " */\n"                                      // 7
    "/**\n"                                      // 8
    " * @brief Does @p n things to @p m.\n"      // 9
    " * @param n The n.\n"                       // 10
    " * @see b_func, b.h, nothing, #shared\n"    // 11
    " */\n"                                      // 12
    "void a_func(int n);\n"                      // 13
    "/**\n"                                      // 14
    " * @brief Shared, like #shared.\n"           // 15
    " */\n"                                      // 16
    "void shared(void);\n";                      // 17

static const char *SRC_B =
    "/**\n * @file b.h\n * @author B\n * @brief B.\n */\n"
    "/**\n * @brief B func.\n */\nvoid b_func(void);\n"
    "/**\n * @brief B type.\n */\ntypedef struct {\n\tint x; /**< The x */\n} b_t;\n"
    "/**\n * @brief B enum.\n */\ntypedef enum {\n\tB_ONE /**< One */\n} b_e;\n"
    "/**\n * @brief Shared.\n */\nvoid shared(void);\n";


void test_cdlint_refs(CuTest *tc)
{
	memdbg_reset();
	cdfile *a = cdfile_new_from_str("dir/a.h", SRC_A, strlen(SRC_A));
	cdfile *b = cdfile_new_from_str("b.h", SRC_B, strlen(SRC_B));
	vec *files = vec_new(sizeof(cdfile *));
	vec_push_rawptr(files, a);
	vec_push_rawptr(files, b);
	cdsymtab *tab = cdsymtab_new(files);

	vec *warns = cdlint(a, tab);
	ASSERT_WARN(6, "DC11", "a.h", "unknown reference #missing");
	ASSERT_WARN(9, "DC11", "a_func", "@p m is not a parameter of a_func");
	ASSERT_WARN(11, "DC10", "a_func", "unknown @see nothing");
	// #shared and @see shared from a.h resolve to a.h's own shared: no warning
	CuAssertSizeTEquals(tc, 3, vec_len(warns));
	cdwarn_vec_free(warns);

	// from b.h, shared is declared in b.h too: still not ambiguous
	warns = cdlint(b, tab);
	CuAssertSizeTEquals(tc, 0, vec_len(warns));
	cdwarn_vec_free(warns);

	cdsymtab_free(tab);
	DESTROY_FLAT(files, vec);
	cdfile_free(a);
	cdfile_free(b);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


static const char *SRC_GEN =
    "/**\n * @file g.h\n * @author G\n * @brief G.\n */\n"
    "/**\n * @brief Pushes @p vall.\n * @param v The vec.\n */\n"                // 6-9
    "#define DECL_PUSH(TYPE) void push_##TYPE(vec *v, TYPE val);\n"            // 10
    "/**\n * @brief Not used.\n */\n"                                        // 11-13
    "#define DECL_ALL(TYPE) DECL_PUSH(TYPE) DECL_POP(TYPE)\n"                  // 14
    "#define DECL_POP(TYPE) TYPE pop_##TYPE(vec *v);\n"                        // 15
    "/**\n * @brief Not used either.\n */\n"                                 // 16-18
    "#define DECL_TWO(TYPE) int one_##TYPE(void); int two_##TYPE(void);\n"    // 19
    "#define XX_TWO(XX, ...) XX(int, __VA_ARGS__) XX(char, __VA_ARGS__)\n"   // 20
    "XX_TWO(DECL_ALL)\n"                                                      // 21
    "/**\n * @brief Not used: two instances.\n */\n"                         // 22-24
    "XX_TWO(DECL_TWO)\n"                                                      // 25
    "NOT_A_MACRO(x)\n";                                                       // 26


void test_cdlint_generators(CuTest *tc)
{
	memdbg_reset();
	cdfile *g = cdfile_new_from_str("g.h", SRC_GEN, strlen(SRC_GEN));
	vec *files = vec_new(sizeof(cdfile *));
	vec_push_rawptr(files, g);
	cdsymtab *tab = cdsymtab_new(files);
	vec *warns = cdlint(g, tab);
	// the doc of DECL_PUSH is checked against void push_TYPE(vec *v, TYPE val)
	ASSERT_WARN(6, "DC8", "push_TYPE", "parameter val is not documented");
	ASSERT_WARN(7, "DC11", "push_TYPE", "@p vall is not a parameter of push_TYPE");
	// docs that are not used
	ASSERT_WARN(11, "DC14", "DECL_ALL", "only invokes other generators");
	ASSERT_WARN(16, "DC14", "DECL_TWO", "declares 2 things");
	ASSERT_WARN(22, "DC14", "XX_TWO", "only if it generates a single declaration");
	// undocumented families, once each
	ASSERT_WARN(21, "DC4", "pop_TYPE", "undocumented macro-generated function (generated by DECL_POP");
	ASSERT_WARN(25, "DC4", "one_TYPE", "undocumented macro-generated function");
	ASSERT_WARN(25, "DC4", "two_TYPE", "undocumented macro-generated function");
	ASSERT_WARN(26, "DC14", "g.h", "cannot expand NOT_A_MACRO(x)");
	// no DC4 for the generators themselves; a type list is a public macro
	ASSERT_WARN(20, "DC4", "XX_TWO", "undocumented macro");
	for (size_t i = 0; i < vec_len(warns); i++) {
		const cdwarn *w = vec_get(warns, i);
		CuAssert(tc, w->msg, !(strcmp(w->rule, "DC4") == 0 && strncmp(w->name, "DECL_", 5) == 0));
	}
	CuAssertSizeTEquals(tc, 10, vec_len(warns));
	cdwarn_vec_free(warns);
	cdsymtab_free(tab);
	DESTROY_FLAT(files, vec);
	cdfile_free(g);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdlint_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdlint");
	SUITE_ADD_TEST(suite, test_cdlint_decls);
	SUITE_ADD_TEST(suite, test_cdlint_file);
	SUITE_ADD_TEST(suite, test_cdlint_refs);
	SUITE_ADD_TEST(suite, test_cdlint_generators);
	return suite;
}
