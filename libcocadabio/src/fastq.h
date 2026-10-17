
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
 * @brief fastq file sequence reader.
 * @author Paulo Fonseca
 *
 */


/**
 * @brief FASTA file/stream type.
 */
typedef struct _fastq fastq;


/**
 * @brief A FASTA record with in-memory sequence
 */
typedef struct {
	char *descr;	/**< Sequence descriptor (does not include the `>`) */
	size_t seq_len;
	char *seq;   	/**< In-memory sequence content */
	char *qlty;   	/**< In-memory sequence content */
	size_t descr_offset;	/**< Descriptor offset from the start of the file */
	size_t seq_offset;		/**< Sequence offset from the start of the file */
	size_t qlty_offset;		/**< Quality offset from the start of the file */
} fastq_rec;


///*
// * @brief A FASTA record with sequence loaded as a stream
// */
//typedef struct {
//	char *descr;      	/**< Sequence descriptor */
//	strread *seqrdr;	/**< Sequence contents reader */
//	strread *qltyrdr;	/**< Sequence quality reader */
//	size_t descr_offset;	/**< Descriptor offset from the start of the file */
//	size_t seq_offset;		/**< Sequence offset from the start of the file */
//	size_t qlty_offset;		/**< Quality offset from the start of the file */
//} fastq_rec_rdr;


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


///**
// * @brief Advances the cursor to the next sequence record, and loads
// * it as a stream.
// * The returned strread reader is a virtual stream that points to the
// * beginning of the sequence contents inside the file, and is used
// * to read these contents as if they were in a separate stream,
// * ignoring line breaks. EOF is reached at the end of the sequence.
// *
// * Example
// * -------
// *
// * ```C
// * fasta *fr = fasta_open("teste.fa");
// * while(fasta_has_next(fr)) {
// *  fasta_rec_rdr rec = fasta_next_reader(fr);
// *  for (xchar c; (c=strread_getc(rec->seqrdr)) != EOF;) {
// *      //do something with c
// *  }
// * }
// * ```
// * @warning The returned record reader object contains internal references and
// * should *not* be modified or destroyed directly.
// * @see strread
// */
//const fastq_rec_rdr *fastq_next_reader(fastq *self);


/**
 * @brief Advances the cursor to the next sequence record, loading
 * the sequence from file to memory as a string.
 *
 * Example
 * -------
 *
 * ```C
 * fasta *fr = fasta_open("teste.fa");
 * while(fasta_has_next(fr)) {
 *  fasta_rec rec = fasta_next(fr);
 * 	printf("sequence is %s\n",rec->seq);
 * }
 * ```
 * @warning The returned record object should *not* be modified or
 * destroyed directly.
 */
const fastq_rec *fastq_next(fastq *self);


/**
 * @brief Destructor. Closes the reader and releases used resources
 */
void fastq_close(fastq *self);

#endif
