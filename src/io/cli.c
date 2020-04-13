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

#include "arrutil.h"
#include "cli.h"
#include "cstrutil.h"
#include "hash.h"
#include "hashmap.h"
#include "iter.h"
#include "mathutil.h"
#include "new.h"
#include "strbuf.h"
#include "vec.h"

struct _cliopt {
	char shortname;		/* Short option name e.g. f (-f) */
	char *longname;		/* Long option name e.g. foo (--foo) */
	char *help;			/* Help description */
	clioptneed need;		/* true=mandatory argument(default); false=optional */
	clioptmultiplicity multi;		/* true=can be declared multiple times, false=can appear only once(default) */
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
	bool single_val;	/* true=one single value; false=multiple */
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



static vec *_vals_vec_new(clioptmultiplicity multi, cliargtype type)
{
	vec *ret = NULL;
	switch (multi) {
	case OPT_SINGLE:
		switch (type) {
		case ARG_NONE:
			ret = vec_new(sizeof(bool));
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
			ret = vec_new(sizeof(int));
			break;
		}
		break;
	case OPT_MULTIPLE: 
		ret	= vec_new(sizeof(vec *));
		break;
	}
	return ret;
}


static void _vals_vec_free(vec *vals, clioptmultiplicity multi, cliargtype type)
{
	switch (type) {
	case ARG_NONE:
		FREE(vals, vec);
		break;
	case ARG_BOOL:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), DTOR(vec))));
			break;
		case OPT_SINGLE:
			FREE(vals, vec);
			break;
		}
		break;
	case ARG_CHAR:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), DTOR(vec))));
			break;
		case OPT_SINGLE:
			FREE(vals, vec);
			break;
		}
		break;
	case ARG_INT:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), DTOR(vec))));
			break;
		case OPT_SINGLE:
			FREE(vals, vec);
			break;
		}
		break;
	case ARG_FLOAT:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), DTOR(vec))));
			break;
		case OPT_SINGLE:
			FREE(vals, vec);
			break;
		}
		break;
	case ARG_STR:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), dtor_cons(DTOR(vec), ptr_dtor()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, dtor_cons(DTOR(vec), ptr_dtor()));
			break;
		}
		break;
	case ARG_FILE:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), dtor_cons(DTOR(vec), ptr_dtor()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, dtor_cons(DTOR(vec), ptr_dtor()));
			break;
		}
		break;
	case ARG_DIR:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), dtor_cons(DTOR(vec), ptr_dtor()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, dtor_cons(DTOR(vec), ptr_dtor()));
			break;
		}
		break;
	case ARG_CHOICE:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), DTOR(vec))));
			break;
		case OPT_SINGLE:
			FREE(vals, vec);
			break;
		}
		break;
	}
}


cliopt *cliopt_new_defaults(char shortname, char *longname, char *help)
{
	return cliopt_new_valued(shortname, longname, help, false, false, 
							 ARG_NONE, OPT_OPTIONAL, OPT_SINGLE, NULL, NULL);
}


cliopt *cliopt_new_valued(char shortname,  char *longname, char *help,
                          clioptneed need, clioptmultiplicity multiplicity, 
						  cliargtype type, size_t min_val_no, size_t max_val_no,
                          vec *choices, vec *defaults )
{
	//assert( shortname!='h' );
	//assert( longname==NULL || strcmp(longname, "help")!=0 );
	assert( need==OPT_OPTIONAL || max_val_no!=0 );
	assert( multiplicity==OPT_SINGLE || max_val_no!=0 );
	assert( max_val_no!=0 || type==ARG_NONE);
	assert( type!=ARG_NONE || max_val_no==0);
	assert( type!=ARG_NONE || defaults==NULL);
	assert( min_val_no <= max_val_no );
	assert( type!=ARG_CHOICE || ( choices!=NULL && vec_len(choices) ));
	assert( defaults==NULL || 
	        ( min_val_no <= vec_len(defaults) 
			  && vec_len(defaults) <= max_val_no ) );
	cliopt *ret = NEW(cliopt);
	ret->shortname = shortname;
	ret->longname = (longname) ? cstr_clone(longname) : NULL;
	ret->help = (help) ? cstr_clone(help) : NULL;
	ret->need = need;
	ret->multi = multiplicity;
	ret->type = type;
	ret->min_val_no = min_val_no;
	ret->max_val_no = max_val_no;
	ret->choices = choices;
	ret->defaults = defaults;
	ret->values = _vals_vec_new(multiplicity , type);
	return ret;
}


