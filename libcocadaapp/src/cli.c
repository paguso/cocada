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
#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include "arrays.h"
#include "cli.h"
#include "cstrutil.h"
#include "hash.h"
#include "hashmap.h"
#include "iter.h"
#include "mathutil.h"
#include "new.h"
#include "strbuf.h"
#include "vec.h"
#include "errlog.h"


struct _cliopt {
	char shortname;		/* Short option name e.g. f (-f) */
	char *longname;		/* Long option name e.g. foo (--foo) */
	char *help;			/* Help description */
	clioptneed need;	/* true=mandatory argument(default); false=optional */
	bool sc;			/* Is short-circuit option? */
	clioptmultiplicity
	multi;				/* true=can be declared multiple times, false=can appear only once(default) */
	cliargtype type;	/* The type of option values */
	int min_val_no;		/* Minimum number of option values (default = 0) */
	int max_val_no;		/* Maximum number of option values (default = 0) */
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


typedef struct {
	clioptcombotype type;
	size_t size;
	char shortnames[32];
} optcombo;


struct _cliparser {
	struct _cliparser *par;		/* parent command (for subcommands) */
	char *name;					/* command name */
	char *help;					/* help message */
	hashmap *subcommands;		/* subcommands indexed by name */
	vec *subcmd_names;			/* subcommand names by order of addition */
	cliparser *active_subcmd;	/* used subcommand in a command call */
	hashmap *options;			/* command options indexed by short name */
	vec *optcombos;				/* Option combos */
	hashmap *long_to_short;		/* Long-to-short option name map */
	cliopt *active_sc_opt;
	vec *args;					/* Vector of positional arguments */
	bool parsed;				/* has this been processed/parsed yet? */
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
			ret = vec_new(sizeof(char *));
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
		DESTROY_FLAT(vals, vec);
		break;
	case ARG_BOOL:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             FNR(vec))));
			break;
		case OPT_SINGLE:
			DESTROY_FLAT(vals, vec);
			break;
		}
		break;
	case ARG_CHAR:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             FNR(vec))));
			break;
		case OPT_SINGLE:
			DESTROY_FLAT(vals, vec);
			break;
		}
		break;
	case ARG_INT:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             FNR(vec))));
			break;
		case OPT_SINGLE:
			DESTROY_FLAT(vals, vec);
			break;
		}
		break;
	case ARG_FLOAT:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             FNR(vec))));
			break;
		case OPT_SINGLE:
			DESTROY_FLAT(vals, vec);
			break;
		}
		break;
	case ARG_STR:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             finaliser_cons(FNR(vec),
			                                     finaliser_new_ptr()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_new_ptr()));
			break;
		}
		break;
	case ARG_FILE:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             finaliser_cons(FNR(vec),
			                                     finaliser_new_ptr()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_new_ptr()));
			break;
		}
		break;
	case ARG_DIR:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             finaliser_cons(FNR(vec),
			                                     finaliser_new_ptr()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_new_ptr()));
			break;
		}
		break;
	case ARG_CHOICE:
		switch (multi)	{
		case OPT_MULTIPLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
			                             finaliser_cons(FNR(vec),
			                                     finaliser_new_ptr()))));
			break;
		case OPT_SINGLE:
			DESTROY(vals, finaliser_cons(FNR(vec), finaliser_new_ptr()));
			break;
		}
		break;
	}
}


static  bool _isletter(char c)
{
	return (('A'<=c && c<='Z' ) || ('a'<=c && c<='z' ));
}

static bool _isid(char *id)
{
	bool ok = true;
	ok &= (id!=NULL);
	ok &= (*id!='\0');
	ok &= ( ('A'<=*id && *id<='Z' ) || ('a'<=*id && *id<='z' ));
	for (char c=*id; ok && c!='\0'; c=*(id++)) {
		ok &= ( ('A'<=c && c<='Z' ) || ('a'<=c && c<='z' ) || ('0'<=c && c<='9' )
		        || (c=='_') || (c=='-'));
	}
	return ok;
}

static bool _arechoices(vec *choices)
{
	bool ok = true;
	ok &= (choices != NULL);
	ok &= (vec_len(choices) > 0);
	for (size_t i=0, l=vec_len(choices); ok && (i < l); i++) {
		char *id = vec_get_cstr(choices, i);
		ok &= _isid(id);
	}
	return ok;
}


static bool _str_eq(const void *s1, const void *s2)
{
	return strcmp(*((char **)s1), *((char **)s2))==0;
}


/*
 * Validates NON-NULL default values. If type==ARG_CHOICE, choices contains
 * the valid alternatives
 */
