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
 * @file cddecltest.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <string.h>

#include "CuTest.h"
#include "cddecl.h"
#include "cdlexer.h"
#include "memdbg.h"
#include "new.h"


typedef struct {
	cddecl_kind kind;
	const char *name;
	const char *sig;
	bool documented;
} exp_decl;


static void check_decls(CuTest *tc, size_t n, exp_decl *exp,
                        const vec *decls)
{
	CuAssertSizeTEquals(tc, n, vec_len(decls));
	for (size_t i = 0; i < n && i < vec_len(decls); i++) {
		const cddecl *d = vec_get(decls, i);
		CuAssertStrEquals(tc, cddecl_kind_name(exp[i].kind), cddecl_kind_name(d->kind));
		CuAssertStrEquals(tc, exp[i].name, d->name);
		if (exp[i].sig) {
			CuAssertStrEquals(tc, exp[i].sig, d->sig);
		}
		CuAssertTrue(tc, exp[i].documented == (d->doc != NULL));
	}
}


// Runs the matcher on src and checks the top-level declarations.
#define CHECK(SRC, EXP) \
	do { \
		vec *toks = cdlex_all(SRC, strlen(SRC)); \
		vec *decls = cddecl_match(SRC, toks); \
		check_decls(tc, sizeof(EXP) / sizeof(EXP[0]), EXP, decls); \
		cddecl_vec_free(decls); \
		DESTROY_FLAT(toks, vec); \
	} while (0)


