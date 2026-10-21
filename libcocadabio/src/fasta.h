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
#ifndef FASTA_H
#define FASTA_H

#include <stdbool.h>

#include "result.h"
#include "strread.h"

/**
 * @file fasta.h
 * @brief FASTA file sequence reader.
 * @author Paulo Fonseca
 *
 * The #fasta reader provides a way to iterate through the records
 * defined in a FASTA-format file
 *
 * @see https://en.wikipedia.org/wiki/FASTA_format
 */


/**
 * @brief FASTA file/stream reader.
 */
typedef struct _fasta fasta;


/**
 * @brief A FASTA record with in-memory sequence
 */
typedef struct {
	char *id;      		 /**< Sequence id (mandatory) */
	char *descr;		 /**< Sequence description (optional) */
	char *seq;   		 /**< In-memory sequence content */
	size_t seq_len; 	 /**< Sequence length */
	size_t hdr_off;		 /**< Header offset from the start of the file */
	size_t seq_off;		 /**< Sequence offset from the start of the file */
} fasta_rec;



DECL_RESULT_OK_ERR(fasta_open, fasta*, code_msg_err)

/**
 * @brief Opens a FASTA file and places the cursor at its beginning.
 *
 * @param path The path to the file
 *
 * @return An OK-Error result containing a pointer to a #fasta, in
 * case of success, or a #code_msg_err in case of an error
 *
 * @see result.h
 */
fasta_open_res fasta_open(const char *path);


/**
 * @brief Returns the FASTA file path
 */
const char *fasta_path(fasta *self);

/**
 * @brief Checks whether there is a sequence *after* the current
 * cursor position. To be used for iterating over sequence records
 *
 * @see fasta_next
 */
bool fasta_has_next(fasta *self);


/**
 * @brief Moves the stream reader to the sequence at the given  offset
 *
 * @param hdr_off The offset of the description line of the record
 * starting with '>'
 *
 * @return If the operation succeeds and the offset position
 * contains a record start char '>', return true; else rewinds to
 * the beginning of the file via #fasta_rewind and return false.
 */
bool fasta_goto(fasta *self, size_t hdr_off);


/**
 * @brief Resets the reader to the beginning of the file.
 */
void fasta_rewind(fasta *self);


/**
 * @brief FASTA Parse error codes
 */
typedef enum {
	FASTA_MISSING_HDR,
	FASTA_EMPTY_ID,
	FASTA_EMPTY_SEQ,
}
fasta_err_code;


/**
 * @brief Error messages corresponding to the parse error codes.
 *
 * @see fasta_err_code
 */
extern const char *const fasta_err_msg[3];


/**
 * @brief Fasta parse error
 */
typedef struct {
	fasta_err_code code;  /**< The error code **/
	size_t off;          /**< The file offset at which the error occurs */
} fasta_err;


DECL_RESULT_OK_ERR(fasta_next, const fasta_rec *, fasta_err)


/**
 * @brief Advances the cursor to the next sequence record, loading
 * the sequence from file to memory as a string.
 *
 * Example
 * -------
 *
 * ```C
 * fasta_open_res r = fasta_open(filename);
 * if (!r.ok) //... treat error and exit
 * fasta *f = r.val.ok;
 * while (fasta_has_next(f)) {
 * 	fasta_next_res r = fasta_next(f);
 * 	if (!r.ok) {
 * 		fprintf(stderr, fasta_error_messages[r.val.err.code]);
 * 		// abort
 * 	}
 * 	const fasta_rec *rec = r.val.ok;
 * 	// do something with the record
 * }
 * fasta_close(f);
 * ```
 * @warning The returned record object should *not* be modified or
 * destroyed directly.
 */
fasta_next_res fasta_next(fasta *self);


/**
 * @brief Destructor. Closes the reader and releases used resources
 */
void fasta_close(fasta *self);

#endif