static bool _validate_defaults(vec *defaults, cliargtype type, vec *choices)
{
	switch (type) {
	case ARG_NONE:
		break;
	case ARG_CHAR:
		assert(vec_typesize(defaults)==sizeof(char));
		for (size_t i=0, l=vec_len(defaults); i<l; i++) {
			char c = vec_get_char(defaults, i);
			assert(isprint(c));
		}
		break;
	case ARG_BOOL:
		assert(vec_typesize(defaults)==sizeof(bool));
		break;
	case ARG_INT:
		assert(vec_typesize(defaults)==sizeof(long));
		break;
	case ARG_FLOAT:
		assert(vec_typesize(defaults)==sizeof(double));
		break;
	case ARG_STR:
		assert(vec_typesize(defaults)==sizeof(char *));
		for (size_t i=0, l=vec_len(defaults); i<l; i++) {
			char *c = vec_get_cstr(defaults, i);
			assert(strlen(c)==0 || c[0]!='-');
		}
		break;
	case ARG_FILE:
		assert(vec_typesize(defaults)==sizeof(char *));
		for (size_t i=0, l=vec_len(defaults); i<l; i++) {
			char *c = vec_get_cstr(defaults, i);
			assert(strlen(c)==0 || c[0]!='-');
		}
		break;
	case ARG_DIR:
		assert(vec_typesize(defaults)==sizeof(char *));
		for (size_t i=0, l=vec_len(defaults); i<l; i++) {
			char *c = vec_get_cstr(defaults, i);
			assert(strlen(c)==0 || c[0]!='-');
		}
		break;
	case ARG_CHOICE:
		assert(vec_typesize(defaults)==sizeof(char *));
		for (size_t i=0, l=vec_len(defaults); i<l; i++) {
			char *c = vec_get_cstr(defaults, i);
			assert(strlen(c)==0 || c[0]!='-');
			assert(vec_find(choices, &c, _str_eq) < vec_len(choices));
		}
		break;
	}
	return true;
}


cliopt *cliopt_new(char shortname,  char *longname, char *help,
                   clioptneed need, clioptmultiplicity multiplicity,
                   cliargtype type, int min_val_no, int max_val_no,
                   vec *choices, vec *defaults )
{
	assert( _isletter(shortname) );
	assert( _isid(longname) );
	assert( need==OPT_OPTIONAL || max_val_no!=0 );
	assert( multiplicity==OPT_SINGLE || max_val_no!=0 );
	assert( max_val_no!=0 || type==ARG_NONE);
	assert( type!=ARG_NONE || max_val_no==0);
	assert( type!=ARG_NONE || defaults==NULL);
	assert( 0<=min_val_no && min_val_no <= max_val_no );
	assert( type!=ARG_CHOICE ||_arechoices(choices) );
	assert( defaults==NULL ||
	        (  min_val_no <= vec_len(defaults)
	           && vec_len(defaults) <= max_val_no
	           && _validate_defaults(defaults, type, choices) ) );
	cliopt *ret = NEW(cliopt);
	ret->shortname = shortname;
	ret->longname = (longname) ? cstr_clone(longname) : NULL;
	WARN_ASSERT(help!=NULL && strlen(help)>0, "CLI: Adding option -%c with empty description.", shortname);
	ret->help = (help) ? cstr_clone(help) : NULL;
	ret->need = need;
	ret->sc = false;
	ret->multi = multiplicity;
	ret->type = type;
	ret->min_val_no = min_val_no;
	ret->max_val_no = max_val_no;
	ret->choices = choices;
	ret->defaults = defaults;
	ret->values = _vals_vec_new(multiplicity, type);
	return ret;
}


cliopt *cliopt_new_defaults(char shortname, char *longname, char *help)
{
	return cliopt_new(shortname, longname, help, 
		OPT_OPTIONAL, OPT_SINGLE, ARG_NONE, 0, 0, NULL, NULL);
}


cliopt *cliopt_new_sc(char shortname,  char *longname, char *help,
                   cliargtype type, int min_val_no, int max_val_no,
                   vec *choices, vec *defaults )
{
	cliopt *ret = cliopt_new(shortname, longname, help, 
		OPT_OPTIONAL, OPT_SINGLE, type, min_val_no, max_val_no, choices, defaults);
	ret->sc = true;
	return ret;
}


cliopt *cliopt_new_sc_defaults(char shortname,  char *longname, char *help)
{
	return cliopt_new_sc(shortname, longname, help, ARG_NONE, 0, 0, NULL, NULL);
}



