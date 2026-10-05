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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "arrays.h"
#include "cli.h"
#include "cstrutil.h"
#include "vec.h"
#include "new.h"
#include "errlog.h"

static cliparser *cmd;


static void test_setup()
{
	cmd = cliparser_new("test", "A Test Program");
	char choice_arr[3][8]  = {"choice1", "choice2", "choice3"};
	vec *choices = vec_new(sizeof(char *));
	for (size_t i = 0; i < 3;
	        vec_push_rawptr(choices, cstr_clone(choice_arr[i++])));
	cliparser_add_option(cmd,
	                     cliopt_new_defaults(
	                         'a',
	                         "aaa",
	                         "optional with no value"
	                     )
	                    );
	cliparser_add_option(cmd,
	                     cliopt_new(
	                         'b',
	                         "bbb",
	                         "non-mandatory single option with one boolean value",
	                         OPT_OPTIONAL, OPT_SINGLE, ARG_BOOL, 1, 1, NULL, NULL
	                     )
	                    );
	vec *def = vec_new_long();
	vec_push_long(def, 1234);
	vec_push_long(def, 4321);
	vec_push_long(def, 2143);
	cliparser_add_option(cmd,
	                     cliopt_new(
	                         'c',
	                         "ccc",
	                         "non-mandatory multiple option with two to five int values",
	                         OPT_OPTIONAL, OPT_MULTIPLE, ARG_INT, 2, 5, NULL, def
	                     )
	                    );
	cliparser_add_option(cmd,
	                     cliopt_new(
	                         'd',
	                         "ddd",
	                         "mandatory single option with one string value",
	                         OPT_REQUIRED, OPT_SINGLE, ARG_STR, 1, 1, NULL, NULL
	                     )
	                    );
	cliparser_add_option(cmd,
	                     cliopt_new(
	                         'e',
	                         "eee",
	                         "non-mandatory multiple option with one tp three file values",
	                         OPT_OPTIONAL, OPT_MULTIPLE, ARG_FILE, 1, 3, NULL, NULL
	                     )
	                    );
	cliparser_add_option(cmd,
	                     cliopt_new(
	                         'f',
	                         "fff",
	                         "non-mandatory single option with unlimited float values",
	                         OPT_OPTIONAL, OPT_SINGLE, ARG_FLOAT, 1, ARGNO_UNLIMITED, NULL, NULL
	                     )
	                    );
	cliparser_add_option(cmd,
	                     cliopt_new(
	                         'g',
	                         "ggg",
	                         "non-mandatory single option three or more  values",
	                         OPT_OPTIONAL, OPT_SINGLE, ARG_CHOICE, 3, ARGNO_UNLIMITED, choices, NULL
	                     )
	                    );
	cliparser_add_pos_arg(cmd,
	                      cliarg_new("arg1", "first integer argument", ARG_INT)
	                     );
	cliparser_add_pos_arg(cmd,
	                      cliarg_new("arg2", "second string argument", ARG_STR)
	                     );
	cliparser_add_pos_arg(cmd,
	                      cliarg_new_multi("arg3", "third multiple file argument", ARG_FILE)
	                     );

	cliparser *scmd1 = cliparser_new("subcommand1", "first subcommand");
	cliparser_add_option(scmd1,
	                     cliopt_new_defaults(
	                         'j',
	                         "jjj",
	                         "optional with no value"
	                     )
	                    );
	cliparser_add_option(scmd1,
	                     cliopt_new(
	                         'k',
	                         "kkk",
	                         "mandatory single option with one boolean value",
	                         OPT_REQUIRED, OPT_SINGLE, ARG_BOOL, 1, 1, NULL, NULL
	                     )
	                    );
	cliparser_add_option(scmd1,
	                     cliopt_new(
	                         'l',
	                         "lll",
	                         "non-mandatory multiple option with two values",
	                         OPT_OPTIONAL, OPT_MULTIPLE, ARG_BOOL, 2, 2, NULL, NULL
	                     )
	                    );
	cliparser_add_option(scmd1,
	                     cliopt_new(
	                         'm',
	                         "mmm",
	                         "non-mandatory single option with no value",
	                         OPT_OPTIONAL, OPT_SINGLE, ARG_NONE, 0, 0, NULL, NULL
	                     )
	                    );
	cliparser_add_option(scmd1,
	                     cliopt_new(
	                         'n',
	                         "nnn",
	                         "non-mandatory single option with one string value",
	                         OPT_OPTIONAL, OPT_SINGLE, ARG_STR, 1, 1, NULL, NULL
	                     )
	                    );
	cliparser_add_pos_arg(scmd1,
	                      cliarg_new("arg1", "first char argument", ARG_CHAR)
	                     );
	cliparser_add_pos_arg(scmd1,
	                      cliarg_new("arg2", "second float argument", ARG_FLOAT)
	                     );
	cliparser_add_pos_arg(scmd1,
	                      cliarg_new_multi("arg3", "third file argument", ARG_FILE)
	                     );
	cliparser_add_subcommand(cmd, scmd1);

}


