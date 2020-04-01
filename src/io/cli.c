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

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"
#include "cli.h"
#include "new.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "arrutil.h"
#include "hash.h"
#include "vec.h"
#include "new.h"


cliopt *cliopt_new_defaults(char shortname, char *longname, char *help)
{
	cliopt *ret = NEW(cliopt);
	ret->shortname = shortname;
	ret->longname = (longname) ? cstr_clone(longname) : NULL;
	ret->help = (help) ? cstr_clone(help) : NULL;
	ret->mandatory = false;
	ret->single = true;
	ret->type = ARG_NONE;
	ret->min_val_no = 0;
	ret->max_val_no = 0;
	ret->choices = NULL;
	ret->defaults = NULL;
	ret->values = NULL;
	return ret;
}


cliopt *cliopt_new_valued(char shortname, char *longname, char *help,
                          bool mandatory, bool single, cliargtype type,
                          size_t min_val_no, size_t max_val_no,
                          vec *choices, vec *defaults )
{
	assert( shortname!='h' && (longname==NULL || strcmp(longname, "help")!=0) );
	cliopt *ret = NEW(cliopt);
	ret->shortname = shortname;
	ret->longname = (longname) ? cstr_clone(longname) : NULL;
	ret->help = (help) ? cstr_clone(help) : NULL;
	ret->mandatory = mandatory;
	ret->single = single;
	ret->type = type;
	assert(type==ARG_NONE || min_val_no <= max_val_no);
	ret->min_val_no = min_val_no;
	ret->max_val_no = max_val_no;
	assert(type != ARG_CHOICE || vec_len(choices));
	ret->choices = choices;
	ret->defaults = defaults;
	switch (type) {
	case ARG_BOOL:
		ret->values = vec_new(sizeof(bool));
		break;
	case ARG_CHAR:
		ret->values = vec_new(sizeof(char));
		break;
	case ARG_INT:
		ret->values = vec_new(sizeof(int));
		break;
	case ARG_FLOAT:
		ret->values = vec_new(sizeof(float));
		break;
	case ARG_STR:
		ret->values = vec_new(sizeof(char *));
		break;
	case ARG_FILE:
		ret->values = vec_new(sizeof(char *));
		break;
	case ARG_DIR:
		ret->values = vec_new(sizeof(char *));
		break;
	case ARG_CHOICE:
		ret->values = vec_new(sizeof(int));
		break;
	}
	return ret;
}


cliarg *cliarg_new(char *name, char*help, cliargtype type)
{
	cliarg *ret = NEW(cliarg);
	ret->name = (name) ? cstr_clone(name) : NULL;
	ret->help = (help) ? cstr_clone(help) : NULL;
	ret->type = type;
	ret->single = true;
	switch (type) {
	case ARG_BOOL:
		ret->values = vec_new(sizeof(bool));
		break;
	case ARG_CHAR:
		ret->values = vec_new(sizeof(char));
		break;
	case ARG_INT:
		ret->values = vec_new(sizeof(int));
		break;
	case ARG_FLOAT:
		ret->values = vec_new(sizeof(float));
		break;
	case ARG_STR:
		ret->values = vec_new(sizeof(char *));
		break;
	case ARG_FILE:
		ret->values = vec_new(sizeof(char *));
		break;
	case ARG_DIR:
		ret->values = vec_new(sizeof(char *));
		break;
	case ARG_CHOICE:
		ret->values = vec_new(sizeof(int));
		break;
	}
	return ret;
}


cliarg *cliarg_new_multi(char *name, char*help, cliargtype type)
{
	cliarg *ret = cliarg_new(name, help, type);
	ret->single = false;
	return ret;
}

/*
static uint64_t _hash_str(const void *s)
{
	return fnv1a_64bit_hash(s, strlen((char *)s));
}


static bool _str_eq(const void *s1, const void *s2)
{
	return strcmp((char *)s1, (char *)s2)==0;
}
*/

cliparse *cliparse_new(char *name, char *help)
{
	cliparse *ret = NEW(cliparse);
	assert(name);
	ret->par = NULL;
	ret->name = name;
	ret->help = help;
	ret->subcommands = vec_new(sizeof(cliparse *));// hashmap_new(sizeof(char *), sizeof(cliparse *),  _hash_str, _str_eq);
	ret->options = hashmap_new(sizeof(char), sizeof(cliopt *), ident_hash_char, eq_char);
	ret->args = vec_new(sizeof(cliarg *));
	cliopt *help_opt = cliopt_new_defaults('h', "help", "Prints help message");
	hashmap_set(ret->options, &help_opt->shortname, &help_opt);
	return ret;
}

static bool _eq_subcmd(const void *l, const void *r)
{
	return(strcmp((((const cliparse**)l)[0])->name, (((const cliparse**)r)[0])->name)==0);
}

void cliparse_add_subcommand(cliparse *cmd,  cliparse *subcmd)
{
	assert(cmd->par==NULL);
	assert(cmd!=subcmd);
	assert(vec_find(cmd->subcommands, &subcmd, _eq_subcmd) == vec_len(cmd->subcommands));
	subcmd->par = cmd;
	vec_push_rawptr(cmd->subcommands, subcmd);
	//hashmap_set(cmd->subcommands, &(subcmd->name), &subcmd);
}


void cliparse_add_option(cliparse *cmd, cliopt *opt)
{
	assert( opt->shortname!='h' && (opt->longname==NULL || strcmp(opt->longname, "help")!=0) );
	//assert(vec_len(cmd->subcommands)==0);
	hashmap_set(cmd->options, &(opt->shortname), &opt);
}