void cliopt_finalise(void *ptr, const finaliser *fnr)
{
	cliopt *opt = (cliopt *)ptr;
	FREE(opt->longname);
	FREE(opt->help);
	DESTROY(opt->choices, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	switch (opt->type) {
	case ARG_STR:
	case ARG_FILE:
	case ARG_DIR:
	case ARG_CHOICE:
		DESTROY(opt->defaults, finaliser_cons(FNR(vec), finaliser_new_ptr()));
		break;
	default:
		DESTROY_FLAT(opt->defaults, vec);
		break;
	}
	_vals_vec_free(opt->values, opt->multi, opt->type);
}


const char cliopt_shortname(const cliopt *opt)
{
	return opt->shortname;
}


cliarg *cliarg_new(char *name, char *help, cliargtype type)
{
	cliarg *ret = NEW(cliarg);
	ret->name = (name) ? cstr_clone(name) : cstr_clone("Unnamed");
	WARN_ASSERT(help!=NULL && strlen(help)>0, "CLI: Adding argument '%s' with empty description.", ret->name);
	ret->help = (help) ? cstr_clone(help) : cstr_new(0);
	ret->type = type;
	ret->single_val = true;
	ret->values = _vals_vec_new(false, ret->type);
	return ret;
}


cliarg *cliarg_new_multi(char *name, char *help, cliargtype type)
{
	cliarg *ret = cliarg_new(name, help, type);
	ret->single_val = false;
	return ret;
}


cliarg *cliarg_new_choice(char *name, char *help, vec *choices)
{
	assert( _arechoices(choices) );
	cliarg *ret = cliarg_new(name, help, ARG_CHOICE);
	ret->choices = choices;
	return ret;
}


cliarg *cliarg_new_choice_multi(char *name, char *help, vec *choices)
{
	cliarg *ret = cliarg_new_choice(name, help, choices);
	ret->single_val = false;
	return ret;
}


void cliarg_finalise(void *ptr, const finaliser *fnr)
{
	cliarg *arg = (cliarg *)ptr;
	FREE(arg->name);
	FREE(arg->help);
	DESTROY(arg->choices, finaliser_cons(FNR(vec), finaliser_new_ptr()));
	_vals_vec_free(arg->values, false, arg->type);
}


static uint64_t _hash_str(const void *ptr)
{
	char *s = *((char **)ptr);
	return fnv1a_64bit_hash(s, strlen((char *)s));
}


cliparser *cliparser_new(char *name, char *help)
{
	assert(_isid(name));
	cliparser *ret = NEW(cliparser);
	ret->par = NULL;
	ret->name = cstr_clone(name);
	ret->help = (help)?cstr_clone(help):cstr_new(0);
	ret->subcommands = hashmap_new(sizeof(char *), sizeof(cliparser *),  _hash_str,
	                               _str_eq);
	ret->subcmd_names = vec_new(sizeof(char *));
	ret->active_subcmd = NULL;
	ret->options = hashmap_new(sizeof(char), sizeof(cliopt *), ident_hash_char,
	                           eq_char);
	ret->optcombos = vec_new(sizeof(optcombo));
	ret->long_to_short = hashmap_new(sizeof(char *), sizeof(char), _hash_str,
	                                 _str_eq);
	ret->active_sc_opt = NULL;
	ret->args = vec_new(sizeof(cliarg *));
	// add help option directly
	cliopt *help_opt = cliopt_new_sc_defaults('h', "help", "Prints help message");
	cliparser_add_option(ret, help_opt);
	ret->parsed = false;
	return ret;
}


void cliparser_finalise(void *ptr, const finaliser *fnr)
{
	cliparser *clip = (cliparser *)ptr;
	DESTROY(clip->subcommands, finaliser_cons(finaliser_cons(FNR(hashmap),
	        finaliser_new_empty()),
	        finaliser_cons(finaliser_new_ptr(), FNR(cliparser))));
	DESTROY_FLAT(clip->subcmd_names, vec);
	DESTROY(clip->options, finaliser_cons(finaliser_cons(FNR(hashmap),
	                                      finaliser_new_empty()),
	                                      finaliser_cons(finaliser_new_ptr(), FNR(cliopt))));
	DESTROY_FLAT(clip->long_to_short, hashmap);
	DESTROY(clip->args, finaliser_cons(FNR(vec), finaliser_cons(finaliser_new_ptr(),
	                                   FNR(cliarg))));
	DESTROY_FLAT(clip->optcombos, vec);
	FREE(clip->name);
	FREE(clip->help);
}


const char *cliparser_name(const cliparser *cmd)
{
	return cmd->name;
}


const cliparser *cliparser_active_subcommand(const cliparser *cmd)
{
	return cmd->active_subcmd;
}


const cliopt *cliparser_active_sc_option(const cliparser *cmd)
{
	return cmd->active_sc_opt;
}


static bool _not_a_subcmd_name(char *id, cliparser *cmd)
{
	for (cliparser *cur_cmd = cmd; cur_cmd != NULL; cur_cmd = cmd->par) {
		ERROR_ASSERT(strcmp(id, cmd->name)
		             && !hashmap_has_key(cur_cmd->subcommands, &id),
		             "Value '%s' is not allowed because it is a (sub)command name.\n", id);
	}
	return true;
}


void cliparser_add_subcommand(cliparser *cmd,  cliparser *subcmd)
{
	assert(cmd->par==NULL); // subcommand cannot have subcommands
	assert(cmd!=subcmd);	// avoid cycles
	assert(subcmd->par==NULL); // a subcommand gets added at most once
	assert(!hashmap_has_key(cmd->subcommands,
	                        &subcmd->name)); // all subcommand names are unique
	// assert that no subcmd option choice is equal to a subcommand name in cmd
	hashmap_iter *it = hashmap_get_iter(subcmd->options);
	FOREACH_IN_ITER(optentry, hashmap_entry, hashmap_iter_as_iter(it)) {
		cliopt *opt = *((cliopt **)(optentry->val));
		if (opt->type == ARG_CHOICE) {
			for (size_t i=0, l=vec_len(opt->choices); i<l; i++) {
				char *ch = vec_get_cstr(opt->choices, i);
				assert(_not_a_subcmd_name(ch, cmd));
			}
		}
		if (opt->defaults!=NULL && ( opt->type==ARG_STR || opt->type==ARG_FILE
		                             || opt->type==ARG_DIR || opt->type==ARG_CHOICE ) ) {
			for (size_t i=0, l=vec_len(opt->defaults); i<l; i++) {
				char *val = vec_get_cstr(opt->defaults, i);
				assert(_not_a_subcmd_name(val, cmd));
			}
		}
	}
	FREE(it);
	subcmd->par = cmd;
	vec_push_cstr(cmd->subcmd_names, subcmd->name);
	hashmap_set(cmd->subcommands, &(subcmd->name), &subcmd);
}


void cliparser_add_option(cliparser *cmd, cliopt *opt)
{
	assert( !hashmap_has_key(cmd->options, &(opt->shortname)) );
	assert( opt->longname==NULL
	        || !hashmap_has_key(cmd->long_to_short, &(opt->longname)) );
	if (opt->type == ARG_CHOICE) {
		for (size_t i=0, l=vec_len(opt->choices); i<l; i++) {
			char *ch = vec_get_cstr(opt->choices, i);
			assert(_not_a_subcmd_name(ch, cmd));
		}
	}
	if (opt->defaults!=NULL && ( opt->type==ARG_STR || opt->type==ARG_FILE
	                             || opt->type==ARG_DIR || opt->type==ARG_CHOICE ) ) {
		for (size_t i=0, l=vec_len(opt->defaults); i<l; i++) {
			char *val = vec_get_cstr(opt->defaults, i);
			assert(_not_a_subcmd_name(val, cmd));
		}
	}
	hashmap_set(cmd->options, &(opt->shortname), &opt);
	if (opt->longname) {
		hashmap_set(cmd->long_to_short, &(opt->longname), &(opt->shortname));
	}
}


void cliparser_add_option_combo(cliparser *cmd, clioptcombotype type, size_t n, ...)
{
	optcombo c;
	c.type = type;
	c.size = 0;
	va_list valist;
	va_start(valist, n);
	for (size_t i = 0; i < n; i++) {
		cliopt *opt = va_arg(valist, cliopt *);
		c.shortnames[c.size++] = opt->shortname;
	}
	va_end(valist);
	c.shortnames[c.size] = '\0'; 
	vec_push(cmd->optcombos, &c);
}


void cliparser_add_pos_arg(cliparser *cmd, cliarg *arg)
{
	assert( arg->single_val ||
	        vec_len(cmd->args) == 0 ||
	        ((cliarg *)vec_last_rawptr(cmd->args))->single_val );
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
		strbuf_nappend(typedescr, " <", 2);
		if (opt->type==ARG_CHOICE) {
			strbuf_join(typedescr, vec_len(opt->choices),
			            (const char **)vec_as_array(opt->choices), "|");
		} else {
			strbuf_nappend(typedescr, type_lbl[opt->type], strlen(type_lbl[opt->type]));
		}
		strbuf_append_char(typedescr, '>');
		if (opt->min_val_no==1 && opt->max_val_no==1) {
		} else if (opt->min_val_no==0 && opt->max_val_no==ARGNO_UNLIMITED) {
			strbuf_append_char(typedescr, '*');
		} else if (opt->min_val_no==1 && opt->max_val_no==ARGNO_UNLIMITED) {
			strbuf_append_char(typedescr,'+');
		} else {
			strbuf_append_char(typedescr, '[');
			uint_to_cstr(nbstr, opt->min_val_no, 10);
			strbuf_nappend(typedescr, nbstr, strlen(nbstr));
			if (opt->min_val_no != opt->max_val_no) {
				strbuf_nappend(typedescr, "..", 2);
				if (opt->max_val_no != ARGNO_UNLIMITED) {
					uint_to_cstr(nbstr, opt->max_val_no, 10);
					strbuf_nappend(typedescr, nbstr, strlen(nbstr));
				}
			}
			strbuf_append_char(typedescr, ']');
		}
		printf("%s", strbuf_as_str(typedescr));
		FREE(nbstr);
		strbuf_free(typedescr);
	}
	printf("\t%s\n", (opt->help) ? opt->help : "");
}



