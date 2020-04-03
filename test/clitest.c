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

#include "arrutil.h"
#include "cli.h"
#include "cstrutil.h"
#include "vec.h"

static cliparse *cmd;

static void test_setup()
{
	cmd = cliparse_new("test", "A Test Program");
	char *choice_arr[3]  = {"choice1", "choice2", "choice3"};
	vec *choices = vec_new_from_arr_cpy(choice_arr, 3, sizeof(char *));
	cliparse_add_option(cmd,
	                    cliopt_new_defaults(
	                        'a',
	                        cstr_clone("aaa"),
	                        cstr_clone("optional with no value")
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'b',
	                        cstr_clone("bbb"),
	                        cstr_clone("mandatory single option with no value"),
	                        true, true, ARG_BOOL, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'c',
	                        cstr_clone("ccc"),
	                        cstr_clone("mandatory multiple option with no value"),
	                        true, false, ARG_BOOL, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'd',
	                        cstr_clone("ddd"),
	                        cstr_clone("non-mandatory multiple option with no value"),
	                        false, false, ARG_BOOL, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'e',
	                        cstr_clone("eee"),
	                        cstr_clone("mandatory single option with one boolean value"),
	                        true, true, ARG_BOOL, 1, 1, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'f',
	                        cstr_clone("fff"),
	                        cstr_clone("mandatory single option with two integer value"),
	                        true, true, ARG_INT, 2, 2, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'g',
	                        cstr_clone("ggg"),
	                        cstr_clone("mandatory single option three or more float values"),
	                        true, true, ARG_FLOAT, 3, ARGNO_UNLIMITED, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'i',
	                        cstr_clone("iii"),
	                        cstr_clone("non-mandatory single option three or more  values"),
	                        false, true, ARG_CHOICE, 3, ARGNO_UNLIMITED, choices, NULL
	                    )
	                   );
	cliparse_add_pos_arg(cmd,
	                     cliarg_new("arg1", "first integer argument", ARG_INT)
	                    );
	cliparse_add_pos_arg(cmd,
	                     cliarg_new("arg2", "second string argument", ARG_STR)
	                    );
	cliparse_add_pos_arg(cmd,
	                     cliarg_new_multi("arg3", "third multiple file argument", ARG_FILE)
	                    );

	cliparse *scmd1 = cliparse_new("subcommand1", "first subcommand");
	cliparse_add_option(scmd1,
	                    cliopt_new_defaults(
	                        'j',
	                        cstr_clone("jjj"),
	                        cstr_clone("optional with no value")
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'k',
	                        cstr_clone("kkk"),
	                        cstr_clone("mandatory single option with no value"),
	                        true, true, ARG_BOOL, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'l',
	                        cstr_clone("lll"),
	                        cstr_clone("mandatory multiple option with no value"),
	                        true, false, ARG_BOOL, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'm',
	                        cstr_clone("mmm"),
	                        cstr_clone("non-mandatory multiple option with no value"),
	                        false, false, ARG_BOOL, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'n',
	                        cstr_clone("nnn"),
	                        cstr_clone("mandatory single option with one boolean value"),
	                        true, true, ARG_BOOL, 1, 1, NULL, NULL
	                    )
	                   );
	cliparse_add_pos_arg(scmd1,
	                     cliarg_new("arg1", "first char argument", ARG_CHAR)
	                    );
	cliparse_add_pos_arg(scmd1,
	                     cliarg_new("arg2", "second float argument", ARG_FLOAT)
	                    );
	cliparse_add_pos_arg(scmd1,
	                     cliarg_new("arg3", "third file argument", ARG_FILE)
	                    );
	cliparse_add_subcommand(cmd, scmd1);

}


static void test_teardown() {
	FREE(cmd);
}


void test_cli_help(CuTest *tc)
{
	test_setup();
	cliparse_print_help(cmd);
	test_teardown();
}

static char **make_argv(char *call, int *argc) 
{
	char *str, *saveptr;
	int i;
	saveptr = call;
	vec *ret = vec_new(sizeof(char *));
	for (i=0, str=call; ; i++, str=NULL)
	{
		char *tok = strtok_r(str, " ", &saveptr);
		if (!tok) break;
		tok = cstr_clone(tok);
		vec_push_rawptr(ret, tok );
	}
	*argc = i;
	char **argv = NEW_ARR(char *, *argc);
	for (i=0; i<*argc; i++) {
		argv[i] = (char *)vec_get_rawptr(ret, i);
	}
	FREE(ret, vec);
	return argv;
}

void freeargv(int argc, char **argv) {
	for (int i=0; i<argc; i++)
		FREE(argv[i]);
	FREE(argv);
}


void test_cli_parse(CuTest *tc)
{
	test_setup();

	int argc;
	char call[] = "test subcommand1 --lll -n A 12.75 file1.c file2.c";
	char **argv = make_argv(call, &argc);

	cliparse_parse(cmd, argc, argv);

	freeargv(argc, argv);


	test_teardown();
}


CuSuite *cli_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	//SUITE_ADD_TEST(suite, test_cli_help);
	SUITE_ADD_TEST(suite, test_cli_parse);
	return suite;
}



