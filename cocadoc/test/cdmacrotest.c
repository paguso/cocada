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
 * @file cdmacrotest.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <string.h>

#include "CuTest.h"
#include "cddecl.h"
#include "cdfile.h"
#include "cdmacro.h"
#include "memdbg.h"
#include "new.h"


static const char *SRC_LIB =
    "#define XX_LIST(XX, ...) \\\n"
    "\tXX(int, __VA_ARGS__) \\\n"
    "\tXX(double, __VA_ARGS__)\n"
    "/**\n * @brief Declares a trait.\n */\n"
    "#define DECL_TRAIT(TYPE, TRAIT)\\\n"
    "\t/** @brief The TRAIT trait of a TYPE. */\\\n"
    "\tTRAIT * TYPE##_as_##TRAIT( TYPE *self );\n"
    "#define DECL_RESULT_OK(NAME, T) \\\n"
    "\ttypedef struct {\\\n\t\tbool ok; /**< Success */\\\n\t\tT val; /**< The T value */\\\n"
    "\t} NAME##_res;\n";

static const char *SRC_VEC =
    "/**\n * @brief Declares vec_push_TYPE.\n */\n"
    "#define DECL_VEC_PUSH( TYPE ) \\\n"
    "\t/**\\\n"
    "\t * @brief Appends a TYPE copy of @p val.\\\n"
    "\t */\\\n"
    "\tvoid vec_push_##TYPE(vec *v, TYPE val);\n"
    "#define DECL_VEC_GET( TYPE ) \\\n"
    "\tTYPE vec_get_##TYPE(const vec *v, size_t pos);\n"
    "/**\n * @brief Not used: composite.\n */\n"
    "#define DECL_TYPED_VEC( TYPE , ...) \\\n"
    "\tDECL_VEC_PUSH(TYPE) \\\n"
    "\tDECL_VEC_GET(TYPE)\n"
    "#define DECL_TWO(TYPE) int one_##TYPE(void); /** @hide */ int two_##TYPE(void);\n"
    "#define REC(X) REC(X)\n"
    "XX_LIST(DECL_TYPED_VEC)\n"
    "XX_LIST(DECL_TWO)\n"
    "/**\n * @brief The iterator as an iter.\n */\n"
    "DECL_TRAIT(vec_iter, iter);\n"
    "DECL_RESULT_OK(semver, semver *);\n"
    "UNKNOWN_MACRO(x)\n"
    "REC(1)\n";


static const char *inst_name(const cdfamily *fam, size_t i)
{
	return ((const cdinstance *)vec_get(fam->instances, i))->decl->name;
}