void cliopt_dtor(void *ptr, const dtor *dt)
{
	cliopt *opt = (cliopt *)ptr;
	FREE(opt->longname);
	FREE(opt->help);
	DESTROY(opt->choices, dtor_cons(DTOR(vec), ptr_dtor()));
	switch (opt->type) {
	case ARG_STR:
	case ARG_FILE:
	case ARG_DIR:
		DESTROY(opt->defaults, dtor_cons(DTOR(vec), ptr_dtor()));
		break;
	default:
		FREE(opt->defaults, vec);
		break;
	}
	_vals_vec_free(opt->values, opt->multi, opt->type);
}


cliarg *cliarg_new(char *name, char*help, cliargtype type)
{
	cliarg *ret = NEW(cliarg);
	ret->name = (name) ? cstr_clone(name) : cstr_clone("Unnamed");
	ret->help = (help) ? cstr_clone(help) : cstr_new(0);
	ret->type = type;
	ret->single_val = true;
	ret->values = _vals_vec_new(false, ret->type);
	return ret;
}


cliarg *cliarg_new_multi(char *name, char*help, cliargtype type)
{
	cliarg *ret = cliarg_new(name, help, type);
	ret->single_val = false;
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
	ret->single_val = false;
	return ret;
}


void cliarg_dtor(void *ptr, const dtor *dt)
{
	cliarg *arg = (cliarg *)ptr;
	FREE(arg->name);
	FREE(arg->help);
	DESTROY(arg->choices, dtor_cons(DTOR(vec), ptr_dtor()));
	_vals_vec_free(arg->values, false, arg->type);
}


static uint64_t _hash_str(const void *ptr)
{
	char *s = *((char **)ptr);
	return fnv1a_64bit_hash(s, strlen((char *)s));
}


static bool _str_eq(const void *s1, const void *s2)
{
	return strcmp(*((char **)s1), *((char **)s2))==0;
}


cliparse *cliparse_new(char *name, char *help)
{
	cliparse *ret = NEW(cliparse);
	assert(name);
	ret->par = NULL;
	ret->name = cstr_clone(name);
	ret->help = (help)?cstr_clone(help):cstr_new(0);
	ret->subcommands = hashmap_new(sizeof(char *), sizeof(cliparse *),  _hash_str, _str_eq);
	ret->subcmd_names = vec_new(sizeof(char *));
	ret->active_subcmd = NULL;
	ret->options = hashmap_new(sizeof(char), sizeof(cliopt *), ident_hash_char, eq_char);
	ret->long_to_short = hashmap_new(sizeof(char *), sizeof(char), _hash_str, _str_eq);
	ret->args = vec_new(sizeof(cliarg *));
	// add help option directly
	cliopt *help_opt = cliopt_new_defaults('h', "help", "Prints help message");
	cliparse_add_option(ret, help_opt);
	return ret;
}


void cliparse_dtor(void *ptr, const dtor *dt) 
{
	cliparse *clip = (cliparse *)ptr;
	DESTROY(clip->subcommands, dtor_cons(dtor_cons(DTOR(hashmap), empty_dtor()), dtor_cons(ptr_dtor(), DTOR(cliparse))));
	FREE(clip->subcmd_names, vec);
	DESTROY(clip->options, dtor_cons(dtor_cons(DTOR(hashmap), empty_dtor()), dtor_cons(ptr_dtor(), DTOR(cliopt))));
	FREE(clip->long_to_short, hashmap);
	DESTROY(clip->args, dtor_cons(DTOR(vec), dtor_cons(ptr_dtor(), DTOR(cliarg))));
	FREE(clip->name);
	FREE(clip->help);
}


const char *cliparse_name(const cliparse *cmd)
{
	return cmd->name;
}


const cliparse *cliparse_active_subcommand(const cliparse *cmd)
{
	return cmd->active_subcmd;
}


void cliparse_add_subcommand(cliparse *cmd,  cliparse *subcmd)
{
	assert(cmd->par==NULL);
	assert(cmd!=subcmd);
	assert(!hashmap_has_key(cmd->subcommands, &subcmd->name));
	subcmd->par = cmd;
	vec_push_rawptr(cmd->subcmd_names, subcmd->name);
	hashmap_set(cmd->subcommands, &(subcmd->name), &subcmd);
}


