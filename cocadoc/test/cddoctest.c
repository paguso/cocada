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
#include "cddoc.h"
#include "memdbg.h"
#include "new.h"


static cddoc *parse(const char *s)
{
	return cddoc_parse(s, strlen(s));
}


static const char *str_at(const vec *v, size_t i)
{
	return (const char *)vec_get_rawptr(v, i);
}


static bool has_diag(const cddoc *doc, const char *substr)
{
	for (size_t i = 0; i < vec_len(doc->diags); i++) {
		if (strstr(str_at(doc->diags, i), substr)) {
			return true;
		}
	}
	return false;
}


void test_cddoc_strip(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/**\n"
	    " * First line.\n"
	    " *\n"
	    " *  - item\n"
	    " *     indented\n"
	    " **/";
	vec *lines = cddoc_strip(src, strlen(src));
	CuAssertSizeTEquals(tc, 4, vec_len(lines));
	CuAssertStrEquals(tc, "First line.", str_at(lines, 0));
	CuAssertStrEquals(tc, "", str_at(lines, 1));
	CuAssertStrEquals(tc, " - item", str_at(lines, 2));
	CuAssertStrEquals(tc, "    indented", str_at(lines, 3));
	DESTROY(lines, finaliser_cons(FNR(vec), finaliser_new_ptr()));

	// no star column: a leading "**" is bold, not decoration
	const char *src2 = "/**< Option\n     **may** be used */";
	lines = cddoc_strip(src2, strlen(src2));
	CuAssertSizeTEquals(tc, 2, vec_len(lines));
	CuAssertStrEquals(tc, "Option", str_at(lines, 0));
	CuAssertStrEquals(tc, "**may** be used", str_at(lines, 1));
	DESTROY(lines, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_brief(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse("/**\n * @brief Pushes a value\n *        to the end.\n *\n * More text.\n */");
	CuAssertStrEquals(tc, "Pushes a value to the end.", doc->brief);
	CuAssertStrEquals(tc, "More text.", doc->details);
	cddoc_free(doc);

	// autobrief: first sentence; abbreviations do not end it
	doc = parse("/** A vector, e.g. a dynamic array. It grows.\n * Second line. */");
	CuAssertStrEquals(tc, "A vector, e.g. a dynamic array.", doc->brief);
	CuAssertStrEquals(tc, "It grows.\nSecond line.", doc->details);
	cddoc_free(doc);

	doc = parse("/**< Option MUST be used on every call */");
	CuAssertStrEquals(tc, "Option MUST be used on every call", doc->brief);
	CuAssertStrEquals(tc, "", doc->details);
	cddoc_free(doc);

	// a list is not a brief
	doc = parse("/**\n * - a\n * - b\n */");
	CuAssertStrEquals(tc, "", doc->brief);
	CuAssertTrue(tc, has_diag(doc, "no brief"));
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_params(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @brief Moves.\n"
	                 " * @param buf (**move**) The buffer\n"
	                 " *            continued.\n"
	                 " * @param name\t(*no transfer*) The name.\n"
	                 " * @param ab (move) alphabet\n"
	                 " * @param x, The x.\n"
	                 " * @param n The size, (see below).\n"
	                 " * @return The vector\n"
	                 " */");
	CuAssertSizeTEquals(tc, 5, vec_len(doc->params));
	const cdparam *p = vec_get(doc->params, 0);
	CuAssertStrEquals(tc, "buf", p->name);
	CuAssertIntEquals(tc, CDO_MOVE, p->own);
	CuAssertStrEquals(tc, "The buffer\ncontinued.", p->desc);
	p = vec_get(doc->params, 1);
	CuAssertStrEquals(tc, "name", p->name);
	CuAssertIntEquals(tc, CDO_NO_TRANSFER, p->own);
	CuAssertStrEquals(tc, "The name.", p->desc);
	p = vec_get(doc->params, 2);
	CuAssertIntEquals(tc, CDO_MOVE, p->own);
	p = vec_get(doc->params, 3);
	CuAssertStrEquals(tc, "x", p->name);
	CuAssertIntEquals(tc, CDO_UNSPECIFIED, p->own);
	p = vec_get(doc->params, 4);
	CuAssertStrEquals(tc, "The size, (see below).", p->desc);
	CuAssertStrEquals(tc, "The vector", doc->ret);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_sections(CuTest *tc)
{
	memdbg_reset();
	// block commands in mid line, as in the vec.h macro docs
	cddoc *doc = parse("/** @brief Creates a TYPE vector @see coretype.h */");
	CuAssertStrEquals(tc, "Creates a TYPE vector", doc->brief);
	CuAssertSizeTEquals(tc, 1, vec_len(doc->see));
	CuAssertStrEquals(tc, "coretype.h", str_at(doc->see, 0));
	cddoc_free(doc);

	doc = parse(
	          "/**\n"
	          " * @brief B.\n"
	          " * @warning\n"
	          " * - one\n"
	          " * - two\n"
	          " *\n"
	          " * Back in details, with @p x.\n"
	          " * @warn Second.\n"
	          " * @note A note.\n"
	          " * @deprecated\n"
	          " * @author Paulo Fonseca\n"
	          " */");
	CuAssertSizeTEquals(tc, 2, vec_len(doc->warnings));
	CuAssertStrEquals(tc, "- one\n- two", str_at(doc->warnings, 0));
	CuAssertStrEquals(tc, "Second.", str_at(doc->warnings, 1));
	CuAssertStrEquals(tc, "Back in details, with @p x.", doc->details);
	CuAssertStrEquals(tc, "A note.", str_at(doc->notes, 0));
	CuAssertStrEquals(tc, "", doc->deprecated);
	CuAssertStrEquals(tc, "Paulo Fonseca", str_at(doc->authors, 0));
	CuAssertTrue(tc, doc->ret == NULL);
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_code(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @brief B.\n"
	                 " *\n"
	                 " * Example:\n"
	                 " * @code\n"
	                 " * x = f(); // @param here is code\n"
	                 " *\n"
	                 " *     indented\n"
	                 " * @endcode\n"
	                 " * ```\n"
	                 " * @see not a command\n"
	                 " * ```\n"
	                 " * Use `@return` literally.\n"
	                 " */");
	CuAssertStrEquals(tc,
	                  "Example:\n"
	                  "```c\n"
	                  "x = f(); // @param here is code\n"
	                  "\n"
	                  "    indented\n"
	                  "```\n"
	                  "```\n"
	                  "@see not a command\n"
	                  "```\n"
	                  "Use `@return` literally.", doc->details);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->params));
	CuAssertSizeTEquals(tc, 0, vec_len(doc->see));
	CuAssertTrue(tc, doc->ret == NULL);
	cddoc_free(doc);

	doc = parse("/** @brief B.\n * @code\n * never closed */");
	CuAssertTrue(tc, has_diag(doc, "unterminated"));
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_diags(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @brief From a source string @src.\n"
	                 " * @par par The parent SOM\n"
	                 " * Mail paguso@cin.ufpe.br is fine.\n"
	                 " * @param\n"
	                 " */");
	CuAssertTrue(tc, has_diag(doc, "unknown command @src (did you mean @p src?)"));
	CuAssertTrue(tc, has_diag(doc, "did you mean @param"));
	CuAssertTrue(tc, has_diag(doc, "@param without a name"));
	CuAssertSizeTEquals(tc, 3, vec_len(doc->diags));
	cddoc_free(doc);

	// a proper @par: titled paragraph in the details, no diagnostic
	doc = parse("/**\n * @brief B.\n * @par Example\n * Some text.\n */");
	CuAssertStrEquals(tc, "**Example**\n\nSome text.", doc->details);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cddoc_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cddoc");
	SUITE_ADD_TEST(suite, test_cddoc_strip);
	SUITE_ADD_TEST(suite, test_cddoc_brief);
	SUITE_ADD_TEST(suite, test_cddoc_params);
	SUITE_ADD_TEST(suite, test_cddoc_sections);
	SUITE_ADD_TEST(suite, test_cddoc_code);
	SUITE_ADD_TEST(suite, test_cddoc_diags);
	return suite;
}