void test_cddecl_func(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/** @file foo.h */\n"
	    "/** a */\n"
	    "vec  *vec_new( size_t\n   typesize );\n"
	    "int undocumented(void);\n"
	    "/** b */\n"
	    "struct node *node_next(const struct node *n, ...);\n"
	    "/** c */\n"
	    "static inline int sq(int x) { if (x) { return x*x; } return 0; }\n"
	    "/** d */\n"
	    "const char **f(char a[], int (*cmp)(const void *, const void *));\n";
	exp_decl exp[] = {
		{CDD_FILE, "foo.h", "", true},
		{CDD_FUNC, "vec_new", "vec *vec_new(size_t typesize)", true},
		{CDD_FUNC, "undocumented", "int undocumented(void)", false},
		{CDD_FUNC, "node_next", "struct node *node_next(const struct node *n, ...)", true},
		{CDD_FUNC, "sq", "static inline int sq(int x)", true},
		{CDD_FUNC, "f", "const char **f(char a[], int (*cmp)(const void *, const void *))", true},
	};
	CHECK(src, exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddecl_types(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/** a */\n"
	    "typedef struct _vec vec;\n"
	    "/** b */\n"
	    "typedef uint64_t (*hash_func)(const void *);\n"
	    "/** c */\n"
	    "struct _iter {\n"
	    "\t/** the vtable */\n"
	    "\titer_vt *vt;\n"
	    "\tint flags : 3; /**< bits */\n"
	    "\tchar buf[16];\n"
	    "\tvoid (*cb)(int);\n"
	    "};\n"
	    "/** d */\n"
	    "typedef enum {\n"
	    "\tA = -1, /**< first */\n"
	    "\tB,\n"
	    "\tC /**< last */\n"
	    "} letters;\n"
	    "/** e */\n"
	    "static const size_t CAP = 1024;\n"
	    "/** f */\n"
	    "struct fwd;\n";
	exp_decl exp[] = {
		{CDD_TYPEDEF, "vec", "typedef struct _vec vec", true},
		{CDD_TYPEDEF, "hash_func", "typedef uint64_t (*hash_func)(const void *)", true},
		{CDD_STRUCT, "_iter", "struct _iter {...}", true},
		{CDD_TYPEDEF, "letters", "typedef enum {...} letters", true},
		{CDD_VAR, "CAP", "static const size_t CAP = 1024", true},
		{CDD_STRUCT, "fwd", "struct fwd", true},
	};
	vec *toks = cdlex_all(src, strlen(src));
	vec *decls = cddecl_match(src, toks);
	check_decls(tc, sizeof(exp) / sizeof(exp[0]), exp, decls);

	const cddecl *it = vec_get(decls, 2);
	exp_decl itm[] = {
		{CDD_MEMBER, "vt", "iter_vt *vt", true},
		{CDD_MEMBER, "flags", "int flags : 3", true},
		{CDD_MEMBER, "buf", "char buf[16]", false},
		{CDD_MEMBER, "cb", "void (*cb)(int)", false},
	};
	check_decls(tc, 4, itm, it->members);
	CuAssertStrEquals(tc, "/**< bits */", ((cddecl *)vec_get(it->members, 1))->doc);

	const cddecl *en = vec_get(decls, 3);
	exp_decl enm[] = {
		{CDD_MEMBER, "A", "A = -1", true},
		{CDD_MEMBER, "B", "B", false},
		{CDD_MEMBER, "C", "C", true},
	};
	check_decls(tc, 3, enm, en->members);
	CuAssertStrEquals(tc, "/**< last */", ((cddecl *)vec_get(en->members, 2))->doc);

	cddecl_vec_free(decls);
	DESTROY_FLAT(toks, vec);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddecl_macros(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/** a */\n"
	    "#define ARGNO_UNLIMITED   INT_MAX /* max */\n"
	    "/** b */\n"
	    "#define FREE( OBJ ) \\\n"
	    "\tif((OBJ)) free((void *)(OBJ))\n"
	    "/** c */\n"
	    "#ifndef NULL\n"
	    "#define NULL ((void *)0)\n"
	    "#endif\n"
	    "/** d */\n"
	    "DECL_TRAIT(vec_iter, iter)\n"
	    "/** e */\n"
	    "XX_CORETYPES(DECL_TYPED_VEC);\n"
	    "/** f */\n"
	    "#include \"x.h\"\n";
	exp_decl exp[] = {
		{CDD_MACRO, "ARGNO_UNLIMITED", "#define ARGNO_UNLIMITED INT_MAX", true},
		{CDD_MACRO, "FREE", "#define FREE( OBJ )", true},
		{CDD_MACRO, "NULL", "#define NULL ((void *)0)", true},
		{CDD_MACROCALL, "DECL_TRAIT", "DECL_TRAIT(vec_iter, iter)", true},
		{CDD_MACROCALL, "XX_CORETYPES", "XX_CORETYPES(DECL_TYPED_VEC)", true},
	};
	CHECK(src, exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddecl_orphans(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/** superseded */\n"
	    "/** a */\n"
	    "int f(void);\n"
	    "void g(void) {\n"
	    "\t/** inside body */\n"
	    "\tint x;\n"
	    "\t/** before close */\n"
	    "}\n"
	    "/** at end */\n";
	exp_decl exp[] = {
		{CDD_FUNC, "f", "int f(void)", true},
		{CDD_FUNC, "g", "void g(void)", false},
	};
	CHECK(src, exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddecl_undocumented(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "#ifndef T_H\n"
	    "#define T_H\n"
	    "#include \"x.h\"\n"
	    "#define MAX(A, B) ((A) > (B) ? (A) : (B))\n"
	    "XX_CORETYPES(DECL_TYPED_VEC)\n"
	    "static const byte_t _MASK[2] = { 0x80, 0x40 };\n"
	    "/** a */\n"
	    "DECL_TRAIT(vec_iter, iter)\n"
	    "#endif\n";
	exp_decl exp[] = {
		{CDD_MACRO, "MAX", "#define MAX(A, B)", false},
		{CDD_VAR, "_MASK", "static const byte_t _MASK[2] = {...}", false},
		{CDD_MACROCALL, "DECL_TRAIT", "DECL_TRAIT(vec_iter, iter)", true},
	};
	CHECK(src, exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddecl_is_generator(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "#define DECL_VEC_PUSH(TYPE) void vec_push_##TYPE(vec *v, TYPE val);\n"
	    "#define IMPL_VEC_PUSH(TYPE) void vec_push_##TYPE(vec *v, TYPE val) {}\n"
	    "#define VEC_PUSH_DECL(TYPE) void vec_push_##TYPE(vec *v, TYPE val);\n"
	    "#define MAX(A, B) ((A) > (B) ? (A) : (B))\n"
	    "int DECL_func(void);\n";
	vec *toks = cdlex_all(src, strlen(src));
	vec *decls = cddecl_match(src, toks);
	CuAssertSizeTEquals(tc, 5, vec_len(decls));
	bool expected[] = {true, true, false, false, false}; // prefix, and only macros
	for (size_t i = 0; i < 5; i++) {
		CuAssertTrue(tc, cddecl_is_generator(vec_get(decls, i)) == expected[i]);
	}
	cddecl_vec_free(decls);
	DESTROY_FLAT(toks, vec);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cddecl_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cddecl");
	SUITE_ADD_TEST(suite, test_cddecl_func);
	SUITE_ADD_TEST(suite, test_cddecl_types);
	SUITE_ADD_TEST(suite, test_cddecl_macros);
	SUITE_ADD_TEST(suite, test_cddecl_orphans);
	SUITE_ADD_TEST(suite, test_cddecl_undocumented);
	SUITE_ADD_TEST(suite, test_cddecl_is_generator);
	return suite;
}
