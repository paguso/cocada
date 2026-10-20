
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
#ifndef FASTQ_H
#define FASTQ_H

#include <stdbool.h>

#include "result.h"
#include "strread.h"

/**
 * @file fasta.h
 * @brief fastq file sequence reader.
 * @author Paulo Fonseca
 *
 */


/**
 * @brief FASTQ file/stream reader type.
 */
typedef struct _fastq fastq;


/**
 * @brief A FASTQ record with in-memory sequence
 */
typedef struct {
	char *id;			/**< Sequence identifier */
	char *descr;		/**< Sequence description (optional) */
	char *seq;			/**< In-memory sequence content */
	char *qlty;   		/**< In-memory sequence content */
	size_t seq_len;		/**< The length of the sequence */
	size_t hdr_offset;	/**< Id offset from the start of the file (starting from the @) */
	size_t seq_offset;	/**< Sequence offset from the start of the file */
	size_t qlty_offset;	/**< Quality offset from the start of the file */
} fastq_rec;



DECL_RESULT_OK_ERR(fastq_open, fastq*, code_msg_err)


/**
 * @brief Opens a fastq file and places the cursor at its beginning.
 *
 * @param filename The path to the file
 *
 * @return An OK-Error result containing a pointer to a #fastq, in
 * case of success, or a #code_msg_err in case of an error
 *
 * @see result.h
 */
fastq_open_res fastq_open(const char *filename);


/**
 * @brief Returns the fastq file path
 */
const char *fastq_path(fastq *self);


/**
 * @brief Checks whether there is a sequence *after* the current
 * cursor position
 */
bool fastq_has_next(fastq *self);


/**
 * @brief Sets the position of the stream to the beginning of the file.
 */
void fastq_rewind(fastq *self);


/**
 * @brief FASTQ Parse error codes
 */
typedef enum {
	FASTQ_EXPECTED_HEADER,
	FASTQ_EMPTY_ID,
	FASTQ_EMPTY_SEQUENCE,
	FASTQ_EXPECTED_SEPARATOR,
	FASTQ_WRONG_QLTY_SEQUENCE_LENGTH
}
fastq_error_code;


/**
 * @brief Error messages corresponding to the parse error codes.
 *
 * @see fastq_error_code
 */
extern const char *const fastq_error_messages[5];


/**
 * @brief Fastq parse error
 */
typedef struct {
	fastq_error_code code; /**< The error code **/
	size_t line;		   /**< The line at which the error occurs */
} fastq_error;


DECL_RESULT_OK_ERR(fastq_next, const fastq_rec *, fastq_error)

/**
 * @brief Advances the cursor to the next sequence record, loading
 * the sequence from file to memory as a string.
 *
 * Example
 * -------
 * ```C
 * fastq_open_res r = fastq_open(filename);
 * if (!r.ok) //... treat error and exit
 * fastq *f = r.val.ok;
 * while (fastq_has_next(f)) {
 * 	fastq_next_res r = fastq_next(f);
 * 	if (!r.ok) {
 * 		fprintf(stderr, fasta_error_messages[r.val.err.code]);
 * 		// abort
 * 	}
 * 	const fastq_rec *rec = r.val.ok;
 * 	// do something with the record
 * }
 * fastq_close(f);
 * ```
 *
 * @warning The returned record object should *not* be modified or
 * destroyed directly.
 */
fastq_next_res fastq_next(fastq *self);


/**
 * @brief Destructor. Closes the reader and releases used resources
 */
void fastq_close(fastq *self);

#endif
