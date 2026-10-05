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
#include "cdfile.h"
#include "cdmd.h"
#include "cdsym.h"
#include "memdbg.h"
#include "new.h"


#define ASSERT_HAS(PAGE, TEXT) \
	CuAssert(tc, "page lacks: " TEXT, strstr(PAGE, TEXT) != NULL)
#define ASSERT_LACKS(PAGE, TEXT) \
	CuAssert(tc, "page has: " TEXT, strstr(PAGE, TEXT) == NULL)


static const char *SRC_A =
    "/**\n"
    " * @file a.h\n"
    " * @author A\n"
    " * @brief Module A.\n"
    " *\n"
    " * # Usage\n"
    " * Uses #b_func, #b_t.x, ::b_func, b.h and #nothing.\n"
    " */\n"
    "/**\n"
    " * @brief A point.\n"
    " */\n"
    "typedef struct {\n"
    "\tint x; /**< The x <b>coordinate</b> */\n"
    "\tint y;\n"
    "} point;\n"
    "/**\n"
    " * @brief Moves @p p into #point, if j<k and <id>.\n"
    " * @param @move p The point.\n"
    " * @return @move The copy.\n"
    " * @warning Careful\n"
    " * - really\n"
    " * @see b_func, b.h\n"
    " */\n"
    "point *take(point *p, int n);\n"
    "void undoc(void);\n"
    "#define TWICE(X) (2 * (X))\n";

static const char *SRC_B =
    "/**\n * @file b.h\n * @author B\n * @brief Module B.\n */\n"
    "/**\n * @brief B func.\n */\nvoid b_func(void);\n"
    "/**\n * @brief B type.\n */\ntypedef struct {\n\tint x; /**< The x */\n} b_t;\n";


void test_cdmd_page(CuTest *tc)
{
	memdbg_reset();
	cdfile *a = cdfile_new_from_str("lib/a.h", SRC_A, strlen(SRC_A));
	cdfile *b = cdfile_new_from_str("lib/sub/b.h", SRC_B, strlen(SRC_B));
	vec *files = vec_new(sizeof(cdfile *));
	vec_push_rawptr(files, a);
	vec_push_rawptr(files, b);
	cdsymtab *tab = cdsymtab_new(files);

	char *name = cdmd_page_name(a);
	CuAssertStrEquals(tc, "a.md", name);
	FREE(name);

	char *page = cdmd_page(a, tab);
	// module
	ASSERT_HAS(page, "# a.h\n\nModule A.\n\n## Usage\n");
	ASSERT_HAS(page, "**Author:** A");
	// references
	ASSERT_HAS(page, "Uses [b_func](b.md#b_func), [b_t.x](b.md#b_t), [b_func](b.md#b_func), "
	           "[b.h](b.md) and `nothing`.");
	// sections, in order
	CuAssertTrue(tc, strstr(page, "## Types and constants") < strstr(page, "## Functions"));
	CuAssertTrue(tc, strstr(page, "## Functions") < strstr(page, "## Macros"));
	// struct with members
	ASSERT_HAS(page, "### point\n\n```c\ntypedef struct {\n\tint x;\n\tint y;\n} point;\n```");
	ASSERT_HAS(page, "- `x`: The x **coordinate**\n- `y`: *undocumented*");
	// function: @p, local link, escaping, params, moved return
	ASSERT_HAS(page, "Moves `p` into [point](#point), if j\\<k and \\<id>.");
	ASSERT_HAS(page, "- `p` **(moved)**: The point.\n- `n`: *undocumented*");
	ASSERT_HAS(page, "**Returns** (moved to the caller): The copy.");
	ASSERT_HAS(page, "> **Warning:** Careful\n> - really");
	ASSERT_HAS(page, "**See also:** [b_func](b.md#b_func), [b.h](b.md)");
	// undocumented function and macro
	ASSERT_HAS(page, "### undoc\n\n```c\nvoid undoc(void);\n```\n\n*Undocumented.*");
	// (an item with no comment at all just says so; no parameter list)
	ASSERT_HAS(page, "### TWICE\n\n```c\n#define TWICE(X)\n```\n\n*Undocumented.*");
	ASSERT_LACKS(page, "**Returns:** *undocumented*\n\n### TWICE"); // void: no return
	FREE(page);

	char *index = cdmd_index(files);
	ASSERT_HAS(index, "## lib\n\n- [a.h](a.md): Module A.\n");
	ASSERT_HAS(index, "## lib/sub\n\n- [b.h](b.md): Module B.\n");
	FREE(index);

	cdsymtab_free(tab);
	DESTROY_FLAT(files, vec);
	cdfile_free(a);
	cdfile_free(b);
	CuAssert(tc, "Memory leak.", memdbg_is_empty());
}


CuSuite *cdmd_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cdmd");
	SUITE_ADD_TEST(suite, test_cdmd_page);
	return suite;
}
