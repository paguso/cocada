
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
 * @file fastq.h
 * @brief FASTQ file sequence reader.
 * @author Paulo Fonseca
 *
 * The #fastq reader provides a way to iterate through the records
 * defined in a FASTQ-format file
 *
 * @see https://en.wikipedia.org/wiki/FASTQ_format
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
	size_t hdr_off;		/**< Id offset from the start of the file (starting from the @) */
	size_t seq_off;		/**< Sequence offset from the start of the file */
	size_t qlty_off;	/**< Quality offset from the start of the file */
} fastq_rec;


/**
 * @brief Ok-Error result for FASTQ open stream
 */
DECL_RESULT_OK_ERR(fastq_open, fastq*, code_msg_err)


/**
 * @brief Opens a fastq file and places the cursor at its beginning.
 *
 * @param path The path to the file
 *
 * @return An OK-Error result containing a pointer to a #fastq, in
 * case of success, or a #code_msg_err in case of an error
 *
 * @see result.h
 */
fastq_open_res fastq_open(const char *path);


/**
 * @brief Returns the fastq file path
 */
const char *fastq_path(fastq *self);



/**
 * @brief Resets the reader to the beginning of the file.
 */
void fastq_rewind(fastq *self);


/**
 * @brief Moves the stream reader to the sequence at the given  offset
 *
 * @param hdr_off The offset of the description line of the record
 * starting with '@'
 *
 * @return If the operation succeeds and the offset position
 * contains a record start char '@', return true; else rewinds to
 * the beginning of the file via #fasta_rewind and return false.
 */
bool fastq_goto(fastq *self, size_t hdr_off);


/**
 * @brief Checks whether there is a sequence *after* the current
 * cursor position. To be used for iterating over the fastq
 * records in conjunction with #fastq_next
 */
bool fastq_has_next(fastq *self);


/**
 * @brief FASTQ Parse error codes
 */
typedef enum {
	FASTQ_MISSING_HDR,
	FASTQ_EMPTY_ID,
	FASTQ_EMPTY_SEQ,
	FASTQ_MISSING_SEP,
	FASTQ_WRONG_QLTY_LEN
}
fastq_err_code;


/**
 * @brief Error messages corresponding to the parse error codes.
 *
 * @see fastq_error_code
 */
extern const char *const fastq_err_msg[5];


/**
 * @brief Fastq parse error
 */
typedef struct {
	fastq_err_code code;   /**< The error code **/
	size_t off;		       /**< The offset at which the error occurs */
} fastq_err;


DECL_RESULT_OK_ERR(fastq_next, const fastq_rec *, fastq_err)

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