void test_cdmacro_families(CuTest *tc)
{
	memdbg_reset();
	cdfile *lib = cdfile_new_from_str("lib.h", SRC_LIB, strlen(SRC_LIB));
	cdfile *vf = cdfile_new_from_str("vec.h", SRC_VEC, strlen(SRC_VEC));
	vec *files = vec_new(sizeof(cdfile *));
	vec_push_rawptr(files, lib);
	vec_push_rawptr(files, vf);
	cdmacrotab *tab = cdmacrotab_new(files);

	vec *warns = vec_new(sizeof(cdmacrowarn));
	vec *fams = cdmacro_families(tab, vf, warns);
	// vec_push, vec_get, one, two, as_trait, result
	CuAssertSizeTEquals(tc, 6, vec_len(fams));

	const cdfamily *push = vec_get(fams, 0);
	CuAssertStrEquals(tc, "DECL_VEC_PUSH", push->gen->name);
	CuAssertStrEquals(tc, "vec_push_TYPE", push->pattern->name);
	CuAssertStrEquals(tc, "void vec_push_TYPE(vec *v, TYPE val)", push->pattern->sig);
	CuAssertStrEquals(tc, "vec_push_TYPE", cdfamily_name(push));
	CuAssertSizeTEquals(tc, 2, vec_len(push->instances));
	CuAssertStrEquals(tc, "vec_push_int", inst_name(push, 0));
	CuAssertStrEquals(tc, "vec_push_double", inst_name(push, 1));
	CuAssertStrEquals(tc, "int", ((const cdinstance *)vec_get(push->instances, 0))->args);
	CuAssertStrEquals(tc, "XX_LIST(DECL_TYPED_VEC)", (char *)vec_get_rawptr(push->via, 0));
	// documented by the doc in the body, not by the one above the #define
	CuAssertTrue(tc, push->doc_decl == push->pattern);
	CuAssertStrEquals(tc, "/**\n\t * @brief Appends a TYPE copy of @p val.\n\t */",
	                  push->doc_decl->doc);
	CuAssertTrue(tc, ((const cdinstance *)vec_get(push->instances, 0))->decl->doc != NULL);
	CuAssertSizeTEquals(tc, 5, cdmacro_doc_line(tab, push->gen, push->doc_decl->doc));
	CuAssertSizeTEquals(tc, 4, cdmacro_doc_line(tab, push->gen, "/** other */"));
	CuAssertTrue(tc, !push->hidden);

	const cdfamily *get = vec_get(fams, 1);
	CuAssertStrEquals(tc, "vec_get_TYPE", get->pattern->name);
	CuAssertTrue(tc, get->doc_decl == NULL); // undocumented

	const cdfamily *one = vec_get(fams, 2);
	CuAssertStrEquals(tc, "one_TYPE", one->pattern->name);
	CuAssertTrue(tc, one->doc_decl == NULL);
	const cdfamily *two = vec_get(fams, 3);
	CuAssertStrEquals(tc, "two_TYPE", two->pattern->name);
	CuAssertTrue(tc, two->doc_decl == two->pattern);
	CuAssertTrue(tc, two->hidden);

	const cdfamily *trait = vec_get(fams, 4);
	CuAssertStrEquals(tc, "TYPE_as_TRAIT", trait->pattern->name);
	CuAssertStrEquals(tc, "vec_iter_as_iter", cdfamily_name(trait)); // single instance
	CuAssertStrEquals(tc, "iter *vec_iter_as_iter(vec_iter *self)",
	                  ((const cdinstance *)vec_get(trait->instances, 0))->decl->sig);
	CuAssertTrue(tc, trait->gen_file == lib);
	// a single instance has its own doc, with the parameters replaced
	const cddecl *tinst = ((const cdinstance *)vec_get(trait->instances, 0))->decl;
	CuAssertTrue(tc, trait->doc_decl == tinst);
	CuAssertStrEquals(tc, "/** @brief The iter trait of a vec_iter. */", tinst->doc);
	CuAssertStrEquals(tc, "/** @brief The TRAIT trait of a TYPE. */", trait->pattern->doc);

	const cdfamily *res = vec_get(fams, 5);
	CuAssertStrEquals(tc, "semver_res", cdfamily_name(res));
	CuAssertIntEquals(tc, CDD_TYPEDEF, res->pattern->kind);
	CuAssertStrEquals(tc, "NAME_res", res->pattern->name);
	// trailing member docs in the macro body are kept
	const cddecl *inst = ((const cdinstance *)vec_get(res->instances, 0))->decl;
	CuAssertSizeTEquals(tc, 2, vec_len(inst->members));
	CuAssertStrEquals(tc, "/**< Success */", ((cddecl *)vec_get(inst->members, 0))->doc);
	CuAssertStrEquals(tc, "/**< The semver * value */", ((cddecl *)vec_get(inst->members, 1))->doc);
	CuAssertStrEquals(tc, "/**< The T value */", ((cddecl *)vec_get(res->pattern->members, 1))->doc);

	// generators
	const vec *pats;
	CuAssertIntEquals(tc, CDG_COMPOSITE, cdmacro_generator(tab, vec_get(vf->decls, 2), &pats));
	CuAssertTrue(tc, pats == NULL);
	CuAssertIntEquals(tc, CDG_LEAF, cdmacro_generator(tab, vec_get(vf->decls, 0), &pats));
	CuAssertSizeTEquals(tc, 1, vec_len(pats));
	// a type list declares nothing by itself
	CuAssertIntEquals(tc, CDG_NONE, cdmacro_generator(tab, vec_get(lib->decls, 0), NULL));

	// UNKNOWN_MACRO is reported; REC stops
	CuAssertSizeTEquals(tc, 1, vec_len(warns));
	CuAssert(tc, ((cdmacrowarn *)vec_get(warns, 0))->msg,
	         strstr(((cdmacrowarn *)vec_get(warns, 0))->msg, "UNKNOWN_MACRO") != NULL);

	cdfamily_vec_free(fams);
	cdmacrowarn_vec_free(warns);
	cdmacrotab_free(tab);
	DESTROY_FLAT(files, vec);
	cdfile_free(lib);
	cdfile_free(vf);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdmacro_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdmacro");
	SUITE_ADD_TEST(suite, test_cdmacro_families);
	return suite;
}