void cliparse_add_pos_arg(cliparse *cmd, cliarg *arg)
{
	//assert(vec_len(cmd->subcommands)==0);
	assert( arg->single ||
	        vec_len(cmd->args) == 0 ||
	        ((cliarg*)vec_last_rawptr(cmd->args))->single );
	vec_push_rawptr(cmd->args, arg);
}


static char *delim_names[8] = {"","","(",")+","[","]","(",")*"};
static char *arity_names[4] = {"!","+","?","*"};
static char *val_types[] = {"", "true|false", "char", "integer", "float", "literal", "file", "dir", "choice"};
void cliopt_print_help(cliopt *opt)
{
	size_t nmdel;
	if (opt->mandatory) { // mandatory
		if (opt->single) nmdel = 0; // exactly once
		else nmdel = 1; // one or more
	} else { // optional
		if (opt->single) nmdel = 2; //zero or one
		else nmdel = 3; // zero or more
	}
	//printf("%s-%c", delim_names[nmdel], opt->shortname);


	printf("  (%s)", arity_names[nmdel]);
	printf(" -%c", opt->shortname);
	if (opt->longname!=NULL) {
		printf(" --%s", opt->longname);
	}


	// print options
	if (opt->max_val_no!=0) {
		char *nbstr = cstr_new(10);
		strbuf *typedescr = strbuf_new();
		strbuf_append(typedescr, " <");
		if (opt->type==ARG_CHOICE) {
			vec_iter *it = vec_get_iter(opt->choices);
			strbuf_join_iter(typedescr, ASTRAIT(it, vec_iter, iter), "|");
			FREE(it);
		} else {
			strbuf_append(typedescr, val_types[opt->type]);
		}
		strbuf_append(typedescr, ">");
		if (opt->min_val_no==1 && opt->max_val_no==1) {
		} else if (opt->min_val_no==0 && opt->max_val_no==ARGNO_UNLIMITED) {
			strbuf_append(typedescr,"*");
		} else if (opt->min_val_no==1 && opt->max_val_no==ARGNO_UNLIMITED) {
			strbuf_append(typedescr,"+");
		} else {
			strbuf_append(typedescr, "[");
			uint_to_cstr(nbstr, opt->min_val_no, 10);
			strbuf_append(typedescr, nbstr);
			if (opt->min_val_no != opt->max_val_no) {
				strbuf_append(typedescr, "..");
				if (opt->max_val_no != ARGNO_UNLIMITED) {
					uint_to_cstr(nbstr, opt->max_val_no, 10);
					strbuf_append(typedescr, nbstr);
				}
			}
			strbuf_append(typedescr, "]");
		}
		printf("%s", strbuf_as_str(typedescr));
		FREE(nbstr);
		strbuf_free(typedescr);
	}

	// printf("%s", delim_names[nmdel+1]);
	//printf("\t%s", arity_names[nmdel]);
	printf("\t%s\n", (opt->help) ? opt->help : "");
}



void cliparse_print_help(cliparse *cmd)
{
	bool has_options = hashmap_size(cmd->options) > 0;
	bool has_args = vec_len(cmd->args) > 0;
	bool has_subcmds = vec_len(cmd->subcommands) > 0;
	printf("Usage:\n\n");
	printf("  %s%s%s", (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	if (has_options) {
		printf(" <options>");
	}
	for (size_t i=0, l=vec_len(cmd->args); i < l; i++) {
		cliarg *arg = (cliarg *)vec_get_rawptr(cmd->args, i);
		printf(" %s", arg->name);
	}
	printf("\n");
	if (has_subcmds) {
		printf("  %s%s%s <subcommand> <subcommand options> <subcommand args>\n",
		       (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	}
	if (has_options) {
		printf("\nOptions:\n\n");
		hashmap_iter *iter = hashmap_get_iter(cmd->options);
		vec *shortnames = vec_new(sizeof(char));
		while(hashmap_iter_has_next(iter)) {
			hashmap_entry entry = hashmap_iter_next(iter);
			vec_push_char(shortnames, *((char *)entry.key));
		}
		vec_qsort(shortnames, cmp_char);
		for (size_t i=0, l=vec_len(shortnames); i<l; i++) {
			cliopt *opt = (cliopt *) hashmap_get_rawptr(cmd->options, vec_get(shortnames, i));
			cliopt_print_help(opt);
		}
		printf("\n  usage : ! = required, ? = optional, + = one or more,  * = zero or more\n");
		printf("  values: <type> = one value,  <type>[N] = N values,\n");
		printf("          <type>[M..] = at least M values, <type>[M..N] = from M to N values\n");
	}
	if (has_subcmds) {
		printf("\nSubcommands:\n\n");
		for (size_t i = 0, l = vec_len(cmd->subcommands); i < l; i++) {
			cliparse *subcmd = (cliparse *)vec_get_rawptr(cmd->subcommands, i);
			printf("  %s\t%s\n",subcmd->name, subcmd->help);
		}
		printf("\n  Run \"%s%s%s <subcommand> --help\" for help on a specific subcommand.",
		       (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	}
}


#define CHECK(ASSERTION, FORMAT, ...) \
if (!(ASSERTION)) { \
	fprintf(stderr,  FORMAT, ##__VA_ARGS__ ); \
	exit(1); \
}


void cliparse_parse(cliparse *clip, int argc, char **argv)
{
	size_t t = 1;
	cliparse *curparse = clip;
	while( t < argc ) {
		char *tok = argv[t];
		size_t tlen = strlen(tok);

		// options
		if (tlen && tok[0] == '-') {
			if (tlen > 1 && tok[1] == '-') {
				// long version
			} else {
				// short version
				CHECK(tlen==3, "Invalid option %s", tok);
			}
		}

		// subcommands

		t++;
	}

}