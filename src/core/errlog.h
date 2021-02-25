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


#include <stdio.h>
#include <inttypes.h>

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
 * DEBUG_LVL=1 (Error level). This module provides macros for triggering
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
 * @brief Triggers an ERROR and prints an error message to an
 * output stream after which the program exits with `
 * EXIT_FAILURE` status.
 *
 * @param STREAM The output stream
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FERROR(STREAM, FORMAT, ...)\
	fprintf(STREAM,  "Error: "FORMAT, ##__VA_ARGS__ ); \
	print_trace(stderr);\
	exit(EXIT_FAILURE);


/**
 * @brief Same as FERROR(stderr, FORMAT, ...)
 * @see FERROR
 */
#define ERROR(FORMAT, ...) FERROR(stderr, FORMAT, ##__VA_ARGS__)


/**
 * @brief Conditionally triggers an ERROR and prints an error
 * message to an output stream, after which the program exits with
 * `EXIT_FAILURE` status.
 *
 * @param STREAM The output stream
 * @param CONDITION The condition that has to be satisfied for
 * the Error to occur.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FERROR_IF(CONDITION, STREAM, FORMAT, ...) \
	if ( CONDITION ) { \
		FERROR(STREAM, FORMAT, ##__VA_ARGS__)\
	}


/**
 * @brief Same as FERROR_IF(CONDITION, stderr, FORMAT, ...)
 * @see FERROR_IF
 */
#define ERROR_IF(CONDITION, FORMAT, ...) \
	FERROR_IF(CONDITION, stderr, FORMAT, ##__VA_ARGS__)


/**
 * @brief Triggers an ERROR if an assertion is false and prints
 * an error message to an output stream, after which the program 
 * exits with `EXIT_FAILURE` status.
 *
 * @param STREAM The output stream
 * @param ASSERTION The condition which, if not satisfied, triggers
 * the Error.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FERROR_ASSERT(ASSERTION, STREAM, FORMAT, ...) \
	FERROR_IF(!(ASSERTION), STREAM, FORMAT, ##__VA_ARGS__)


/**
 * @brief Same as FERROR_ASSERT(ASSERTION, stderr, FORMAT, ...)
 * @see FERROR_ASSERT
 */
#define ERROR_ASSERT(ASSERTION, FORMAT, ...) \
	FERROR_ASSERT(ASSERTION, stderr, FORMAT, ##__VA_ARGS__)

#else

#define FERROR(IGN, ORE, ...) ((void)0)
#define ERROR(IGNORE, ...) ((void)0)
#define FERROR_IF(IG, NO, RE, ...) ((void)0)
#define ERROR_IF(IGN, ORE, ...) ((void)0)
#define FERROR_ASSERT(IG, NO, RE, ...) ((void)0)
#define ERROR_ASSERT(IGN, ORE, ...) ((void)0)

#endif


// ------------------------ WARNING ---------------------------

#if DEBUG_LVL>=2

/**
 * @brief Issues a WARNING and prints an error message to 
 * an output stream if DEBUG_LVL>=2. Otherwise nothing takes place.
 *
 * @param STREAM The output stream
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FWARN(STREAM, FORMAT, ...) \
	fprintf(STREAM,  "Warning: "FORMAT, ##__VA_ARGS__ )


/**
 * @brief Same as FWARN(stderr, FORMAT, ...)
 * @see FWARN
 */
#define WARN(FORMAT, ...) FWARN(stderr, FORMAT, ##__VA_ARGS__)


/**
 * @brief Conditionally issues a WARNING and prints a message to
 * an output stream. If DEBUG_LVL < 2 nothing takes place regardless of
 * the condition.
 *
 * @param STREAM The output stream
 * @param CONDITION The condition that has to be satisfied for
 * the warning.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FWARN_IF(CONDITION, STREAM, FORMAT, ...) \
	if ( CONDITION ) { \
		fprintf(STREAM,  "Warning: "FORMAT, ##__VA_ARGS__ ); \
	}


/**
 * @brief Same as FWARN_IF(CONDITION, stderr, FORMAT, ...)
 * @see FWARN_IF
 */
#define WARN_IF(CONDITION, FORMAT, ...) \
	FWARN_IF(CONDITION, stderr, FORMAT, ##__VA_ARGS__)


/**
 * @brief Issues a WARNING if an assertion is false and prints
 * an error message to an output stream.
 * If DEBUG_LVL < 2 nothing takes place regardless of the assertion.
 *
 * @param STREAM The output stream
 * @param ASSERTION The condition which, if not satisfied, triggers
 * the Error.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FWARN_ASSERT(ASSERTION, STREAM, FORMAT, ...) \
	if ( !(ASSERTION) ) { \
		fprintf(STREAM,  "Warning: "FORMAT, ##__VA_ARGS__ ); \
	}


/**
 * @brief Same as FWARN_ASSERT(ASSERTION, stderr, FORMAT, ...)
 * @see FWARN_ASSERT
 */
#define WARN_ASSERT(ASSERTION, FORMAT, ...) \
	FWARN_ASSERT(ASSERTION, stderr, FORMAT, ##__VA_ARGS__)

#else

#define FWARN(IGN, ORE, ...) ((void)0)
#define WARN(IGNORE, ...) ((void)0)
#define FWARN_IF(IG, NO, RE, ...) ((void)0)
#define WARN_IF(IGN, ORE, ...) ((void)0)
#define FWARN_ASSERT(IG, NO, RE, ...) ((void)0)
#define WARN_ASSERT(IGN, ORE, ...) ((void)0)

#endif

// ------------------------ DEBUG ---------------------------

#if DEBUG_LVL>=3

/**
 * @brief Prints a DEBUG info message to an output stream
 * if DEBUG_LVL=3. Otherwise nothing takes place.
 *
 * @param STREAM The output stream
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FDEBUG(STREAM, FORMAT, ...) \
	fprintf(STREAM,  "Debug: "FORMAT, ##__VA_ARGS__ );


/**
 * @brief Same as FDEBUG(stderr, FORMAT, ...)
 * @see FDEBUG
 */
#define DEBUG(FORMAT, ...) FDEBUG(stderr, FORMAT, ##__VA_ARGS__)


/**
 * @brief Conditionally prints a DEBUG info message to
 * an output stream. If DEBUG_LVL < 3 nothing takes place 
 * regardless of the condition.
 *
 * @param STREAM The output stream
 * @param CONDITION The condition that has to be satisfied for
 * the warning.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FDEBUG_IF(CONDITION, STREAM, FORMAT, ...) \
	if ( CONDITION ) { \
		fprintf(STREAM,  "Debug: "FORMAT, ##__VA_ARGS__ );\
	}


/**
 * @brief Same as FDEBUG_IF(CONDITION, stderr, FORMAT, ...)
 * @see FDEBUG_IF
 */
#define DEBUG_IF(CONDITION, FORMAT, ...) \
	FDEBUG_IF(CONDITION, stderr, FORMAT, ##__VA_ARGS__)


/**
 * @brief Prints a DEBUG info message to an output stream if 
 * an assertion is false.
 * If DEBUG_LVL < 3 nothing takes place regardless of the assertion.
 *
 * @param STREAM The output stream
 * @param ASSERTION The condition which, if not satisfied, triggers
 * the Error.
 * @param FORMAT Is the format string of the message, followed
 * by the variable list of arguments as in `printf`-like functions.
 */
#define FDEBUG_ASSERT(ASSERTION, STREAM, FORMAT, ...) \
	if ( !(ASSERTION) ) { \
		fprintf(STREAM,  "Debug: "FORMAT, ##__VA_ARGS__ );\
	}


/**
 * @brief Same as FDEBUG_ASSERT(ASSERTION, stderr, FORMAT, ...)
 * @see FDEBUG_ASSERT
 */
#define DEBUG_ASSERT(ASSERTION, FORMAT, ...) \
	FDEBUG_ASSERT(ASSERTION, stederr, FORMAT, ##__VA_ARGS__) 


/**
 * @brief Executes a statement if in DEBUG mode, i.e. if  DEBUG_LVL >= 3
 */
#define DEBUG_EXEC(STATEMENT)\
	{STATEMENT;}\

#else

#define FDEBUG(IGN, ORE, ...) ((void)0)
#define DEBUG(IGNORE , ...) ((void)0)
#define FDEBUG_IF(IG, NO, RE , ...) ((void)0)
#define DEBUG_IF(IGN, ORE , ...) ((void)0)
#define FDEBUG_ASSERT(IG, NO, RE , ...) ((void)0)
#define DEBUG_ASSERT(IGN, ORE , ...) ((void)0)
#define DEBUG_ACTION(IGNORE) ((void)0)

#endif

#endif // ERRLOG_H