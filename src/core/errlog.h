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

#ifndef ERRLOG_H
#define ERRLOG_H


/**
 * @file errlog.h
 * @brief Error logging
 * @author Paulo Fonseca
 *
 * COCADA defines four debug levels, as defined by the DEBUG_LVL macro,
 * which is often declared at compile time with the appropriate compiler
 * option (`-DDEBUG_LVL=L` in `gcc`).
 *
 * The four levels in increasing sensitivity are
 * 
 * - Off      (DEBUG_LVL=0)  
 * - Error    (DEBUG_LVL=1)
 * - Warning  (DEBUG_LVL=2)
 * - Debug    (DEBUG_LVL=3 Default)
 * 
 * The Off level is used to turn off all verifications in the code.
 *
 * The error level is used for checking critical, unrecoverable error 
 * conditions that cause the program to exit with a EXIT_FAILURE status.
 *
 * The warning level is used for recoverable errors or extreme conditions
 * which do not abort the program but may result in suboptimal behaviour.
 *
 * The debug level is used for conditionally reporting specific 
 * information about the program state during execution.
 *
 * These levels are strictly hierarchical. Any event of a level `L` is
 * also pertinent to levels greater than `L`. That is, an Error event
 * is also a Warning and a Debug event. A Warning is also a Debug info
 * but not an Error. A Debug event is just a Debug info event and nothing
 * else. Conversely, an event is only considered if its level is
 * >= DEBUG_LVL. For example, all Warning events are disregarded if
 * DEBUG_LVL=0 (Error level). This module provides macros for triggering
 * and reporting  such events.
 */


#ifndef DEBUG_LVL
#warning Unset debug level (DEBUG_LVL). Setting to default DEBUG(3) level.
/**
 * @brief The debug level described in the module documentation
 */
#define DEBUG_LVL 3
#elif DEBUG_LVL<0 || DEBUG_LVL>3
#warning Invalid debug level (DEBUG_LVL). Setting to default DEBUG(3) level.
#undef DEBUG_LVL
#define DEBUG_LVL 3
#endif

/**
 * @brief Prints execution stack trace
 */
void print_trace(FILE *);


// ------------------------ ERROR ---------------------------

#if DEBUG_LVL>=1

/**
 * @brief Triggers an ERROR and prints an error message to `stderr`,
 * after which the program exits with `EXIT_FAILURE` status.
 *
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define ERROR(FORMAT, ...)\
	fprintf(stderr,  "Error: "FORMAT, ##__VA_ARGS__ ); \
	print_trace(stderr);\
	exit(EXIT_FAILURE);


/**
 * @brief Conditionally triggers an ERROR and prints an error
 * message to `stderr`, after which the program exits with
 * `EXIT_FAILURE` status.
 *
 * @param CONDITION The condition that has to be satisfied for
 * the Error to occur.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define ERROR_IF(CONDITION, FORMAT, ...) \
if ( CONDITION ) { \
	ERROR(FORMAT, ##__VA_ARGS__)\
}


/**
 * @brief Triggers an ERROR if an assertion is false and prints
 * an error message to `stderr`, after which the program exits with
 * `EXIT_FAILURE` status.
 *
 * @param ASSERTION The condition which, if not satisfied, triggers
 * the Error.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define ERROR_ASSERT(ASSERTION, FORMAT, ...) \
	ERROR_IF(!(ASSERTION), FORMAT, ##__VA_ARGS__)


#else 

#define ERROR(IGNORE, ...) ((void)0)
#define ERROR_IF(IGN, ORE, ...) ((void)0)
#define ERROR_ASSERT(IGN, ORE, ...) ((void)0)

#endif


// ------------------------ WARNING ---------------------------

#if DEBUG_LVL>=2

/**
 * @brief Issues a WARNING and prints an error message to `stderr`
 * if DEBUG_LVL>=1. Otherwise nothing takes place.
 *
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define WARN(FORMAT, ...) \
	fprintf(stderr,  "Warning: "FORMAT, ##__VA_ARGS__ )


/**
 * @brief Conditionally issues a WARNING and prints a message to
 * `stderr`. If DEBUG_LVL<1 nothing takes place regardless of
 * the condition.
 *
 * @param CONDITION The condition that has to be satisfied for
 * the warning.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define WARN_IF(CONDITION, FORMAT, ...) \
	if ( CONDITION ) { \
		fprintf(stderr,  "Warning: "FORMAT, ##__VA_ARGS__ ); \
	}


/**
 * @brief Issues a WARNING if an assertion is false and prints
 * an error message to `stderr`.
 * If DEBUG_LVL<1 nothing takes place regardless of  the assertion.
 *
 * @param ASSERTION The condition which, if not satisfied, triggers
 * the Error.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define WARN_ASSERT(ASSERTION, FORMAT, ...) \
	if ( !(ASSERTION) ) { \
		fprintf(stderr,  "Warning: "FORMAT, ##__VA_ARGS__ ); \
	}

#else

#define WARN(IGNORE, ...) ((void)0)
#define WARN_IF(IGN, ORE, ...) ((void)0)
#define WARN_ASSERT(IGN, ORE, ...) ((void)0)

#endif

// ------------------------ DEBUG ---------------------------

#if DEBUG_LVL>=3

/**
 * @brief Prints a DEBUG info message to `stderr`
 * if DEBUG_LVL==2. Otherwise nothing takes place.
 *
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define DEBUG(FORMAT, ...) \
	fprintf(stderr,  "Debug: "FORMAT, ##__VA_ARGS__ );

/**
 * @brief Conditionally prints a DEBUG info message to
 * `stderr`. If DEBUG_LVL<2 nothing takes place regardless of
 * the condition.
 *
 * @param CONDITION The condition that has to be satisfied for
 * the warning.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define DEBUG_IF(CONDITION, FORMAT, ...) \
if ( CONDITION ) { \
	fprintf(stderr,  "Debug: "FORMAT, ##__VA_ARGS__ );\
}

/**
 * @brief Prints a DEBUG info message to `stderr`if an assertion is
 * false.
 * If DEBUG_LVL<2 nothing takes place regardless of the assertion.
 *
 * @param ASSERTION The condition which, if not satisfied, triggers
 * the Error.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define DEBUG_ASSERT(ASSERTION, FORMAT, ...) \
if ( !(ASSERTION) ) { \
	fprintf(stderr,  "Debug: "FORMAT, ##__VA_ARGS__ );\
}

#else

#define DEBUG(IGNORE , ...) ((void)0)
#define DEBUG_IF(IGN, ORE , ...) ((void)0)
#define DEBUG_ASSERT(IGN, ORE , ...) ((void)0)

#endif

#endif // ERRLOG_H