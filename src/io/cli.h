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
#include "new.h"

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
 * Some validation can be performed on option values by setting types 
 * to these values. Option types are chosen from the enum type ::cliargtype. 
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
 * grep -r cocada *.c
 * ```
 * The two positional arguments are the pattern string `cocada`, and
 * the second is a list of files where this pattern is to be searched
 * for. So the last argument must admit multiple values.
 * 
 * The values of the positional arguments are implemented with the
 * language types defined in the table above
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
 * The alternatives, meaning and corresponding language-types are given in the 
 * following table.
 * 
 *  Enum cliargtype  | Description                                     | Language type      
 *  -----------------|-------------------------------------------------|---------------
 *  ARG_NONE         | No type (boolean switch)                        | bool             
 *  ARG_BOOL         | Boolean (true/false)                            | bool               
 *  ARG_CHAR         | Single character                                | char               
 *  ARG_INT          | Integer                                         | long               
 *  ARG_FLOAT        | Floating point                                  | doouble             
 *  ARG_STR          | String literal (no validation)                  | char * (heap)      
 *  ARG_FILE         | File name (description only, no validation)     | char * (heap)
 *  ARG_DIR          | Directory name (descrption only, no validation) | char * (heap)    
 *  ARG_CHOICE       | Finite set of string alternatives               | int (choice #)
 */
typedef enum {
	ARG_NONE = 0,  /* No type, used for valueless options */
	ARG_BOOL = 1,  /* Boolean type */
	ARG_CHAR = 2,  /* Single character */
	ARG_INT = 3,   /* Integer type, stored as long */
	ARG_FLOAT = 4, /* Floating point type, stored as double */
	ARG_STR = 5,   /* Free-form string literal type */
	ARG_FILE = 6,  /* File type (no validation, used for 
						descriptive/cumentation purposes) */
	ARG_DIR = 7,   /* Directory type (no validadtio, used for 
						descriptive/cumentation purposes) */
	ARG_CHOICE = 8 /* Finite set of string alternatives */
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
 * @param multiple		Can the option be declared multiple times?
 * @param type			The type of the option values (ir any)
 * @param min_val_no	The minimum number of option values
 * @param max_val_no	The maximum number of option values. 
 * 						Required: @p min_val_no <= @p max_val_no.
 * 						Use ARGNO_UNLIMITED for an unlimited number ofvalues.
 * @param choices		(**transfer**) Possible choices if @p type = ARG_CHOICE
 * @param defaults		(**transfer**) Default values (PENDING)
 * 
 * @warning
 * The following are required :
 * - @p min_val_no <= @p max_val_no
 * - If @p max_val_no == 0 then @type == ARG_NONE and vice versa (iff)
 * - If @p mandatory == true, then @p max_val_no != 0 (equiv @p type != ARG_NONE)
 * - If @p multiple == true, then @p max_val_no != 0 (equiv @p type != ARG_NONE)
 * - If @type == ARG_CHOICE then @p choices must be a non-empty vector
 * - If @p max_val_no == 0 (equiv @p type == ARG_NONE), then @single == true
 * - If @p mandatory == true, @p defaults is discarded
 * - If @p mandatory == false, @p defaults must have at least @p min_val_no and 
 *   at most @p max_val_no elements
 */
cliopt *cliopt_new_valued(char shortname, char *longname, char *help,
                          bool mandatory, bool multiple, cliargtype type,
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
 * @brief Destructor. 
 * Use with default destructor `DTOR(cliparse)`, or
 * simply `FREE(obj, cliparse)`.
 */
void cliparse_dispose(void *ptr, const dtor *dt);


/**
 * @brief Returns the name of a command parser
 */
const char *cliparse_name(const cliparse *cmd);


/**
 * @brief Returns the invoked subcommand of a command, if any.
 * @return Prior to parsing a call with ::cliparse_parse (cmd), returns NULL.
 * After parsing a call, if a subcommand was called, returns the
 * corresponding (populated) parser, else returns NULL.
 */
const cliparse *cliparse_active_subcommand(const cliparse *cmd);



/**
 * @brief Adds a subprogram (subcommand parser) to a program (command parser).
 */
void cliparse_add_subcommand(cliparse *cmd, cliparse *subcmd);


/**
 * @brief Adds an option to a (sub)program CLI parser.
 * @warning Both the short (`-`) and long (`--`) names of the option must
 * be unique for the command.
 * Trying to add options with existing names will cause an assertion error.
 *  In particular A help option with names 
 * `-h`, `--help` is automatically added to every cliparse. 
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
 * If the call is sucessfully parsed, the @p cmd is populated with the
 * option and argument values, including default values for undeclared 
 * non-required options, if available. If a parse error occurs, an error message
 * id printed to stderr and the program exits.
 * 
 * @param argc The number of tokens (normally received by main())
 * @param argv (*no transfer*) The program call tokens (also received by main())
 
 * The parser assumes @p argv to contain the tokens of a program call in 
 * one of the two following forms. 
 * 1. `command [command options...] [command arguments...]`; or
 * 2. `command [command options...] [subcommand] [subcommand options...] [subcommand arguments...]`
 * So, in particular, currently you CANNOT:
 * - declare command option after the subcommand name
 * - declare an option after an argument
 * 
 * Notice that some calls may be ambiguous, so it is important to know how parsing
 * works. For example, if a command has an option `-a` with 1 to 3 integer values
 * and one argument with multiple integer values, then the call
 * ```
 * command -a 1 2 3 4 5
 * ```
 * would be ambiguous since we don`t know how many of the integer values
 * and how many are argument values. This could be disambiguated by calling
 * for instance
 * ```
 * command 4 5 -a 1 2 3
 * ```
 * but this form is not currently allowed (although this is expected change in
 * the future).
 * 
 * This function handles option values as follows. When an option name is
 * found at position `i`, it then tries to collect as many values as possible 
 * from positions `i+1`, `i+2` ,... until 
 * - The end of @p argv, OR
 * - The maximum number of allowed values are successfully parsed, OR
 * - A type-mismatch occurs.
 * So, in our current example, because `-a` accepts up to 3 values, the
 * call would be parsed as if the values of `-a` were (`1`,`2`,`3`) and 
 * the values of the argument were (`4`,`5`).
 */
