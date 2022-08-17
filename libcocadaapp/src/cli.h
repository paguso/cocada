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

#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stdint.h>

#include "hashmap.h"
#include "vec.h"
#include "new.h"
#include "result.h"


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
 *
 * An option can either be *required*, when it must be declared in the
 * program call, or *optional*, when it may be left unspecified.
 * A required option must have an associated value.
 *
 * An option can also be allowed to be declared one *single* time,
 * or either *multiple* times. For example, the `gcc -I <include dir>`
 * option can be declared multiple times for specifying several
 * directories where to look for the header files.
 *
 * ## Short-circuit (s/c) options
 *
 * A short-circuit option is an option that, when found, causes the CLI
 * parse to be interrupted and other options  and positional arguments to
 * be ignored.  The archeypical example of such an option is the
 * `--help (-h)` option, which ususally signals that the program should
 * print the usage help message and exit.
 * The s/c help function is automatically added to every command.
 * Another very common example is  the `--version (-v)`.
 * A s/c option is meant to be used alone, is always optional and can
 * be used at most once. It can however take values just like other options.
 * As soon as the first s/c option is  found in a program call, the parsing
 * is interrupted and further validations concerning other options and
 * positional arguments are ignored, even though some of them might have
 * already been performed.
 *
 * ## Option combos
 *
 * Sometimes there can be dependencies between options. For example,
 * there can be a situation in which if an option is used, then a
 * complementary option should also be used. COCADA lets such dependencies
 * be specified via "option combos" ::clioptcombotype.
 *
 *
 * # Positional arguments
 *
 * The required inputs to a program are given as a list of positional
 * arguments, usually at the end of the program call. Arguments are typed
 * with the same ::cliargtype types as option values. Argument are validated
 * during the parsing of the program call.  In general, every argument
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
 * In order to address this issue, a program can be organised
 * in *subcommands* or *subprograms*, each of which with its own
 * set of options and arguments. The program may still have a set of global
 * options that affect all or at least a significant set of subcommands.
 *
 * # CLI grammar
 *
 * A command line call accepted by this module will be parsed according
 * to the following grammar.
 *
 * ```
 * <command> ::= <command name> <options or args> |
 *          	 <command name> <options> <subcommand>
 *
 * <command name> ::= <id>
 *
 * <id> ::= <letter>(<letter>|<digit>|<dash>)*
 *
 * <letter> ::= [A-Z] | [a-z]
 *
 * <digit> ::= [0-9]
 *
 * <dash> ::= _ | -
 *
 * <options or args> ::= <option> <options or args> |
 *                       <arg> <options or args> |
 *                       <empty>
 *
 * <options> ::= <empty> | <option> <options>
 *
 * <option> ::= <option name> <option values>
 *
 * <option name> ::= -<letter> | --<id>
 *
 * <option values> ::= <empty> | <value> <option values>
 *
 * <arg> := <value>
 *
 * <subcommand> ::= <subcommand name> <options or args>
 *
 * <subcommand name> ::= <id>
 *
 * <value> ::= <boolean> | <char> | <int> | <float> | <string> |
 *             <file> | <dir> | <choice>
 *
 * <boolean> ::= 0 | 1 | false | true
 *
 * <char> ::= printable ascii character
 *
 * <integer> ::= long recognised by stlib.h atol
 *
 * <float> ::= double recognised by stlib.h atof
 *
 * <string> ::= printable ascii string not starting with '-'
 * 				and which is not a subcommand name
 *
 * <file> ::= currently a <string> with no further validation
 *
 * <dir> ::= currently a <string> with no further validation
 *
 * <choice> ::= <id>
 * ```
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
 *  ARG_CHOICE       | Finite set of string alternatives               | char * (heap)
 */
typedef enum {
	ARG_NONE = 0,  /* No type, used for valueless options */
	ARG_BOOL = 1,  /* Boolean type */
	ARG_CHAR = 2,  /* Single character */
	ARG_INT = 3,   /* Integer type, stored as long */
	ARG_FLOAT = 4, /* Floating point type, stored as double */
	ARG_STR = 5,   /* Free-form string literal type */
	ARG_FILE = 6,  /* File type (no validation, used for
						descriptive/documentation purposes) */
	ARG_DIR = 7,   /* Directory type (no validation, used for
						descriptive/documentation purposes) */
	ARG_CHOICE = 8 /* Finite set of string alternatives */
} cliargtype;