void cliparse_add_option(cliparse *cmd, cliopt *opt)
{
	assert( !hashmap_has_key(cmd->options, &(opt->shortname)) );
	assert( opt->longname==NULL || !hashmap_has_key(cmd->long_to_short, &(opt->longname)) );
	if (hashmap_has_key(cmd->long_to_short, &(opt->longname)) ){
		char sn = hashmap_get_char(cmd->long_to_short, &(opt->longname));
		cliopt *aopt = (cliopt *)hashmap_get_rawptr(cmd->options, &sn);
		assert(strcmp(aopt->longname,opt->longname)==0);
	}
	assert( !hashmap_has_key(cmd->long_to_short, &(opt->longname)) );
	hashmap_set(cmd->options, &(opt->shortname), &opt);
	if (opt->longname) {
		hashmap_set(cmd->long_to_short, &(opt->longname), &(opt->shortname));
	}
}


void cliparse_add_pos_arg(cliparse *cmd, cliarg *arg)
{
	//assert(vec_len(cmd->subcommands)==0);
	assert( arg->single_val ||
	        vec_len(cmd->args) == 0 ||
	        ((cliarg*)vec_last_rawptr(cmd->args))->single_val );
	vec_push_rawptr(cmd->args, arg);
}


static char *mult_lbl[4] = {"!","+","?","*"};
static char *type_lbl[9] = {"", "boolean", "char", "integer", "float", "literal", "file", "dir", "choice (see help)"};

