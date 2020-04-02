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

/**
 * @file cli.h
 *
 * @brief Command Line Interface (CLI) parsing utilities.
 *
 * @author Paulo Fonseca
 * 
 * This code provides utilities for parsing, validation and
 * documentation of Command Line Interface commands in one of
 * the following simplified forms:
 * 
 * 1. `command [command options...] [command arguments...]`; or
 * 2. `command [command options...] [subcommand] [subcommand options...] [subcommand arguments...]`
 * 
 * where:
 * - `command` stands for the program name
 * - `[command options...]` represents a sequence of zero or more program 
 *    options
 * - `[command arguments...]` represents a sequence of zero or more positional 
 *    arguments (inputs) for the program
 * - `subcommand` is the name of a subprogram that performs a specific task
 * - `[subcommand options...]` represents a sequence of zero or more subprogram 
 *    options
 * - `[subcommand arguments...]` represents a sequence of zero or more 
 *    positional arguments (inputs) for the subprogram
 * 
 * ### Examples
 * 
 * ```
 *  cp -r src/ dest/
 * ```
 * `cp` is the command, `-r` is a command option, and `src/` and `dest/` 
 *  are the positional arguments #0 and #1, respectively
 * 
 * ```
 * git checkout -b devel
 * ```
 * `git` is the command, `checkout` is the subcommand, `-b` is the subcommand
 * option and `devel` is the subprogram argument.
 * 
 * 
 * # Options
 * 
 * Options are switches used to configure the execution of a (sub)program.
 * Every option is declared by a dash '`-`' followed by a distinct *short name* 
 * consisting in a single a alphabetical character (case-sensisitve), e.g
 * `-v`, or alternatively by a double-dash '`--`' followed by a *long name*
 * consisting in a string of alphanumerical characters, e.g. `--verbose`.
 * 
 * An option can have an associated *value*, or more generally, a list
 * of values. An option with no associated value is a boolean
 * switch, whereas a valued option may require one or more  parameters to
 * specify its meaning. Thus every option has a mimimum 
 * and a maximum number of values, 0<=nmin<=nmax<=INFINITY, respectively.
 * Setting nmin=nmax enforces an exact number of values. 
 * 
 * COCADA CLI may perform some validation by setting types to option
 * values, chosen amongst the elements of the enum type cliargtype. 
 * In particular the option ARG_STR accepts any string literal without 
 * performing any previous validation. Currently all values of an option 
 * must be of the same type.
 *  
 * An option can either be *mandatory*, when it must be declared in the
 * program call, or *optional*, when it may be left unspecified. Although
 * not forbidden, it is normally pointless to have a required option
 * with no associated value.
 * 
 * An option can also be allowed to be declared one *single* time,
 * or either *multiple* times. For example, the `gcc -I <include dir>`
 * option can be declared multiple times for specifying several
 * directories where to look for the header files.
 *
 * # Positional arguments
 * 
 * The required inputs to a program are given as a list of positional
 * arguments at the end of the program call. Arguments are typed
 * with the same types as option values. Argument values are validated
 * during the parsing of the program call. Use ARG_STRING for 
 * a string literal type with no validation. In general, evey argument
 * has a single value, however some programs may accept an unbounded
 * list of inputs. So we allow the last positional argument to 
 * have multiple values, with the same restriction as the option
 * values that they must be of the same type. This is useful, 
 * for example, for dealing with wildcard file name expasions made 
 * by the shell. 
 * 
 * ## Example
 * ```
 * grep -r cocada src/*.c
 * ```
 * The two positional arguments are the pattern string `cocada`, and
 * the second is a list of files where this pattern is to be searched
 * for. So the last argument must admit multiple values.
 * 
 *  
 * # Subcommands
 * 
 * If a program performs different tasks, it may be necessary to define
 * many different options, which could make the use of the program
 * confusing. Moreover, many of these options would not make sense for
 * the program as a whole, but rather for a specific operation. 
 * In order to address this issue, a program can be divided in 
 * *subcommands* or *subprograms*, each of which can have its own
 * set of options. The program may still have a set of global
 * options that affect all or at least a significant set of subcommands.
 *  
 */


/**
 * @brief CLI option/argument type
 */