void cliparser_print_help(const cliparser *cmd)
{
	bool has_options = hashmap_size(cmd->options) > 0;
	bool has_args = vec_len(cmd->args) > 0;
	bool has_subcmds = hashmap_size(cmd->subcommands) > 0;
	printf("Usage:\n\n");
	printf("  %s%s%s", (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"",
	       cmd->name);
	if (has_options) {
		printf(" <options>");
	}
	for (size_t i=0, l=vec_len(cmd->args); i < l; i++) {
		cliarg *arg = (cliarg *)vec_get_rawptr(cmd->args, i);
		printf(" <%s%s>", arg->name, (arg->single_val)?"":"...");
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
			cliopt *opt = (cliopt *) hashmap_get_rawptr(cmd->options, vec_get(shortnames,
			              i));
			_cliopt_print_help(opt);
		}
		DESTROY_FLAT(shortnames, vec);
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
			char *subcmd_name = vec_get_cstr(cmd->subcmd_names, i);
			cliparser *subcmd = (cliparser *)hashmap_get_rawptr(cmd->subcommands,
			                    &subcmd_name);
			printf("  %s\t%s\n",subcmd->name, (subcmd->help)?subcmd->help:"");
		}
		printf("\n  Run \"%s%s%s <subcommand> --help\" for help on a specific subcommand.\n",
		       (cmd->par)?(cmd->par->name):"", (cmd->par)?" ":"", cmd->name);
	}
}


