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
 * @file cdlexertest.c
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 */

#include <string.h>

#include "CuTest.h"
#include "cdlexer.h"
#include "memdbg.h"
#include "new.h"


typedef struct {
	cdtoken_type type;
	const char *text;
	size_t line;
} exp_tok;


static void check_tokens(CuTest *tc, const char *src, size_t n, exp_tok *exp)
{
	vec *toks = cdlex_all(src, strlen(src));
	CuAssertSizeTEquals(tc, n, vec_len(toks));
	for (size_t i = 0; i < n && i < vec_len(toks); i++) {
		const cdtoken *tk = vec_get(toks, i);
		CuAssertStrEquals(tc, cdtoken_type_name(exp[i].type),
		                  cdtoken_type_name(tk->type));
		CuAssertSizeTEquals(tc, strlen(exp[i].text), tk->len);
		CuAssert(tc, exp[i].text, strncmp(exp[i].text, src + tk->pos, tk->len) == 0);
		CuAssertSizeTEquals(tc, exp[i].line, tk->line);
	}
	DESTROY_FLAT(toks, vec);
}


void test_cdlexer_decl(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/**\n"
	    " * @brief Push.\n"
	    " */\n"
	    "void vec_push(vec *v, const void *src);\n";
	exp_tok exp[] = {
		{CDT_DOC, "/**\n * @brief Push.\n */", 1},
		{CDT_IDENT, "void", 4},
		{CDT_IDENT, "vec_push", 4},
		{CDT_PUNCT, "(", 4},
		{CDT_IDENT, "vec", 4},
		{CDT_PUNCT, "*", 4},
		{CDT_IDENT, "v", 4},
		{CDT_PUNCT, ",", 4},
		{CDT_IDENT, "const", 4},
		{CDT_IDENT, "void", 4},
		{CDT_PUNCT, "*", 4},
		{CDT_IDENT, "src", 4},
		{CDT_PUNCT, ")", 4},
		{CDT_PUNCT, ";", 4},
	};
	check_tokens(tc, src, sizeof(exp) / sizeof(exp[0]), exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cdlexer_comments(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "/* plain */ a // line /** not doc */\n"
	    "/**/ b /***********/ c\n"
	    "d /**< post */\n";
	exp_tok exp[] = {
		{CDT_IDENT, "a", 1},
		{CDT_IDENT, "b", 2},
		{CDT_IDENT, "c", 2},
		{CDT_IDENT, "d", 3},
		{CDT_DOC_POST, "/**< post */", 3},
	};
	check_tokens(tc, src, sizeof(exp) / sizeof(exp[0]), exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cdlexer_pp(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "#include \"a.h\"\n"
	    "  #define F( X ) \\\n"
	    "    g(X) /* c\n"
	    "    c */\n"
	    "x = a # b;\n";
	exp_tok exp[] = {
		{CDT_PP, "#include \"a.h\"", 1},
		{CDT_PP, "#define F( X ) \\\n    g(X) /* c\n    c */", 2},
		{CDT_IDENT, "x", 5},
		{CDT_PUNCT, "=", 5},
		{CDT_IDENT, "a", 5},
		{CDT_PUNCT, "#", 5},
		{CDT_IDENT, "b", 5},
		{CDT_PUNCT, ";", 5},
	};
	check_tokens(tc, src, sizeof(exp) / sizeof(exp[0]), exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cdlexer_literals(CuTest *tc)
{
	memdbg_reset();
	const char *src =
	    "s = \"a \\\" /* not comment */\"; c = '\\''; L\"w\";\n"
	    "1.5e-3f 0x1F 42ULL .5\n";
	exp_tok exp[] = {
		{CDT_IDENT, "s", 1},
		{CDT_PUNCT, "=", 1},
		{CDT_STRING, "\"a \\\" /* not comment */\"", 1},
		{CDT_PUNCT, ";", 1},
		{CDT_IDENT, "c", 1},
		{CDT_PUNCT, "=", 1},
		{CDT_CHAR, "'\\''", 1},
		{CDT_PUNCT, ";", 1},
		{CDT_STRING, "L\"w\"", 1},
		{CDT_PUNCT, ";", 1},
		{CDT_NUMBER, "1.5e-3f", 2},
		{CDT_NUMBER, "0x1F", 2},
		{CDT_NUMBER, "42ULL", 2},
		{CDT_NUMBER, ".5", 2},
	};
	check_tokens(tc, src, sizeof(exp) / sizeof(exp[0]), exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


void test_cdlexer_unterminated(CuTest *tc)
{
	memdbg_reset();
	const char *src = "a /** never closed";
	exp_tok exp[] = {
		{CDT_IDENT, "a", 1},
		{CDT_DOC, "/** never closed", 1},
	};
	check_tokens(tc, src, sizeof(exp) / sizeof(exp[0]), exp);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdlexer_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdlexer");
	SUITE_ADD_TEST(suite, test_cdlexer_decl);
	SUITE_ADD_TEST(suite, test_cdlexer_comments);
	SUITE_ADD_TEST(suite, test_cdlexer_pp);
	SUITE_ADD_TEST(suite, test_cdlexer_literals);
	SUITE_ADD_TEST(suite, test_cdlexer_unterminated);
	return suite;
}