static void test_teardown()
{
	DESTROY_FLAT(cmd, cliparser);
}


static char **make_argv(char *call, int *argc)
{
	char *str, *saveptr;
	int i;
	saveptr = call;
	vec *ret = vec_new(sizeof(char *));
	for (i = 0, str = call; ; i++, str = NULL) {
		char *tok = strtok_r(str, " ", &saveptr);
		if (!tok) break;
		tok = cstr_clone(tok);
		vec_push_rawptr(ret, tok );
	}
	*argc = i;
	char **argv = ARR_NEW(char *, *argc);
	for (i = 0; i < *argc; i++) {
		argv[i] = (char *)vec_get_rawptr(ret, i);
	}
	DESTROY_FLAT(ret, vec);
	return argv;
}


void freeargv(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
		FREE(argv[i]);
	FREE(argv);
}


void test_cli_parse(CuTest *tc)
{
	test_setup();

	int argc;
	char call[] =
	    "test -d somestring  subcommand1 -k true --lll true 0 -n some_string A 12.75 file1.c file2.c";
	char **argv = make_argv(call, &argc);

	cliparse_res result = cliparser_parse(cmd, argc, argv, false);
	CuAssert(tc, "CLI Parse error", result.ok);

	CuAssert(tc, "Used option not detected", cliparser_opt_used_from_shortname(cmd, 'd'));
	CuAssert(tc, "Non-used option detected", !cliparser_opt_used_from_shortname(cmd, 'z'));
	// -c is declared and has default values, so its value vector is not empty
	CuAssert(tc, "Declared, unused option with defaults detected as used",
	         !cliparser_opt_used_from_shortname(cmd, 'c'));
	CuAssert(tc, "Used option not detected by long name",
	         cliparser_opt_used_from_longname(cmd, "ddd"));
	CuAssert(tc, "Unused option detected by long name",
	         !cliparser_opt_used_from_longname(cmd, "ccc"));

	freeargv(argc, argv);

	test_teardown();
}


// Parses call with a parser for "prog <first> [<rest...>]" (or just
// "prog [<rest...>]" if !with_first). Returns the number of values of
// rest, or -1 on a parse error.
static int parse_optional_args(char *call, bool with_first)
{
	cliparser *prog = cliparser_new("prog", "Test program");
	if (with_first) {
		cliparser_add_pos_arg(prog, cliarg_new("first", "first argument", ARG_STR));
	}
	cliparser_add_pos_arg(prog, cliarg_new_multi_optional("rest", "other arguments",
	                      ARG_FILE));
	int argc;
	char **argv = make_argv(call, &argc);
	cliparse_res result = cliparser_parse(prog, argc, argv, false);
	int ret = -1;
	if (result.ok) {
		ret = (int)vec_len(cliparser_arg_val_from_pos(prog, with_first ? 1 : 0));
	}
	freeargv(argc, argv);
	DESTROY_FLAT(prog, cliparser);
	return ret;
}


void test_cli_optional_args(CuTest *tc)
{
	char none[] = "prog", one[] = "prog x", three[] = "prog x a.c b.c";
	char none2[] = "prog", two[] = "prog a.c b.c";
	CuAssertIntEquals(tc, -1, parse_optional_args(none, true)); // <first> missing
	CuAssertIntEquals(tc, 0, parse_optional_args(one, true));
	CuAssertIntEquals(tc, 2, parse_optional_args(three, true));
	CuAssertIntEquals(tc, 0, parse_optional_args(none2, false));
	CuAssertIntEquals(tc, 2, parse_optional_args(two, false));
}


CuSuite *cli_get_test_suite()
{
	CuSuite *suite = CuSuiteNew("cli");
	SUITE_ADD_TEST(suite, test_cli_parse);
	SUITE_ADD_TEST(suite, test_cli_optional_args);
	return suite;
}