/*
 * Checks to see if required options were not declared.
 * AND
 * Attribute default values for undeclared non-required options when available.
 */
static cliparse_res _check_missing_options(cliparser *cmd)
{
	cliparse_res result = {.ok = true, .res.ok = cmd};

	strbuf *longname = strbuf_new_with_capacity(16);
	hashmap_iter *opt_it = hashmap_get_iter(cmd->options);
	FOREACH_IN_ITER(entry, hashmap_entry, hashmap_iter_as_iter(opt_it)) {
		cliopt *opt = *((cliopt **)(entry->val));
		strbuf_clear(longname);
		if (opt->longname) {
			strbuf_nappend(longname, ", --", 4);
			strbuf_nappend(longname, opt->longname, strlen(opt->longname));
		}
		if ( opt->need == OPT_REQUIRED && (opt->values == NULL || vec_len(opt->values) == 0 ) ) {
			result.ok = false;
			result.res.err.code = MISSING_REQ_OPT;
			snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Undefined required option -%c%s.", opt->shortname, strbuf_as_str(longname));
			goto cleanup;
		}
		if (opt->need == OPT_OPTIONAL && vec_len(opt->values)==0
		        && opt->defaults!=NULL) {
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
				for (size_t i=0, l=vec_len(opt->defaults); i<l; i++) {
					char *val = vec_get_cstr(opt->defaults, i);
					vec_push_rawptr(vals, cstr_clone(val));
				}
				break;
			case ARG_FILE:
				for (size_t i=0, l=vec_len(opt->defaults); i<l; i++) {
					char *val = vec_get_cstr(opt->defaults, i);
					vec_push_rawptr(vals, cstr_clone(val));
				}
				break;
			case ARG_DIR:
				for (size_t i=0, l=vec_len(opt->defaults); i<l; i++) {
					char *val = vec_get_cstr(opt->defaults, i);
					vec_push_rawptr(vals, cstr_clone(val));
				}
				break;
			case ARG_CHOICE:
				for (size_t i=0, l=vec_len(opt->defaults); i<l; i++) {
					char *val = vec_get_cstr(opt->defaults, i);
					vec_push_rawptr(vals, cstr_clone(val));
				}
				break;
			}
			switch (opt->multi) {
			case OPT_MULTIPLE:
				vec_push(opt->values, &vals);
				break;
			case OPT_SINGLE:
				vec_cat(opt->values, vals);
				DESTROY_FLAT(vals, vec);
				break;
			}
		}
	}
	result.ok = true;
	result.res.ok = cmd;
cleanup:
	FREE(opt_it);
	strbuf_free(longname);
	return result;
}


static cliparse_res _check_option_combos(cliparser *cmd)
{
	cliparse_res result = {.ok = true, .res.ok = cmd};

	for (size_t i = 0, l = vec_len(cmd->optcombos); i < l; i++) {
		optcombo *c = (optcombo *) vec_get(cmd->optcombos, i);
		size_t nused = 0;
		for (size_t j = 0; j<c->size; j++) {
			char shortname = c->shortnames[j];
			if (hashmap_has_key(cmd->options, &shortname)) {
				nused++;
			}
		}	
		if (c->type == ALL_OF && nused != c->size) {
			result.ok = false;
			result.res.err.code = INVALID_OPT_COMBO;
			snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
				"Invalid option usage: all of '%s' must be declared.", 
				c->shortnames);
			break;
		}
		else if (c->type == ONE_OF && nused != 1) {
			result.ok = false;
			result.res.err.code = INVALID_OPT_COMBO;
			snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
				"Invalid option usage: only one of '%s' must be declared.", 
				c->shortnames);
			break;
		} 
		else if (c->type == ALL_IF_ANY && nused != 0  && nused != c->size) {
			result.ok = false;
			result.res.err.code = INVALID_OPT_COMBO;
			snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
				"Invalid option usage: all or none of '%s' must be declared.", 
				c->shortnames);
			break;
		}
		else if (c->type == ONE_IF_ANY && nused != 0  && nused != 1) {
			result.ok = false;
			result.res.err.code = INVALID_OPT_COMBO;
			snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
				"Invalid option usage: at most one of '%s' must be declared.", 
				c->shortnames);
			break;
		}
	}
	return result;
}


