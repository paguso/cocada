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
 * @file cddoctest.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
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


// Number of diagnostics of a rule whose message contains substr
static size_t count_diag(const cddoc *doc, const char *rule, const char *substr)
{
	size_t n = 0;
	for (size_t i = 0; i < vec_len(doc->diags); i++) {
		const cddiag *d = vec_get(doc->diags, i);
		n += strcmp(d->rule, rule) == 0 && strstr(d->msg, substr) != NULL;
	}
	return n;
}


#define ASSERT_DIAG(DOC, RULE, SUBSTR) \
	CuAssert(tc, "missing " RULE " diagnostic: " SUBSTR, count_diag(DOC, RULE, SUBSTR) == 1)


void test_cddoc_strip(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/**\n"
	    " * First line.\n"
	    " *\n"
	    " *  - item\n"
	    " *     indented\n"
	    " */";
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
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);

	// member docs: the first sentence is the brief
	doc = parse("/**< Option MUST be used on every call */");
	CuAssertStrEquals(tc, "Option MUST be used on every call", doc->brief);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);

	// no @brief: the first sentence is used, with a warning
	doc = parse("/**\n * A vector, e.g. a dynamic array. It grows.\n */");
	CuAssertStrEquals(tc, "A vector, e.g. a dynamic array.", doc->brief);
	CuAssertStrEquals(tc, "It grows.", doc->details);
	ASSERT_DIAG(doc, "DC5", "no @brief");
	cddoc_free(doc);

	doc = parse("/**\n * @brief One. Two.\n */");
	ASSERT_DIAG(doc, "DC5", "more than one sentence");
	cddoc_free(doc);

	doc = parse("/**\n * - a\n * - b\n */");
	CuAssertStrEquals(tc, "", doc->brief);
	ASSERT_DIAG(doc, "DC5", "no brief description");
	cddoc_free(doc);

	doc = parse("/**\n * @brief\n */");
	ASSERT_DIAG(doc, "DC5", "empty @brief");
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_params(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @brief Moves.\n"
	                 " * @param @move buf The buffer\n"
	                 " *            continued.\n"
	                 " * @param name The name.\n"
	                 " * @return @move The vector\n"
	                 " */");
	CuAssertSizeTEquals(tc, 2, vec_len(doc->params));
	const cdparam *p = vec_get(doc->params, 0);
	CuAssertStrEquals(tc, "buf", p->name);
	CuAssertTrue(tc, p->move);
	CuAssertStrEquals(tc, "The buffer\ncontinued.", p->desc);
	CuAssertSizeTEquals(tc, 2, p->line);
	p = vec_get(doc->params, 1);
	CuAssertStrEquals(tc, "name", p->name);
	CuAssertTrue(tc, !p->move);
	CuAssertStrEquals(tc, "The vector", doc->ret);
	CuAssertTrue(tc, doc->ret_move);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);

	// deprecated forms: accepted, with warnings
	doc = parse(
	          "/**\n"
	          " * @brief Moves.\n"
	          " * @param ab (move) alphabet\n"
	          " * @param name (**no transfer**) The name.\n"
	          " * @param (no transfer) src Source.\n"
	          " * @param x, The x.\n"
	          " * @param y @move The y.\n"
	          " * @param [in] z The z.\n"
	          " * @return (**transfer**) The result\n"
	          " */");
	CuAssertSizeTEquals(tc, 6, vec_len(doc->params));
	p = vec_get(doc->params, 0);
	CuAssertTrue(tc, p->move);
	CuAssertStrEquals(tc, "alphabet", p->desc);
	ASSERT_DIAG(doc, "DC9", "write @param @move ab instead of (move)");
	p = vec_get(doc->params, 1);
	CuAssertTrue(tc, !p->move);
	CuAssertStrEquals(tc, "The name.", p->desc);
	CuAssertSizeTEquals(tc, 2, count_diag(doc, "DC9", "not moving is the default"));
	p = vec_get(doc->params, 2);
	CuAssertStrEquals(tc, "src", p->name);
	CuAssertStrEquals(tc, "Source.", p->desc);
	p = vec_get(doc->params, 3);
	CuAssertStrEquals(tc, "x", p->name);
	ASSERT_DIAG(doc, "DC8", "punctuation after the parameter name");
	p = vec_get(doc->params, 4);
	CuAssertTrue(tc, p->move);
	CuAssertStrEquals(tc, "The y.", p->desc);
	ASSERT_DIAG(doc, "DC9", "@move goes before the parameter name");
	p = vec_get(doc->params, 5);
	CuAssertStrEquals(tc, "z", p->name);
	ASSERT_DIAG(doc, "DC8", "direction annotations");
	CuAssertTrue(tc, doc->ret_move);
	CuAssertStrEquals(tc, "The result", doc->ret);
	ASSERT_DIAG(doc, "DC9", "write @return @move instead of (**transfer**)");
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_sections(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @brief B.\n"
	                 " * @param x The x.\n"
	                 " * @warning\n"
	                 " * - one\n"
	                 " * - two\n"
	                 " *\n"
	                 " * Back in details, with @p x.\n"
	                 " * @warn Second.\n"
	                 " * @note A note.\n"
	                 " * @deprecated\n"
	                 " * @see a_sym, file.h\n"
	                 " */");
	CuAssertSizeTEquals(tc, 2, vec_len(doc->warnings));
	CuAssertStrEquals(tc, "- one\n- two", str_at(doc->warnings, 0));
	CuAssertStrEquals(tc, "Second.", str_at(doc->warnings, 1));
	CuAssertStrEquals(tc, "Back in details, with @p x.", doc->details);
	CuAssertStrEquals(tc, "A note.", str_at(doc->notes, 0));
	CuAssertStrEquals(tc, "", doc->deprecated);
	CuAssertStrEquals(tc, "a_sym, file.h", str_at(doc->see, 0));
	CuAssertTrue(tc, doc->ret == NULL);
	ASSERT_DIAG(doc, "DC7", "details after @warning");
	ASSERT_DIAG(doc, "DC6", "@warn is not allowed; use @warning");
	CuAssertSizeTEquals(tc, 2, vec_len(doc->diags));
	cddoc_free(doc);

	// block commands in mid line
	doc = parse("/**\n * @brief Creates a TYPE vector @see coretype.h\n */");
	CuAssertStrEquals(tc, "Creates a TYPE vector", doc->brief);
	CuAssertStrEquals(tc, "coretype.h", str_at(doc->see, 0));
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
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
	                 " * @see not a command, <tt>not html</tt>\n"
	                 " * ```\n"
	                 " * Use `@return` and `f()` literally.\n"
	                 " */");
	CuAssertStrEquals(tc,
	                  "Example:\n"
	                  "```c\n"
	                  "x = f(); // @param here is code\n"
	                  "\n"
	                  "    indented\n"
	                  "```\n"
	                  "```\n"
	                  "@see not a command, <tt>not html</tt>\n"
	                  "```\n"
	                  "Use `@return` and `f()` literally.", doc->details);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->params));
	CuAssertSizeTEquals(tc, 0, vec_len(doc->see));
	ASSERT_DIAG(doc, "DC12", "@code is not allowed");
	CuAssertSizeTEquals(tc, 1, vec_len(doc->diags));
	cddoc_free(doc);

	doc = parse("/**\n * @brief B.\n * ```\n * never closed\n */");
	ASSERT_DIAG(doc, "DC12", "unterminated");
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_text_rules(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @brief From a source string @src.\n"
	                 " *\n"
	                 " * See ::other and other_func() and <tt>x</tt> and cddoc::brief.\n"
	                 " * ## Heading\n"
	                 " * Ownership @move here is misplaced.\n"
	                 " * Mail paguso@cin.ufpe.br is fine.\n"
	                 " * @par par The parent SOM\n"
	                 " * @returns R\n"
	                 " * \\note Backslash form.\n"
	                 " * @see foo for details\n"
	                 " */");
	ASSERT_DIAG(doc, "DC6", "unknown command @src (did you mean @p src?)");
	ASSERT_DIAG(doc, "DC11", "::other: write #other");
	ASSERT_DIAG(doc, "DC11", "other_func(): write #other_func");
	ASSERT_DIAG(doc, "DC11", "cddoc::brief: write #cddoc.brief");
	ASSERT_DIAG(doc, "DC12", "HTML tag <tt>");
	ASSERT_DIAG(doc, "DC12", "heading outside a file comment");
	ASSERT_DIAG(doc, "DC9", "@move must come right after");
	ASSERT_DIAG(doc, "DC6", "did you mean @param or @p?");
	ASSERT_DIAG(doc, "DC6", "@returns is not allowed; use @return");
	ASSERT_DIAG(doc, "DC6", "\\note: write @note");
	ASSERT_DIAG(doc, "DC10", "found \"foo for details\"");
	// line numbers are relative to the opening /**
	for (size_t i = 0; i < vec_len(doc->diags); i++) {
		const cddiag *d = vec_get(doc->diags, i);
		if (strstr(d->msg, "@returns")) {
			CuAssertSizeTEquals(tc, 8, d->line);
		}
	}
	cddoc_free(doc);

	// headings are fine in file comments
	doc = parse("/**\n * @file x.h\n * @author A\n * @brief X.\n *\n * # Usage\n */");
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_form(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse("/** @brief One line. */");
	ASSERT_DIAG(doc, "DC1", "one-line doc comment");
	cddoc_free(doc);

	doc = parse("/** @brief Opening.\n * More.\n **/");
	ASSERT_DIAG(doc, "DC1", "text on the opening");
	ASSERT_DIAG(doc, "DC1", "close with */, not **/");
	cddoc_free(doc);

	doc = parse("/**\n * @brief Closing.\n * text */");
	ASSERT_DIAG(doc, "DC1", "text on the closing");
	cddoc_free(doc);

	doc = parse("/**< A member\n     on two lines */");
	ASSERT_DIAG(doc, "DC2", "single line");
	cddoc_free(doc);

	doc = parse("/**< A member **/");
	ASSERT_DIAG(doc, "DC1", "close with */, not **/");
	cddoc_free(doc);

	doc = parse("/**< A member @see x */");
	ASSERT_DIAG(doc, "DC2", "no block commands");
	cddoc_free(doc);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cddoc_ai(CuTest *tc)
{
	memdbg_reset();
	cddoc *doc = parse(
	                 "/**\n"
	                 " * @file x.h\n"
	                 " * @author A\n"
	                 " * @ai ai-generated, Claude (Anthropic)\n"
	                 " * @ai ai-assisted,Other AI\n"
	                 " * @brief X.\n"
	                 " */");
	CuAssertSizeTEquals(tc, 2, vec_len(doc->ai));
	const cdai *ai = vec_get(doc->ai, 0);
	CuAssertStrEquals(tc, "ai-generated", ai->level);
	CuAssertStrEquals(tc, "Claude (Anthropic)", ai->agent);
	ai = vec_get(doc->ai, 1);
	CuAssertStrEquals(tc, "ai-assisted", ai->level);
	CuAssertStrEquals(tc, "Other AI", ai->agent);
	CuAssertStrEquals(tc, "X.", doc->brief);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);

	doc = parse("/**\n * @file x.h\n * @author A\n * @ai human\n * @brief X.\n */");
	CuAssertStrEquals(tc, "", ((cdai *)vec_get(doc->ai, 0))->agent);
	CuAssertSizeTEquals(tc, 0, vec_len(doc->diags));
	cddoc_free(doc);

	doc = parse("/**\n * @file x.h\n * @author A\n * @ai ai-written, Claude\n"
	            " * @ai ai-assisted\n * @brief X.\n */");
	ASSERT_DIAG(doc, "DC15", "unknown AI level \"ai-written\"");
	ASSERT_DIAG(doc, "DC15", "@ai ai-assisted without the AI used");
	cddoc_free(doc);

	doc = parse("/**\n * @brief F.\n * @ai ai-generated, Claude (Anthropic)\n */");
	ASSERT_DIAG(doc, "DC15", "only used in file comments");
	cddoc_free(doc);

	CuAssertStrEquals(tc, "AI-generated, human-directed", cdai_level_title("ai-generated"));
	CuAssertTrue(tc, cdai_level_title("nonsense") == NULL);
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
	SUITE_ADD_TEST(suite, test_cddoc_text_rules);
	SUITE_ADD_TEST(suite, test_cddoc_form);
	SUITE_ADD_TEST(suite, test_cddoc_ai);
	return suite;
}
