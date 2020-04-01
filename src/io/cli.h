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

#ifndef _CLI_H_
#define _CLI_H_

#include <stdbool.h>
#include <stdint.h>

#include "hashmap.h"
#include "vec.h"

typedef enum {
	ARG_NONE = 0,
	ARG_BOOL = 1,
	ARG_CHAR = 2,
	ARG_INT = 3,
	ARG_FLOAT = 4,
	ARG_STR = 5,
	ARG_FILE = 6,
	ARG_DIR = 7,
	ARG_CHOICE = 8
} cliargtype;

static const size_t ARGNO_UNLIMITED = SIZE_MAX;

typedef struct {
	char shortname; /**< Short option name e.g. f (-f) */
	char *longname; /**< Long option name e.g. foo (--foo) */
	char *help; /**< Help description */
	bool mandatory; /**< true=mandatory argument(default); false=optional */
	bool single; /**< true=can appear only once(default), false=can appear multiple times */
	cliargtype type; /**< The type of option values */
	size_t min_val_no; /**< Minimum number of option values (default = 0) */
	size_t max_val_no; /**< Maximum number of option values (default = 0) */
	vec *choices;
	vec *defaults;
	vec *values;
} cliopt;

typedef struct {
	char *name;
	char *help;
	cliargtype type;
	bool single;
	vec *values;
} cliarg;

typedef struct _cliparse {
	struct _cliparse *par;
	char *name;
	char *help;
	vec *subcommands;
	hashmap *options;
	vec *args;
} cliparse;



cliopt *cliopt_new_defaults(char shortname, char *longname, char *help);

cliopt *cliopt_new_valued(char shortname, char *longname, char *help,
                          bool mandatory, bool single, cliargtype type,
                          size_t min_val_no, size_t max_val_no,
                          vec *choices, vec *defaults );


cliarg *cliarg_new(char *name, char*help, cliargtype type);

cliarg *cliarg_new_multi(char *name, char*help, cliargtype type);

cliparse *cliparse_new(char *name, char *help);

void cliparse_add_subcommand(cliparse *cmd, cliparse *subcmd);

void cliparse_add_option(cliparse *cmd, cliopt *opt);

void cliparse_add_pos_arg(cliparse *cmd, cliarg *arg);

void cliparse_print_help(cliparse *cmd);



#endif