void cliparse_parse(cliparse *cmd, int argc, const char **argv);


/**
 * @brief Gets the values of an option from its short name.
 * 
 * @param cmd The (sub)command parser
 * @param shortname The short (`-`) name of the option.
 * 
 * @return If the option is not found, returns NULL.
 * If the option is found, returns its values.
 * The physical type/size of the values will depend on the ::cliargtype type of
 * the option.
 * If the option can be declared multiple types (parameter `multiple==true` of 
 * ::cliopt_new_valued ), then the returned vector is a two-level vector of vectors, 
 * with child vectors containing the values of each declaration of the option. 
 * For example if we call
 * ```
 * command -o 1 2 3 -o 4 5 input.txt
 * ```
 * for an ARG_INT option `-o` with one to three values, then
 * calling this function with @p shortname = `'o'` would return a
 * vector with with two child vectors, the first with three long ints
 * and the second with two long ints.
 */
const vec *cliparse_opt_val_from_shortname(cliparse *cmd, char shortname);


/**
 * @brief Gets the values of an option from its long name.
 * 
 * @param cmd The (sub)command parser
 * @param longname The long (`--`) name of the option.
 * 
 * @see cliparse_opt_val_from_shortname
 * 
 */
const vec *cliparse_opt_val_from_longname(cliparse *cmd, char *longname);


/**
 * @brief Gets the values of a positional argument from its position.
 * 
 * @param cmd The (sub)command parser
 * @param pos The position (0-based) of the argument.
 * 
 * @returns A vector with the values of argument #@p pos. 
 * The physical type/size of  the vector elements is defined by the 
 * ::cliargtype type of the argument.
 * If @p pos is >= the number of arguments, returns NULL.
 * 
 */
const vec *cliparse_arg_val_from_pos(cliparse *cmd, size_t pos);



#endif