/**
 * @brief Does a CLI option need to bee used on each call?
 */
typedef enum {
	OPT_OPTIONAL  =  0, /**< Option may or may not be used on a program call */
	OPT_REQUIRED  =  1  /**< Option MUST be used on every call */
} clioptneed;


/**
 * @brief How many times can an option be used on a single call?
 */
typedef enum {
	OPT_SINGLE   = 0,	/**< Option can be used at most once per call */
	OPT_MULTIPLE = 1	/**< Option can be used multiple times on a call */
} clioptmultiplicity;


/**
 * @brief Indicates an unlimited number of option/argument values
 */
#define ARGNO_UNLIMITED INT_MAX


/**
 * @brief CLI Option type (opaque)
 */
typedef struct _cliopt cliopt;


/**
 * @brief Option combo type which specifies how groups of inderdependent
 * options should be used together.
 */
typedef enum {
	ONE_OF,		/**< Exactly one of the options in the combo should be used. **/
	ALL_OF,		/**< All of the options in the combo should be used. **/
	ONE_IF_ANY,	/**< At most one of the option int the combo should be used. **/
	ALL_IF_ANY	/**< All or nono of the options in the combo should be used. **/
} clioptcombotype;


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
typedef struct _cliparser cliparser;


/**
 * @brief Creates a new non-sc option.
 *
 * @param shortname		(**no transfer**) The one-character distinct name
 * @param longname		(**no transfer**) The multi-character distinct name
 * @param help			(**no transfer**) A short description of the option
 * 						used for help messages.
 * @param need			Is the option use mandatory?
 * @param multiplicity	Can the option be declared multiple times?
 * @param type			The type of the option values (ir any)
 * @param min_val_no	The minimum number of option values
 * @param max_val_no	The maximum number of option values.
 * 						Required: @p min_val_no <= @p max_val_no.
 * 						Use ARGNO_UNLIMITED for an unlimited number ofvalues.
 * @param choices		(**transfer**) Possible choices if @p type = ARG_CHOICE
 * @param defaults		(**transfer**) Default values
 *
 * The following validations will be performed:
 *
 * - @p shortname is an alphabetical character (<letter> in the CLI grammar)
 * - @p longname ia a valid <id> according to the CLI grammar (a letter followed by
 *   a string of alphanumeric chars, dash '-' or underscores '_')
 * - @p min_val_no <= @p max_val_no
 * - If @p max_val_no == 0 then @p type == ARG_NONE and vice versa (iff)
 * - If @p need == OPT_REQUIRED, then @p max_val_no != 0 (equiv @p type != ARG_NONE)
 * - If @p multiplicity == OPT_MULTIPLE, then @p max_val_no != 0 (equiv @p type != ARG_NONE)
 * - If @p type == ARG_CHOICE then @p choices must be a non-empty vector of strings with
 *   every member being a valid <id> according to the CLI grammar
 * - If @p max_val_no == 0 (equiv @p type == ARG_NONE), then @p multiplicity == OPT_SINGLE
 * - If @p need == OPT_REQUIRED, @p defaults is discarded
 * - If @p need == OPT_OPTIONAL, and @p defaults is given, it must have at least
 *   @p min_val_no and at most @p max_val_no elements. In such case, the type of @p defaults
 *   and the constraints that each of its elements must observe is given in the following table.
 *
 * option type  | defaults type                 | element constraints
 * -------------|-------------------------------|---------------------------------------------
 * ARG_BOOL     | vec of bool                   | valid boolean constant
 * ARG_CHAR     | vec of char                   | isprint(elt) == true
 * ARG_INT      | vec of long                   | valid long constant
 * ARG_FLOAT    | vec of double                 | valid double constant
 * ARG_STRING   | vec of heap-allocated char *  | elt[0] != '-'
 * ARG_FILE     | vec of heap-allocated char *  | elt[0] != '-'
 * ARG_DIR      | vec of heap-allocated char *  | elt[0] != '-'
 * ARG_CHOICE   | vec of heap-allocated char *  | elt must be equal to some member of choices
 *
 */
