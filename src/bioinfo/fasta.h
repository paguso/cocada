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

#include <stdbool.h>

#include "strread.h"

/**
 * @file fasta.h
 * @author Paulo Fonseca
 * @brief FASTA file sequence reader
 *
 * The `fasta` reader provides a way to iterate through the sequences
 * defined in a FASTA-format file.
 */

typedef struct _fasta fasta;

/**
 * @brief A FASTA record with in-memory sequence
 */
typedef struct {
	char *descr; /**< Sequence descriptor (does not include the `>`) */
	char *seq;   /**< In-memory sequence content */
} fasta_rec;

/**
 * @brief A FASTA record with sequence loaded as a stream
 */
typedef struct {
	char *descr;      /**< Sequence descriptor */
	strread *seqrdr; /**< Sequence contents reader */
} fasta_rec_rdr;


/**
 * @brief Opens a FASTA file and places the cursor at the beginning
 */
fasta *fasta_open(char *filename);


/**
 * @brief Checks whether there is a sequence *after* the current
 * cursor position
 */
bool fasta_has_next(fasta *self);


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
 *  for (xchar c; (c=strread_getc(rec->seqread)) != EOF;) {
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
 * @warning The returned record reader object should *not* be modified or 
 * destroyed directly.
 * @see strread
 */
const fasta_rec *fasta_next(fasta *self);


/**
 * @brief Closes the reader and releases used resources
 */
void fasta_close(fasta *self);