/*
 * Try to parse and validade tok as a value of a given type and add it to vals.
 * If type=ARGV_CHOICE, a non-null choices vector must be informed.
 * Returns whether parsing went OK.
 */
static bool _parse_value(vec *vals, char *tok, cliargtype type, vec *choices,
                         cliparser *cmd)
{
	switch (type) {
	case ARG_NONE:
		return false;
		break;
	case ARG_BOOL:
		if (cstr_equals_ignore_case(tok, "true") || cstr_equals(tok, "1")) {
			vec_push__Bool(vals, true);
		} else if (cstr_equals_ignore_case(tok, "false") || cstr_equals(tok, "0")) {
			vec_push__Bool(vals, false);
		} else {
			return false;
		}
		break;
	case ARG_CHAR:
		if (strlen(tok) != 1) {
			return false;
		}
		vec_push_char(vals, *tok);
		break;
	case ARG_INT:
		;
		char *endptr = NULL;
		long lval = strtol(tok, &endptr, 10);
		if ( (errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN))
		        || (errno != 0 && lval == 0) || endptr == tok || *endptr != '\0') {
			return false;
		}
		vec_push_long(vals, lval);
		break;
	case ARG_FLOAT:
		;
		endptr = NULL;
		double dval = strtod(tok, &endptr);
		if ( (errno == ERANGE && (dval ==  HUGE_VALF || dval == HUGE_VALL))
		        || (errno != 0 && dval == 0) || endptr == tok || *endptr != '\0') {
			return false;
		}
		vec_push_double(vals, dval);
		break;
	case ARG_STR:
		for (cliparser *cur_cmd = cmd; cur_cmd!=NULL; cur_cmd = cur_cmd->par) {
			if (tok[0] == '-' || hashmap_has_key(cur_cmd->subcommands, &tok)) {
				return false;
			}
		}
		char *sval = cstr_clone(tok);
		vec_push(vals, &sval);
		break;
	case ARG_FILE:
		for (cliparser *cur_cmd = cmd; cur_cmd!=NULL; cur_cmd = cur_cmd->par) {
			if (tok[0] == '-' || hashmap_has_key(cur_cmd->subcommands, &tok)) {
				return false;
			}
		}
		char *fval = cstr_clone(tok);
		vec_push(vals, &fval);
		break;
	case ARG_DIR:
		for (cliparser *cur_cmd = cmd; cur_cmd!=NULL; cur_cmd = cur_cmd->par) {
			if (tok[0] == '-' || hashmap_has_key(cur_cmd->subcommands, &tok)) {
				return false;
			}
		}
		char *pval = cstr_clone(tok);
		vec_push(vals, &pval);
		break;
	case ARG_CHOICE:
		assert(choices);
		if (vec_find(choices, &tok, _str_eq) == vec_len(choices)) {
			return false;
		}
		vec_push_cstr(vals, cstr_clone(tok));
		break;
	}
	return true;
}


typedef enum {
	PS_CMD = 0,
	PS_VAL = 1,
} parse_state;