cliopt *cliopt_new(char shortname,  char *longname, char *help,
                   clioptneed need, clioptmultiplicity multiplicity,
                   cliargtype type, int min_val_no, int max_val_no,
                   vec *choices, vec *defaults );


/**
 * @brief Creates a new option with default settings:
 * - Optional (non-mandatory)
 * - Single usage (can appear at most once per program call)
 * - No associated value (boolean switch).
 *
 * @param shortname (**no transfer**) The one-character distinct name
 * @param longname  (**no transfer**) The multi-character distinct name
 * @param help      (**no transfer**) A short description of the option
 *                  used for help messages.
 *
 * This is equivalent to
 * ```C
 * clopt_new(shortname, longname, help, OPT_OPTIONAL, OPT_SINGLE, ARG_NONE, 0, 0, NULL, NULL )
 * ```
 */
cliopt *cliopt_new_defaults(char shortname, char *longname, char *help);


/**
 * @brief Creates a new short circuit (sc) option.
 * @see cliopt_new
 */
cliopt *cliopt_new_sc(char shortname,  char *longname, char *help,
                      cliargtype type, int min_val_no, int max_val_no,
                      vec *choices, vec *defaults );


/**
 * @brief Creates a new short circuit (sc) option with default settings.
 * @see cliopt_new_defaults
 */
cliopt *cliopt_new_sc_defaults(char shortname,  char *longname, char *help);


/**
 * @brief Returns the option shortname.
 */
const char cliopt_shortname(const cliopt *opt);


/**
 * @brief Creates a new single-valued positional argument.
 * @param name	(**no transfer**) The name of the argument used for
 * 				descritptive/documentation purposes only.
 * @param help	(**no transfer**) A short description of the argument
 * 				used for help messages.
 * @param type	The type of the argument.
 */
cliarg *cliarg_new(char *name, char *help, cliargtype type);


/**
 * @brief Creates a new multi-valued positional argument.
 * 		  Only the last positional argument can be multi-valued.
 * @param name	(**no transfer**) The name of the argument used for
 * 				descritptive/documentation purposes only.
 * @param help	(**no transfer**) A short description of the argument
 * 				used for help messages.
 * @param type	The type of the argument.
 */
cliarg *cliarg_new_multi(char *name, char *help, cliargtype type);


/**
 * @brief Creates a new (sub)program CLI parser.
 * @param name	(**no transfer**) The name of the (sub)program.
 * @param help	(**no transfer**) A short description of the (sub)program
 * 				used for help messages.
 */
cliparser *cliparser_new(char *name, char *help);


/**
 * @brief Destructor.
 * Use with default destructor `FNR(cliparser)`, or
 * simply `DESTROY_FLAT(obj, cliparser)`.
 */
void cliparser_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Returns the name of a command parser
 */
const char *cliparser_name(const cliparser *cmd);


/**
 * @brief Adds a subprogram (subcommand parser) to a
 * program (command parser).
 *
 * The following validations are performed:
 *
 * - Neither can @p cmd be a subcommand, or and @p subcmd
 *   have subcommands, so that we have at most one level
 * 	 of subcommands nesting
 * - Moreover every @p subcmd must be added as a subcommand
 *   at most once
 * - If @p subcmd has an option of type @p ARG_CHOICE, then
 *   none of its choice values can be equal to a command
 *   or subcommand name of the @p cmd parser.
 */
void cliparser_add_subcommand(cliparser *cmd, cliparser *subcmd);


/**
 * @brief Adds an option to a (sub)program CLI parser.
 *
 * The following validations are performed:
 *
 * - Both the short (`-`) and long (`--`) names of the option must
 *   be unique for the command. In particular, a help option with names
 *  `-h`, `--help` is automatically added to every cliparser.
 * - if @p opt is of type ARG_CHOICE, then none of its choice values
 *   can be equal to any command or subcommand name of the parser @p cmd
 */
void cliparser_add_option(cliparser *cmd, cliopt *opt);


/**
 * @brief Adds an option combo specification to the parser. A variable
 * list of  @p n cliopt  references should be passed as arguments.
 *
 * @warning The combo specifies the dependencies between the options only,
 * and they must be independently added via ::cliparser_add_option.
 */
void cliparser_add_option_combo(cliparser *cmd, clioptcombotype type, size_t n,
                                ...);


