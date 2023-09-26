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
 * A FASTA file is a plain text sequence of *records*, each consisting
 * of *description* and the *sequence itself*.
 * The `fasta` reader provides a way to iterate through the records
 * defined in a FASTA-format file.
 * During the iteration, each sequence can *either* be entirely loaded to
 * memory as a string, or be treated as a char input stream with a
 * buffered string reader (::strread), which avoids having to load
 * potentially large sequences fully into memory.
 */


/**
 * @brief FASTA file/stream type.
 */
typedef struct _fasta fasta;


/**
 * @brief A FASTA record with in-memory sequence
 */
typedef struct {
	char *descr;	/**< Sequence descriptor (does not include the `>`) */
	char *seq;   	/**< In-memory sequence content */
	size_t descr_offset;	/**< Descriptor offset from the start of the file **/
	size_t seq_offset;		/**< Sequence offset from the start of the file **/
} fasta_rec;


/**
 * @brief A FASTA record with sequence loaded as a stream
 */
typedef struct {
	char *descr;      	/**< Sequence descriptor */
	strread *seqrdr;	/**< Sequence contents reader */
	size_t descr_offset;	/**< Descriptor offset from the start of the file **/
	size_t seq_offset;		/**< Sequence offset from the start of the file **/
} fasta_rec_rdr;



/**
 * @brief Opens a FASTA file and places the cursor at its beginning.
 * @param filename The path to the file (**NO TRANSFER OF OWNERSHIP**)
 */
rawptr_ok_err_res fasta_open(char *filename);


/**
 * @brief Returns the FASTA file path
 */
const char *fasta_path(fasta *self);

/**
 * @brief Checks whether there is a sequence *after* the current
 * cursor position
 */
bool fasta_has_next(fasta *self);


/**
 * @brief Moves the stream to the sequence at the given description offset
 * @p descr_offset.
 * 
 * @return true on success, false on fail.
 */
bool fasta_goto(fasta *self, size_t *descr_offset);


/**
 * @brief Sets the position of the stream to the beginning of the file.
 */
void fasta_rewind(fasta *self);


/**
 * @brief Advances the cursor to the next sequence record, and loads
 * it as a stream.
 * The returned strread reader is a virtual stream that points to the
 * beginning of the sequence contents inside the file, and is used
 * to read these contents as if they were in a separate stream,
 * ignoring line breaks. EOF is reached at the end of the sequence.
 *
 * Example
 * -------
 *
 * ```C
 * fasta *fr = fasta_open("teste.fa");
 * while(fasta_has_next(fr)) {
 *  fasta_rec_rdr rec = fasta_next_reader(fr);
 *  for (xchar c; (c=strread_getc(rec->seqrdr)) != EOF;) {
 *      //do something with c
 *  }
 * }
 * ```
 * @warning The returned record reader object contains internal references and
 * should *not* be modified or destroyed directly.
 * @see strread
 */
const fasta_rec_rdr *fasta_next_reader(fasta *self);


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
const fasta_rec *fasta_next(fasta *self);


/**
 * @brief Destructor. Closes the reader and releases used resources
 */
void fasta_close(fasta *self);

#endif