cliparse_res cliparser_parse(cliparser *clip, int argc, char **argv, bool exit_on_error)
{
	//cliparse_res result = {.ok = true, .res.ok = clip};
	cliparse_res result = {0};

	WARN_IF( clip->parsed, "This CLI has already been processed!"
	         "Parsing the CLI more than once may cause unexpected errors!\n"
	         "Try resetting or creating a new parser." );
	clip->parsed = true;

	int t = 1;
	parse_state state = PS_CMD;
	cliparser *cur_parse = clip;
	int cur_opt_pos = -1;
	cliopt *cur_opt = NULL;
	int cur_arg_no = 0;
	vec *cur_vals = NULL;

	char *tok;
	int toklen;
	while (t < argc) {
		tok = argv[t];
		toklen = strlen(tok);

		if (state == PS_CMD) {
			// try fetch valid option
			if (tok[0] == '-') { // option
				char shortname = '\0';
				if (toklen <= 1) {
					result.ok = false;
					result.res.err.code = INVALID_OPTION;
					snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Invalid empty option name - at position %d.", t);
					goto cleanup;	
				}
				if (tok[1] == '-') { // long option
					char *longname = &tok[2];
					if (!_isid(longname)) {
						result.ok = false;
						result.res.err.code = INVALID_OPTION;
						snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Invalid option name %s at position %d.", tok, t);
						goto cleanup;	
					}
					if (!hashmap_has_key(cur_parse->long_to_short, &longname)) {
						result.ok = false;
						result.res.err.code = INVALID_OPTION;
						snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Unknown option %s at position %d.", tok, t);
						goto cleanup;	
					}
					shortname = hashmap_get_char(cur_parse->long_to_short, &longname);
				} 
				else { // short option
					if (toklen != 2) {
						result.ok = false;
						result.res.err.code = INVALID_OPTION;
						snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Invalid option short name %s at position %d.", tok, t);
						goto cleanup;	
					} 
					if (!_isletter(tok[1])) {
						result.ok = false;
						result.res.err.code = INVALID_OPTION;
						snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Invalid option name %s at position %d.", tok, t);
						goto cleanup;	
					}
					shortname = tok[1];
				}
				if (!hashmap_has_key(cur_parse->options, &shortname)) {
					result.ok = false;
					result.res.err.code = INVALID_OPTION;
					snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Unknown option %s of (sub)command %s at position %d.", tok, cur_parse->name, t);
					goto cleanup;	
				}
				cur_opt = hashmap_get_rawptr(cur_parse->options, &shortname);
				// help option used
				/*if (cur_opt->shortname == 'h') {
					cliparser_print_help(cur_parse);
					result.ok = true;
					result.res.ok = clip;
					goto cleanup;
				}*/
				if ( cur_opt->multi!=OPT_MULTIPLE && vec_len(cur_opt->values)>0 ) {
					result.ok = false;
					result.res.err.code = INVALID_MULT_OPTION;
					snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Invalid multiple declaration of option %s at position %d.", tok, t);
					goto cleanup;
				}
				// valid option found
				if (cur_opt->max_val_no > 0) {
					if (t + 1 >= argc) {
						result.ok = false;
						result.res.err.code = INVALID_OPT_VAL_NO;
						snprintf( result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
								  "Option %s at positon %d requires %s%d value%s of type <%s>, but none given.",
					              argv[t], t, (cur_opt->min_val_no==cur_opt->max_val_no)?"":"at least ",
					              cur_opt->min_val_no, (cur_opt->min_val_no==1)?"":"s", type_lbl[cur_opt->type] );
						goto cleanup;		
					}
					cur_vals = _vals_vec_new(OPT_SINGLE, cur_opt->type);
					state = PS_VAL;
				} else { // no values, boolean assumed
					if (vec_len(cur_opt->values)) {
						vec_set__Bool(cur_opt->values, 0, true);
					} else {
						vec_push__Bool(cur_opt->values, true);
					}
					// stop after parsing first sc option
					if (cur_opt->sc) {
						cur_parse->active_sc_opt = cur_opt;
						goto success;
					}
				}
				t++;
			} 
			else if (hashmap_has_key(cur_parse->subcommands, &tok)) { // valid subcommand
				if ( vec_len(cur_parse->args)!=0 &&
				 	 ( cur_arg_no != 0 || vec_len(((cliarg *)vec_first_rawptr(cur_parse->args))->values) > 0) ) {
					result.ok = false;
					result.res.err.code = INVALID_SUBCMD;
					snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Subcommand %s at position %d cannot be called after arguments to the root command have been given.", tok, t );
					goto cleanup;
				}
				cliparser *subcmd = hashmap_get_rawptr(cur_parse->subcommands, &tok);
				cur_parse->active_subcmd = subcmd;
				cur_parse = subcmd;
				cur_arg_no = 0;
				t++;
			} 
			else {
				// see if it is a correct positional argument
				if ( cur_arg_no >= vec_len(cur_parse->args) ) {
					result.ok = false;
					result.res.err.code = UNEXPECTED_TK;
					snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, "Unexpected token '%s' at position %d.", tok, t);
					goto cleanup;
				}
				cliarg *cur_arg = (cliarg *)vec_get_rawptr(cur_parse->args, cur_arg_no);
				bool is_arg = _parse_value(cur_arg->values, tok, cur_arg->type,
				                           cur_arg->choices, cur_parse);
				if (!is_arg) {
					result.ok = false;
					result.res.err.code = INVALID_ARG_VAL;
					snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
						"Wrong value '%s' for argument #%d of type <%s> at position %d.",
						tok, cur_arg_no, type_lbl[cur_arg->type], t );
					goto cleanup;
				}
				if (cur_arg->single_val) cur_arg_no++;
				t++;
			}
		} else if (state == PS_VAL) {   
			// can only be here if expecting values for a valued option
			bool val_ok = _parse_value(cur_vals, tok, cur_opt->type, cur_opt->choices,
			                           cur_parse);
			bool max_val_no_reached =  val_ok
			                           && ( vec_len(cur_vals) == cur_opt->max_val_no );
			bool eof_argv = val_ok && ((t+1)==argc);
			if (!val_ok || max_val_no_reached || eof_argv) {
				if ( cur_opt->min_val_no > vec_len(cur_vals) ) {
					result.ok = false;
					result.res.err.code = INVALID_OPT_VAL_NO;
					snprintf( result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
				        "Too few values for option %s at positon %d. %s%d value%s of type <%s> required, but %zu given.",
				        argv[cur_opt_pos], cur_opt_pos,
				        (cur_opt->min_val_no==cur_opt->max_val_no)?"":"At least ",
				        cur_opt->min_val_no, (cur_opt->min_val_no==1)?"":"s",
				        type_lbl[cur_opt->type], vec_len(cur_vals) );
					goto cleanup;
				}
				if ( vec_len(cur_vals) > cur_opt->max_val_no ) {
					result.ok = false;
					result.res.err.code = INVALID_OPT_VAL_NO;
					snprintf( result.res.err.msg, CLIPARSE_ERROR_BUFSZ,
				    	"Too many values for option %s at positon %d. %s%d value%s of type <%s> allowed, but %zu given.",
				        argv[cur_opt_pos], cur_opt_pos,
				        (cur_opt->min_val_no==cur_opt->max_val_no)?"":"At most ",
				        cur_opt->max_val_no, (cur_opt->max_val_no==1)?"":"s",
				        type_lbl[cur_opt->type], vec_len(cur_vals) );
					goto cleanup;
				}
				// finished obtaining option values
				switch (cur_opt->multi) {
				case OPT_SINGLE:
					vec_cat(cur_opt->values, cur_vals);
					DESTROY_FLAT(cur_vals, vec);
					cur_vals = NULL;
					break;
				case OPT_MULTIPLE:
					vec_push_rawptr(cur_opt->values, &cur_vals);
					cur_vals = NULL;
					break;
				}
				// stop after parsing first sc option
				if (cur_opt->sc) {
					cur_parse->active_sc_opt = cur_opt;
					goto success;
				}
				state = PS_CMD;
			}
			if (val_ok) {
				t++;
			}
		} else {
			result.ok = false;
			result.res.err.code = UNDEF_ERR;
			snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ, 
				"Uh-oh: undefined error parsing command arguments at position %d (WRONG STATE).", t);
			goto cleanup;
		}
	}
	// check if some arguments undefined
	if ( (cur_arg_no != vec_len(cur_parse->args)) &&
	     ( vec_len(cur_parse->args) == 0 || cur_arg_no != vec_len(cur_parse->args)-1
	       || vec_len(((cliarg *)vec_last_rawptr(cur_parse->args))->values) == 0 ) ) {
		result.ok = false;
		result.res.err.code = INVALID_ARG_VAL_NO;
		snprintf(result.res.err.msg, CLIPARSE_ERROR_BUFSZ,
	        "Missing value for argument #%d (%s) of type <%s>.",
	        cur_arg_no, ((cliarg *)vec_get_rawptr(cur_parse->args, cur_arg_no))->help,
	        type_lbl[((cliarg *)vec_get_rawptr(cur_parse->args, cur_arg_no))->type]);
		goto cleanup;
	}
	// check if mandatory options with no default values undefined
	result = _check_option_combos(clip);
	if (!result.ok) {
		goto cleanup;
	}
	// check option combos
	result = _check_option_combos(clip);
	if (!result.ok) {
		goto cleanup;
	}
	if (clip->active_subcmd) {
		result = _check_missing_options(clip->active_subcmd);
		if (!result.ok) {
			goto cleanup;
		}
		result = _check_option_combos(clip->active_subcmd);
		if (!result.ok) {
			goto cleanup;
		}
	}
success:
	result.ok = true;
	result.res.ok = clip;
cleanup:
	if (exit_on_error && !result.ok) {
		ERROR("%s (code=%d)\n", result.res.err.msg, result.res.err.code);
		exit(1);
	}
	return result;
}


const vec *cliparser_opt_val_from_shortname(const cliparser *cmd, char shortname)
{
	if (hashmap_has_key(cmd->options, &shortname)) {
		return ((cliopt *)hashmap_get_rawptr(cmd->options, &shortname))->values;
	} else {
		return NULL;
	}
}


const vec *cliparser_opt_val_from_longname(const cliparser *cmd, char *longname)
{
	if (hashmap_has_key(cmd->long_to_short, &longname)) {
		char shortname = hashmap_get_char(cmd->long_to_short, &longname);
		return ((cliopt *)hashmap_get_rawptr(cmd->options, &shortname))->values;
	} else {
		return NULL;
	}
}


const vec *cliparser_arg_val_from_pos(const cliparser *cmd, size_t pos)
{
	if (pos < vec_len(cmd->args)) {
		return ((cliarg *)vec_get_rawptr(cmd->args, pos))->values;
	} else {
		return NULL;
	}
}