/**
 * @brief Adds a positional argument to a (sub)program CLI parser.
 *
 * Ir
 *
 */
void cliparser_add_pos_arg(cliparser *cmd, cliarg *arg);


/**
 * @brief Prints the automatically-generated help message of a program
 * 			to the standard output.
 */
void cliparser_print_help(const cliparser *cmd);


/**
 * @brief CLI parse error codes
 */
typedef enum {
	UNPARSED,
	INVALID_OPTION,
	INVALID_MULT_OPTION,
	INVALID_OPT_VAL_NO,
	MISSING_REQ_OPT,
	INVALID_OPT_COMBO,
	INVALID_SUBCMD,
	UNEXPECTED_TK,
	INVALID_ARG_VAL,
	INVALID_ARG_VAL_NO,
	UNDEF_ERR
} cliparse_err_code;

#define CLIPARSE_ERROR_BUFSZ 128

/**
 * CLI parse error result type.
 */
typedef struct {
	cliparse_err_code code;
	char msg[CLIPARSE_ERROR_BUFSZ];
} cliparse_error;


DECL_RESULT_ERR(cliparse, cliparser *, cliparse_error)

/**
 * @brief Parses a program call.
 *
 * If the call is sucessfully parsed, the @p cmd is populated with the
 * option and argument values, including default values for undeclared
 * non-required options, if available. If a parse error occurs, an error object
 * cliparser_error is returned containing an error message, unless the
 * paramater @p exit_on_error is set to true, in which case the error message
 * is printed to stderr and the program exits.
 *
 * @param argc The number of tokens (normally received by main())
 * @param argv (*no transfer*) The program call tokens (also received by main())
 * @param exit_on_error Indicates whether the program should exit if a parse error is found.
 *
 * The parser assumes @p argv to contain the tokens of a program call to be
 * parsed according to the CLI grammar shown in the module
 * documentation. **All tokens** must be separated by spaces.
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
 * but let us suppose the the first form has been entered.
 *
 * This function handles option values as follows. When an option name is
 * found at position `i`, it then tries to greedily collect as many values as
 * possible from positions `i+1`, `i+2` ,... until
 * - The end of @p argv, OR
 * - The maximum number of allowed values are successfully parsed, OR
 * - A type-mismatch occurs.
 *
 * So, in our current example, because `-a` accepts up to 3 values, the
 * call would be parsed as if the values of `-a` were (`1`,`2`,`3`) and
 * the values of the argument were (`4`,`5`).
 *
 * Now, if instead of having one argument with multiple int values, the
 * command had three single int arguments, the parse would result
 * in an error (missing argument), althought it could have been interpreted
 * as if option `-a` had values (`1`, `2`) and the arguments values were
 * `3`, `4`, and `5`.
 *
 */
cliparse_res cliparser_parse(cliparser *cmd, int argc, char **argv,
                             bool exit_on_error);


//const char *cliparser_parse_status_msg(cliparser *cmd);

/**
 * @brief Returns the invoked subcommand of a command, if any.
 * @return Prior to parsing a call with ::cliparser_parse (cmd), returns NULL.
 * After parsing a call, if a subcommand was called, returns the
 * corresponding (populated) parser, else returns NULL.
 */
const cliparser *cliparser_active_subcommand(const cliparser *cmd);


/**
 * @brief Returns the declared sc option of a (sub)command, if any.
 * @return Prior to parsing a call with ::cliparser_parse(cmd), returns NULL.
 * After parsing a call, if an sc option was declared, returns the
 * corresponding (populated) cliopt, else returns NULL.
 */
const cliopt *cliparser_active_sc_option(const cliparser *cmd);


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
 * If the option can be declared multiple types (parameter `multi==OPT_MULTIPLE` of
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
const vec *cliparser_opt_val_from_shortname(const cliparser *cmd,
        char shortname);


/**
 * @brief Gets the values of an option from its long name.
 *
 * @param cmd The (sub)command parser
 * @param longname The long (`--`) name of the option.
 *
 * @see cliparser_opt_val_from_shortname
 *
 */
const vec *cliparser_opt_val_from_longname(const cliparser *cmd,
        char *longname);


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
const vec *cliparser_arg_val_from_pos(const cliparser *cmd, size_t pos);



#endif