typedef enum {
	ARG_NONE = 0,  /**< No type, used for valueless options */
	ARG_BOOL = 1,  /**< Boolean type */
	ARG_CHAR = 2,  /**< Single character */
	ARG_INT = 3,   /**< Integer type, stored as long */
	ARG_FLOAT = 4, /**< Floating point type, stored as double */
	ARG_STR = 5,   /**< Free-form string literal type */
	ARG_FILE = 6,  /**< File type (no validation, used for 
						descriptive/documentation purposes) */
	ARG_DIR = 7,   /**< Directory type (no validadtio, used for 
						descriptive/documentation purposes) */
	ARG_CHOICE = 8 /**< Finite set of string alternatives */
} cliargtype;

/**
 * @brief Indicates an unlimited number of option/argument values 
 */
#define ARGNO_UNLIMITED  SIZE_MAX

/**
 * @brief CLI Option type (opaque)
 */
typedef struct _cliopt cliopt;

/**
 * @brief CLI Positional argument type (opaque)
 */
typedef struct _cliarg cliarg;

/**
 * @brief CLI parser type (opaque). 
 * Each program or subprogram corresponds to a parser. A program parser
 * contains 
 * - A set of options 
 * - A set of subprogram parsers
 * - A list of positional arguments
 * Currently only one level of program nesting is supported (subprograms 
 * cannot contain sub-subprograms),
 */
typedef struct _cliparse cliparse;


/**
 * @brief Creates a new option with default characteristics.
 * Default characteristics are:
 * - Optional (non-mandatory)
 * - Single usage (can appear at most once per program call)
 * - No associated value (boolean switch).
 * @param shortname (**no transfer**) The one-character distinct name
 * @param longname  (**no transfer**) The multi-character distinct name
 * @param help      (**no transfer**) A short description of the option 
 *                  used for help messages.
 */
cliopt *cliopt_new_defaults(char shortname, char *longname, char *help);


/**
 * @brief Creates a new option. 
 * @param shortname		(**no transfer**) The one-character distinct name
 * @param longname		(**no transfer**) The multi-character distinct name
 * @param help			(**no transfer**) A short description of the option 
 * 						used for help messages.
 * @param mandatory  	Is the option use mandatory?
 * @param single		Can the option be used at most once?
 * @param type			The type of the option values (ir any)
 * @param min_val_no	The minimum number of option values
 * @param max_val_no	The maximum number of option values. 
 * 						Required: @p min_val_no <= @p max_val_no.
 * 						Use ARGNO_UNLIMITED for an unlimited number ofvalues.
 * @param choices		(**transfer**) Possible choices if @p type = ARG_CHOICE
 * @param defaults		(**transfer**) Default values (PENDING)
 */
cliopt *cliopt_new_valued(char shortname, char *longname, char *help,
                          bool mandatory, bool single, cliargtype type,
                          size_t min_val_no, size_t max_val_no,
                          vec *choices, vec *defaults );


/**
 * @brief Creates a new single-valued positional argument.
 * @param name	(**no transfer**) The name of the argument used for 
 * 				descritptive/documentation purposes only.
 * @param help	(**no transfer**) A short description of the argument 
 * 				used for help messages.
 * @param type	The type of the argument.
 */
cliarg *cliarg_new(char *name, char*help, cliargtype type);


/**
 * @brief Creates a new multi-valued positional argument.
 * 		  Only the last positional argument can be multi-valued.
 * @param name	(**no transfer**) The name of the argument used for 
 * 				descritptive/documentation purposes only.
 * @param help	(**no transfer**) A short description of the argument 
 * 				used for help messages.
 * @param type	The type of the argument.
 */
cliarg *cliarg_new_multi(char *name, char*help, cliargtype type);


/**
 * @brief Creates a new (sub)program CLI parser.
 * @param name	(**no transfer**) The name of the (sub)program.
 * @param help	(**no transfer**) A short description of the (sub)program
 * 				used for help messages.
 */ 
cliparse *cliparse_new(char *name, char *help);


/**
 * @brief Adds a subprogram (subcommand parser) to a program (command parser).
 */
void cliparse_add_subcommand(cliparse *cmd, cliparse *subcmd);


/**
 * @brief Adds an option to a (sub)program CLI parser.
 */
void cliparse_add_option(cliparse *cmd, cliopt *opt);


/**
 * @brief Adds a positional argument to a (sub)program CLI parser.
 */
void cliparse_add_pos_arg(cliparse *cmd, cliarg *arg);


/**
 * @brief Prints the automatically-generated help message of a program
 * 			to the standard output.
 */
void cliparse_print_help(cliparse *cmd);


/**
 * @brief Parses a program call.
 */
void cliparse_parse(cliparse *cmd, int argc, char **argv);

#endif