static void _cliopt_print_help(cliopt *opt)
{
	size_t mult_idx = 0;
	switch (opt->need) {
	case OPT_REQUIRED:
		switch (opt->multi) {
			case OPT_MULTIPLE:
				mult_idx = 1;
				break;
			case OPT_SINGLE:
				mult_idx = 0;
				break;
		} 
		break;
	case OPT_OPTIONAL:
		switch (opt->multi) {
			case OPT_MULTIPLE:
				mult_idx = 3; //zero or more
				break;
			case OPT_SINGLE:
				mult_idx = 2; // zero or one
				break;
		} 
		break;
	}
	//printf("%s-%c", delim_names[nmdel], opt->shortname);
	printf("  (%s)", mult_lbl[mult_idx]);
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
			strbuf_append(typedescr, type_lbl[opt->type]);
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

	//printf("%s", delim_names[nmdel+1]);
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
		printf(" %s%s", arg->name, (arg->single_val)?"":"...");
	}
	printf("\n");
	if (has_subcmds) {
		printf("  %s%s%s <subcommand> <subcommand options> <subcommand args>\n",
		       (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	}
	if (has_options) {
		printf("\nOptions:\n\n");
		vec *shortnames = vec_new(sizeof(char));
		hashmap_iter *it = hashmap_get_iter(cmd->options);
		FOREACH_IN_ITER(entry, hashmap_entry, hashmap_iter_as_iter(it)) {
			vec_push_char(shortnames, *((char *)(entry->key)));
		}
		FREE(it);
		vec_qsort(shortnames, cmp_char);
		for (size_t i=0, l=vec_len(shortnames); i<l; i++) {
			cliopt *opt = (cliopt *) hashmap_get_rawptr(cmd->options, vec_get(shortnames, i));
			_cliopt_print_help(opt);
		}
		printf("\n  usage : ! = required, ? = optional, + = one or more,  * = zero or more\n");
		printf("  values: <type> = one value,  <type>[N] = N values,\n");
		printf("          <type>[M..] = at least M values, <type>[M..N] = from M to N values\n");
	}
	if (has_args) {
		printf("\nArguments:\n\n");
		for (size_t i=0, l=vec_len(cmd->args); i < l; i++) {
			cliarg *arg = (cliarg *)vec_get_rawptr(cmd->args, i);
			printf("  %s%s\t%s\t(%s%s)\n", arg->name, (arg->single_val)?"":"...", 
					(arg->help)?arg->help:"", type_lbl[arg->type], (arg->single_val)?"":"...");
		}
	}
	if (has_subcmds) {
		printf("\nSubcommands:\n\n");
		for (size_t i = 0, l = vec_len(cmd->subcmd_names); i < l; i++) {
			char *subcmd_name = (char *)vec_get_rawptr(cmd->subcmd_names, i);
			cliparse *subcmd = (cliparse *)hashmap_get_rawptr(cmd->subcommands, &subcmd_name);
			printf("  %s\t%s\n",subcmd->name, (subcmd->help)?subcmd->help:"");
		}
		printf("\n  Run \"%s%s%s <subcommand> --help\" for help on a specific subcommand.\n",
		       (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	}
}


#define CHECK(ASSERTION, FORMAT, ...) \
if (!(ASSERTION)) { \
	fprintf(stderr,  "Error: "FORMAT, ##__VA_ARGS__ ); \
	exit(EXIT_FAILURE); \
}


/*
 * Checks to see if required options were not declared.
 * AND
 * Attribute default values for undeclared non-required options when available.
 */
static void _check_missing_options(cliparse *cmd)
{
	strbuf *longname = strbuf_new_with_capacity(16);
	hashmap_iter *opt_it = hashmap_get_iter(cmd->options);
	FOREACH_IN_ITER(entry, hashmap_entry, hashmap_iter_as_iter(opt_it)) {
		cliopt *opt = *((cliopt **)(entry->val));
		strbuf_clear(longname);
		strbuf_append(longname, ", --");
		if (opt->longname) strbuf_append(longname, opt->longname);
		CHECK( opt->need == OPT_OPTIONAL || (opt->values != NULL && vec_len(opt->values) > 0 ),
			"Undefined mandatory option -%c%s.\n", opt->shortname, 
			(strbuf_len(longname)>4)?strbuf_as_str(longname):"");
		if (opt->need == OPT_OPTIONAL && vec_len(opt->values)==0 && opt->defaults!=NULL) {
			// add default values
			vec *vals = _vals_vec_new(opt->multi, opt->type);
			switch (opt->type) {
			case ARG_NONE:
				vec_cat(vals, opt->defaults);
				break;
			case ARG_BOOL:
				vec_cat(vals, opt->defaults);
				break;
			case ARG_CHAR:
				vec_cat(vals, opt->defaults);
				break;
			case ARG_INT:
				vec_cat(vals, opt->defaults);
				break;
			case ARG_FLOAT:
				vec_cat(vals, opt->defaults);
				break;
			case ARG_STR:
				for(size_t i=0, l=vec_len(opt->defaults); i<l; i++)
					vec_push_rawptr(vals, cstr_clone(vec_get(opt->defaults, i)));
				break;
			case ARG_FILE:
				for(size_t i=0, l=vec_len(opt->defaults); i<l; i++)
					vec_push_rawptr(vals, cstr_clone(vec_get(opt->defaults, i)));
				break;
			case ARG_DIR:
				for(size_t i=0, l=vec_len(opt->defaults); i<l; i++)
					vec_push_rawptr(vals, cstr_clone(vec_get(opt->defaults, i)));
				break;
			case ARG_CHOICE:
				break;
			}
			switch(opt->multi) {
			case OPT_MULTIPLE:
				vec_push(opt->values, &vals);
				break;
			case OPT_SINGLE:
				vec_cat(opt->values, vals);
				FREE(vals, vec);
				break;
			}
		}
	}
	FREE(opt_it);
	strbuf_free(longname);
}


/*
 * Try to collect at most max_no values from argv[from..] into the vector vals.
 * Every token is type-validated before being added.
 * If type=ARGV_CHOICE, a non-null choices vector must be informed.
 * The process halts as soon as an invalid value is found or max_no values
 * are successfully collected or argv is exhausted.
 */
vec *_collect_vals(vec *ret, int argc, char **argv, size_t from, size_t max_no, cliargtype type, vec *choices)
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
	for (int i=from, l = MIN(argc, from+max_no); i < l; i++) {
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
				return ret;
			}
			break;
		case ARG_CHAR:
			if (strlen(argv[i]) != 1) {
				return ret;
			}
			vec_push_char(ret, argv[i][0]);
			break;
		case ARG_INT:
			;
			char *endptr = NULL;
			long lval = strtol(tok, &endptr, 10);
			if ( (errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN))
			        || (errno != 0 && lval == 0) || endptr == tok || *endptr != '\0') {
				return ret;
			}
			vec_push_long(ret, lval);
			break;
		case ARG_FLOAT:
			;
			endptr = NULL;
			double dval = strtod(tok, &endptr);
			if ( (errno == ERANGE && (dval ==  HUGE_VALF || dval == HUGE_VALL))
			        || (errno != 0 && dval == 0) || endptr == tok || *endptr != '\0') {
				return ret;
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
			if (vec_find(choices, &tok, _str_eq) == vec_len(choices)) {
				return ret;
			}
			break;
		}
	}
	strbuf_free(tokbuf);
	return ret;
}



typedef enum {
	P_CMD_OPT = 0,
	P_CMD_ARG = 1,
	P_SUB_OPT = 2,
	P_SUB_ARG = 3,
	P_DONE    = 4
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
				CHECK( hashmap_has_key(cur_parse->long_to_short, &tok),
				      "Unknown option %s at position %d.\n", argv[t], t );
				shortname = hashmap_get_char(cur_parse->long_to_short, &tok);
			} else {
				// short version
				CHECK(tlen==2, "Unknown option %s at position %d\n", tok, t);
				shortname = tok[1];
			}
			if (shortname=='h') {
				cliparse_print_help(cur_parse);
				exit(EXIT_SUCCESS);
			}
			cur_opt = (cliopt *) hashmap_get_rawptr(cur_parse->options, &shortname);
			CHECK(cur_opt->multi == OPT_MULTIPLE || vec_len(cur_opt->values) == 0,
			      "Invalid multiple definition of option %s at position %d.", argv[t], t);
			if (cur_opt->max_val_no > 0) { // has values
				vec *vals = _collect_vals(NULL, argc, argv, t+1, cur_opt->max_val_no, cur_opt->type, cur_opt->choices);
				size_t nargs = vec_len(vals);					
				CHECK( nargs >= cur_opt->min_val_no,
				      "Too few values for option %s at position %d. Found %zu values of type %s, but at least %zu are required.\n", 
					   argv[t], t, nargs, type_lbl[cur_opt->type], cur_opt->min_val_no);
				switch(cur_opt->multi) {
				case OPT_MULTIPLE:
					vec_push(cur_opt->values, &vals);
					break;
				case OPT_SINGLE:
					vec_cat(cur_opt->values, vals);
					FREE(vals, vec);
					break;
				}
				t += (nargs + 1);
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
			cur_parse->active_subcmd  = (cliparse *) hashmap_get_rawptr(cur_parse->subcommands, &tok);
			cur_parse = cur_parse->active_subcmd;
			state = P_SUB_OPT;
		}
		// arguments
		else {
			CHECK(state==P_CMD_OPT || state==P_SUB_OPT || state==P_CMD_ARG || state==P_SUB_ARG, 
				  "Unexpected token \"%s\" at position %d.\n", argv[t], t);
			cliarg *cur_arg = (cliarg *)vec_get_rawptr(cur_parse->args, cur_pos_arg);
			//int err_pos;
			size_t nvals = vec_len(cur_arg->values);
			_collect_vals(cur_arg->values, argc, argv, t, 1, cur_arg->type, NULL);// &err_pos);
			nvals = vec_len(cur_arg->values) - nvals;
			CHECK(nvals==1, "Wrong value for argument #%zu (%s). Expected a <%s> value, but found %s.\n", 
				  cur_pos_arg+1, cur_arg->name, type_lbl[cur_arg->type], argv[t] );
			if (cur_arg->single_val) { 
				cur_pos_arg++;
				if (cur_pos_arg >= vec_len(cur_parse->args)) 
					state = P_DONE;
			}
			if (state==P_CMD_OPT) 
				state = P_CMD_ARG;
			else if (state==P_SUB_OPT) 
				state = P_SUB_ARG;
		}
		t++;
	}
	// check if some arguments undefined
	cliarg *cur_arg = (cliarg *)vec_get_rawptr(cur_parse->args, cur_pos_arg);
	CHECK( (vec_len(cur_parse->args)==0 && cur_pos_arg==0) ||
	       (vec_len(cur_parse->args)>0 && cur_pos_arg == vec_len(cur_parse->args)-1 
		   	&& vec_len(cur_arg->values) > 0),
		   "Missing argument #%zu of type <%s> at position %d.\n", 
			cur_pos_arg+1, type_lbl[cur_arg->type], t );
	// check if mandatory options with no default values undefined
	_check_missing_options(clip);
	if (clip->active_subcmd) _check_missing_options(clip->active_subcmd);
	strbuf_free(tokbuf);
}


const vec *cliparse_opt_val_from_shortname(cliparse *cmd, char shortname)
{
	if (hashmap_has_key(cmd->options, &shortname)) {
		return ((cliopt*)hashmap_get_rawptr(cmd->options, &shortname))->values;
	} else {
		return NULL;
	}
}


const vec *cliparse_opt_val_from_longname(cliparse *cmd, char *longname)
{
	if (hashmap_has_key(cmd->long_to_short, &longname)) {
		char shortname = hashmap_get_char(cmd->long_to_short, &longname);
		return ((cliopt*)hashmap_get_rawptr(cmd->options, &shortname))->values;
	} else {
		return NULL;
	}
}


const vec *cliparse_arg_val_from_pos(cliparse *cmd, size_t pos)
{
	if (pos < vec_len(cmd->args)) {
		return ((cliarg *)vec_get_rawptr(cmd->args, pos))->values;
	} else {
		return NULL;
	}
}

