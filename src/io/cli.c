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
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include "hashmap.h"
#include "cli.h"
#include "new.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "arrutil.h"
#include "hash.h"
#include "vec.h"
#include "new.h"

struct _cliopt {
	char shortname;		/* Short option name e.g. f (-f) */
	char *longname;		/* Long option name e.g. foo (--foo) */
	char *help;			/* Help description */
	bool mandatory;		/* true=mandatory argument(default); false=optional */
	bool single;		/* true=can appear only once(default), false=can appear multiple times */
	cliargtype type;	/* The type of option values */
	size_t min_val_no;	/* Minimum number of option values (default = 0) */
	size_t max_val_no;	/* Maximum number of option values (default = 0) */
	vec *choices;		/* Value choices if type==ARG_CHOICE */
	vec *defaults;		/* Default values */
	vec *values;		/* Actual parsed values */
};

struct _cliarg {
	char *name;			/* argument name */
	char *help;			/* help message */
	cliargtype type;	/* value type */
	bool single;		/* true=one single value; false=multiple */
	vec *choices;		/* value choices if type=ARG_CHOICE */
	vec *values;		/* Actual parsed value(s) */
};

struct _cliparse {
	struct _cliparse *par;		/* parent command (fur subcommands) */
	char *name;					/* command name */	
	char *help;					/* help message */
	hashmap *subcommands;		/* subcomands indexed by name */
	vec *subcmd_names;			/* subcommand names by order of addition */
	cliparse *active_subcmd;	/* used subcommand in a command call */
	hashmap *options;			/* command options indexed by short name */
	hashmap *long_to_short;		/* Long-to-short option name map */
	vec *args;					/* Vector of positional arguments */
};


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
	if (ret->single) {
		switch (type) {
		case ARG_NONE:
			ret->values = NULL;
			break;
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
	} else {
		ret->values	= vec_new(sizeof(vec *));
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
	case ARG_NONE:
		ret->values = NULL;
		break;
	case ARG_BOOL:
		ret->values = vec_new(sizeof(bool));
		break;
	case ARG_CHAR:
		ret->values = vec_new(sizeof(char));
		break;
	case ARG_INT:
		ret->values = vec_new(sizeof(long));
		break;
	case ARG_FLOAT:
		ret->values = vec_new(sizeof(double));
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


cliarg *cliarg_new_choice(char *name, char*help, vec *choices)
{
	cliarg *ret = cliarg_new(name, help, ARG_CHOICE);
	ret->choices = choices;
	return ret;
}


cliarg *cliarg_new_choice_multi(char *name, char*help, vec *choices)
{
	cliarg *ret = cliarg_new_choice(name, help, choices);
	ret->single = false;
	return ret;
}

static uint64_t _hash_str(const void *s)
{
	return fnv1a_64bit_hash(s, strlen((char *)s));
}


static bool _str_eq(const void *s1, const void *s2)
{
	return strcmp((char *)s1, (char *)s2)==0;
}


cliparse *cliparse_new(char *name, char *help)
{
	cliparse *ret = NEW(cliparse);
	assert(name);
	ret->par = NULL;
	ret->name = name;
	ret->help = help;
	ret->subcommands = hashmap_new(sizeof(char *), sizeof(cliparse *),  _hash_str, _str_eq);
	ret->subcmd_names = vec_new(sizeof(char *));
	ret->active_subcmd = NULL;
	ret->options = hashmap_new(sizeof(char), sizeof(cliopt *), ident_hash_char, eq_char);
	ret->long_to_short = hashmap_new(sizeof(char *), sizeof(char), _hash_str, _str_eq);
	ret->args = vec_new(sizeof(cliarg *));
	cliopt *help_opt = cliopt_new_defaults('h', "help", "Prints help message");
	hashmap_set(ret->options, &help_opt->shortname, &help_opt);
	return ret;
}


void cliparse_add_subcommand(cliparse *cmd,  cliparse *subcmd)
{
	assert(cmd->par==NULL);
	assert(cmd!=subcmd);
	assert(!hashmap_has_key(cmd->subcommands, &subcmd->name));
	subcmd->par = cmd;
	vec_push_rawptr(cmd->subcmd_names, &subcmd->name);
	hashmap_set(cmd->subcommands, &(subcmd->name), &subcmd);
}


void cliparse_add_option(cliparse *cmd, cliopt *opt)
{
	assert( opt->shortname!='h' && (opt->longname==NULL || strcmp(opt->longname, "help")!=0) );
	//assert(vec_len(cmd->subcommands)==0);
	assert( !hashmap_has_key(cmd->options, &opt->shortname) );
	assert( !hashmap_has_key(cmd->long_to_short, &opt->longname) );
	hashmap_set(cmd->options, &(opt->shortname), &opt);
	if (opt->longname) {
		hashmap_set(cmd->long_to_short, &opt->longname, &opt->shortname);
	}
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
	bool has_subcmds = hashmap_size(cmd->subcommands) > 0;
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
		for (size_t i = 0, l = vec_len(cmd->subcmd_names); i < l; i++) {
			char *subcmd_name = (char *)vec_get_rawptr(cmd->subcmd_names, i);
			cliparse *subcmd = (cliparse *)hashmap_get_rawptr(cmd->subcommands, &subcmd_name);
			printf("  %s\t%s\n",subcmd->name, subcmd->help);
		}
		printf("\n  Run \"%s%s%s <subcommand> --help\" for help on a specific subcommand.",
		       (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	}
}


#define CHECK(ASSERTION, FORMAT, ...) \
if (!(ASSERTION)) { \
	fprintf(stderr,  FORMAT, ##__VA_ARGS__ ); \
	exit(EXIT_FAILURE); \
}

bool _collect_vals(vec *ret, char **argv, size_t from, size_t to, cliargtype type, vec *choices, int *err_pos)
{
	if (ret == NULL) {
		switch (type) {
		case ARG_NONE:
			break;
		case ARG_BOOL:
			ret = vec_new(sizeof(bool));
			break;
		case ARG_CHAR:
			ret = vec_new(sizeof(char));
			break;
		case ARG_INT:
			ret = vec_new(sizeof(long));
			break;
		case ARG_FLOAT:
			ret = vec_new(sizeof(double));
			break;
		case ARG_STR:
			ret = vec_new(sizeof(char *));
			break;
		case ARG_FILE:
			ret = vec_new(sizeof(char *));
			break;
		case ARG_DIR:
			ret = vec_new(sizeof(char *));
			break;
		case ARG_CHOICE:
			assert(choices);
			ret = vec_new(sizeof(int));
			break;
		}
	}
	strbuf *tokbuf = strbuf_new();
	for (int i=from; i<to; i++) {
		strbuf_clear(tokbuf);
		strbuf_append(tokbuf, argv[i]);
		const char *tok = strbuf_as_str(tokbuf);
		switch (type) {
		case ARG_NONE:
		case ARG_BOOL:
			if (cstr_equals_ignore_case(tok, "true") || cstr_equals(tok, "1")) {
				vec_push__Bool(ret, true);
			} else if (cstr_equals_ignore_case(tok, "false") || cstr_equals(tok, "0")) {
				vec_push__Bool(ret, false);
			} else {
				*err_pos = i;
				return false;
			}
			break;
		case ARG_CHAR:
			if (strlen(argv[i]) != 1) {
				*err_pos = i;
				return false;
			}
			vec_push_char(ret, argv[i][0]);
			break;
		case ARG_INT:
			;
			char *endptr = NULL;
			long lval = strtol(tok, &endptr, 10);
			if ( (errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN))
			        || (errno != 0 && lval == 0) || endptr == tok || *endptr != '\0') {
				*err_pos = i;
				return false;
			}
			vec_push_long(ret, lval);
			break;
		case ARG_FLOAT:
			;
			endptr = NULL;
			double dval = strtod(tok, &endptr);
			if ( (errno == ERANGE && (dval ==  HUGE_VALF || dval == HUGE_VALL))
			        || (errno != 0 && dval == 0) || endptr == tok || *endptr != '\0') {
				*err_pos = i;
				return false;
			}
			vec_push_double(ret, dval);
			break;
		case ARG_STR:
			;
			char *sval = cstr_clone(tok);
			vec_push(ret, &sval);
			break;
		case ARG_FILE:
			;
			char *fval = cstr_clone(tok);
			vec_push(ret, &fval);
			break;
		case ARG_DIR:
			;
			char *pval = cstr_clone(tok);
			vec_push(ret, &pval);
			break;
		case ARG_CHOICE:
			assert(choices);
			if (vec_find(choices, (void *)tok, _str_eq) == vec_len(choices)) {
				*err_pos = i;
				return false;
			}
			break;
		}
	}
	strbuf_free(tokbuf);
	return true;
}

typedef enum {
	P_CMD_OPT = 0,
	P_CMD_ARG = 1,
	P_SUB_OPT = 2,
	P_SUB_ARG = 3
} parse_state;

void cliparse_parse(cliparse *clip, int argc, char **argv)
{
	int t = 1;
	parse_state state = P_CMD_OPT;
	cliparse *cur_parse = clip;
	cliopt *cur_opt = NULL;
	size_t cur_pos_arg = 0;
	strbuf *tokbuf = strbuf_new();
	while( t < argc ) {
		strbuf_clear(tokbuf);
		strbuf_append(tokbuf, argv[t]);
		const char *tok = strbuf_as_str(tokbuf);
		size_t tlen = strbuf_len(tokbuf);
		// options
		if ( tok[0] == '-') {
			CHECK(state==P_CMD_OPT || state==P_SUB_OPT, 
				  "Unexpected option %s at position %d.\n", argv[t], t);
			CHECK(tlen > 1, "Unknown option - at position %d.\n", t);
			char shortname;
			if (tlen > 1 && tok[1] == '-') {
				// long version
				tok = &tok[2];
				CHECK(hashmap_has_key(clip->long_to_short, &tok), "Unknown option %s at position %d.\n", argv[t], t);
				shortname = hashmap_get_char(clip->long_to_short, &tok);
			} else {
				// short version
				CHECK(tlen==2, "Unknown option %s at position %d\n", tok, t);
				shortname = tok[1];
			}
			cur_opt = (cliopt *) hashmap_get_rawptr(clip->options, &shortname);
			CHECK(cur_opt->single && vec_len(cur_opt->values) > 0,
			      "Invalid multiple definition of option %s at position %d.", argv[t], t);
			if (cur_opt->max_val_no > 0) { // has values
				// look ahead for potential values
				int end;
				for (end=0; end < argc && argv[end][0]!='-' 
					 && hashmap_has_key(cur_parse->subcommands, &argv[end]); end++);
				// check to see if number of args is OK
				int nargs = end - t - 1;
				CHECK(nargs < cur_opt->min_val_no,
				      "Too few arguments for option %s at position %d \
					   (found %d, but at least %zu required)\n.", argv[t], t, nargs, cur_opt->min_val_no);
				CHECK(nargs > cur_opt->max_val_no,
				      "Too many arguments for option %s at position %d \
					   (found %d, but at most %zu allowed)\n.", argv[t], t, nargs, cur_opt->max_val_no);
				// collect values
				vec *vals;
				int err_pos = -1;
				CHECK( _collect_vals(vals, argv, t+1, end, cur_opt->type, cur_opt->choices, &err_pos),
				       "Wrong value for option %s at position %d. \
					   Found %s at position %d whereas expected type was <%s>",
				       argv[t], t, argv[err_pos], err_pos, val_types[cur_opt->type]);
				
				if (cur_opt->single) {
					vec_cat(cur_opt->values, vals);
					FREE(vals, vec);
				}
				else {
					vec_push(cur_opt->values, &vals);
				}
				t = end;
				continue;
			}
			else { // has no values, boolean assumed
				if (vec_len(cur_opt->values)) {
					vec_set__Bool(cur_opt->values, 0, true);
				}
				else {
					vec_push__Bool(cur_opt->values, true);
				}
			}
		}
		// subcommands
		else if(hashmap_has_key(cur_parse->subcommands, &tok)) {
			CHECK(state==P_CMD_OPT || state==P_SUB_OPT, 
				  "Unexpected subcommand \"%s\" at position %d.\n", argv[t], t);
			assert(cur_parse==clip);
			assert(cur_parse->active_subcmd==NULL);
			cur_parse->active_subcmd  = (cliparse *) hashmap_get(cur_parse->subcommands, &tok);
			cur_parse = cur_parse->active_subcmd;
			state = P_SUB_OPT;
		}
		// arguments
		else {
			CHECK(state==P_CMD_OPT || state==P_SUB_OPT || state==P_CMD_ARG || state==P_SUB_ARG, 
				  "Unexpected token \"%s\" at position %d.\n", argv[t], t);
			cliarg *cur_arg = (cliarg *)vec_get_rawptr(cur_parse->args, cur_pos_arg);
			CHECK( t + 1 < argc, "Missing argument #%zu of type <%s> at position %d.\n", 
				   cur_pos_arg+1, val_types[cur_arg->type], t );
			int err_pos;
			_collect_vals(cur_arg->values, argv, t+1, t+2, cur_arg->type, NULL, &err_pos);
			if (cur_arg->single) { 
				cur_pos_arg++;
			}
			if (state==P_CMD_OPT) 
				state = P_CMD_ARG;
			else if (state==P_SUB_OPT) 
				state = P_SUB_ARG;
		}
		t++;
	}
	strbuf_free(tokbuf);
}