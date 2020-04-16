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
#include "new.h"

static cliparse *cmd;


static void test_setup()
{
	/*
	cmd = cliparse_new("test", "A Test Program");
	char choice_arr[3][8]  = {"choice1", "choice2", "choice3"};
	vec *choices = vec_new(sizeof(char *));
	for (size_t i=0; i<3; vec_push_rawptr(choices, cstr_clone(choice_arr[i++])));
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
	                        cstr_clone("non-mandatory single option with one boolean value"),
	                        OPT_OPTIONAL, OPT_SINGLE, ARG_BOOL, 1, 1, NULL, NULL
	                    )
	                   );
	vec *def = vec_new(sizeof(long));
	vec_push_long(def,1234);
	vec_push_long(def,4321);
	vec_push_long(def,2143);
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'c',
	                        cstr_clone("ccc"),
	                        cstr_clone("non-mandatory multiple option with two int values"),
	                        OPT_OPTIONAL, OPT_MULTIPLE, ARG_INT, 2, 5, NULL, def
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'd',
	                        cstr_clone("ddd"),
	                        cstr_clone("mandatory single option with one string value"),
	                        OPT_REQUIRED, OPT_SINGLE, ARG_STR, 1, 1, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'e',
	                        cstr_clone("eee"),
	                        cstr_clone("non-mandatory multiple option with one tp three file value"),
	                        OPT_OPTIONAL, OPT_MULTIPLE, ARG_FILE, 1, 3, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'f',
	                        cstr_clone("fff"),
	                        cstr_clone("non-mandatory single option with unlimited float values"),
	                        OPT_OPTIONAL, OPT_SINGLE, ARG_FLOAT, 1, ARGNO_UNLIMITED, NULL, NULL
	                    )
	                   );
	cliparse_add_option(cmd,
	                    cliopt_new_valued(
	                        'g',
	                        cstr_clone("ggg"),
	                        cstr_clone("non-mandatory single option three or more  values"),
	                        OPT_OPTIONAL, OPT_SINGLE, ARG_CHOICE, 3, ARGNO_UNLIMITED, choices, NULL
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
	                        cstr_clone("mandatory single option with one boolean value"),
	                        OPT_REQUIRED, OPT_SINGLE, ARG_BOOL, 1, 1, NULL, NULL
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'l',
	                        cstr_clone("lll"),
	                        cstr_clone("non-mandatory multiple option with two values"),
	                        OPT_OPTIONAL, OPT_MULTIPLE, ARG_BOOL, 2, 2, NULL, NULL
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'm',
	                        cstr_clone("mmm"),
	                        cstr_clone("non-mandatory single option with no value"),
	                        OPT_OPTIONAL, OPT_SINGLE, ARG_NONE, 0, 0, NULL, NULL
	                    )
	                   );
	cliparse_add_option(scmd1,
	                    cliopt_new_valued(
	                        'n',
	                        cstr_clone("nnn"),
	                        cstr_clone("non-mandatory single option with one string value"),
	                        OPT_OPTIONAL, OPT_SINGLE, ARG_STR, 1, 1, NULL, NULL
	                    )
	                   );
	cliparse_add_pos_arg(scmd1,
	                     cliarg_new("arg1", "first char argument", ARG_CHAR)
	                    );
	cliparse_add_pos_arg(scmd1,
	                     cliarg_new("arg2", "second float argument", ARG_FLOAT)
	                    );
	cliparse_add_pos_arg(scmd1,
	                     cliarg_new_multi("arg3", "third file argument", ARG_FILE)
	                    );
	cliparse_add_subcommand(cmd, scmd1);
	*/
	vec *choices = vec_new(sizeof(char *));
	vec_push_rawptr(choices, cstr_clone("plain"));
	vec_push_rawptr(choices, cstr_clone("fasta"));
	//vec_push_rawptr(choices, cstr_clone("index"));
	vec *deftype = vec_new(sizeof(char*));
	vec_push_rawptr(deftype, cstr_clone("fasta"));
	cmd = cliparse_new("vmat", "Variable Minimizer Alignment Tool");
	cliparse *index = cliparse_new("index", "Create variable-sized minimiser index");
	cliparse_add_option(index, cliopt_new('w', "window-size", "window sizes", OPT_REQUIRED, OPT_SINGLE, ARG_INT, 1, 3, NULL, NULL));
	cliparse_add_option(index, cliopt_new('k', "kmer-size", "kmer sizes", OPT_REQUIRED, OPT_SINGLE, ARG_INT, 1, 3, NULL, NULL));
	cliparse_add_option(index, cliopt_new('t',"input-type", "Input sequence file type", OPT_OPTIONAL, OPT_SINGLE, ARG_CHOICE, 1, 1, choices, deftype));
	cliparse_add_pos_arg(index, cliarg_new("sequence file", "Input sequence file", ARG_FILE));
	cliparse_add_subcommand(cmd, index);

}


static void test_teardown()
{
	FREE(cmd, cliparse);
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
	for (i=0, str=call; ; i++, str=NULL) {
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

void freeargv(int argc, char **argv)
{
	for (int i=0; i<argc; i++)
		FREE(argv[i]);
	FREE(argv);
}


void test_cli_parse(CuTest *tc)
{
	test_setup();

	int argc;
	//char call[] = "test -d somestring  subcommand1 -k true --lll true 0 -n some_string A 12.75 file1.c file2.c";
	char call[] = "vmat index -k 5 10 input.fasta -w 20 30 40";
	char **argv = make_argv(call, &argc);

	cliparse_parse(cmd, argc, argv);

	cliparse *subcmd = cliparse_active_subcommand(cmd);
	const vec *tvals = cliparse_opt_val_from_shortname(subcmd, 't');
	CuAssertSizeTEquals(tc, 1, vec_len(tvals));
	CuAssertStrEquals(tc, "fasta", (char*)vec_first_rawptr(tvals));

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



