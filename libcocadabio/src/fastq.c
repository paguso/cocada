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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cstrutil.h"
#include "fastq.h"
#include "new.h"
#include "strbuf.h"
#include "strfilereader.h"
#include "strread.h"
#include "strview.h"
#include "trait.h"
#include "errlog.h"


const char *const fastq_error_messages[5] = {
	"Expected field 1 starter @",
	"Empty record id",
	"Empty sequence",
	"Expected quality field separator +",
	"Wrong quality sequence length"
};

struct _fastq {
	char *src_path;
	strfilereader *freader;
	size_t cur_line;
	size_t cur_offset;
	strbuf *linebuf;
	strbuf *seqbuf;
	fastq_rec cur_rec;
};


fastq_open_res fastq_open(const char *filename)
{
	fastq_open_res result = {.ok = true};
	fastq *f = NEW(fastq);
	f->freader = strfilereader_new_from_path(filename);
	if (!f->freader) {
		result.ok = false;
		if (errno) {
			result.val.err = (code_msg_err) {
				.code = errno, .msg = strerror(errno)
			};
		}
		goto ERROR;
	}
	f->src_path = cstr_clone(filename);
	f->linebuf = strbuf_new();
	f->seqbuf = strbuf_new();
	f->cur_line = 1;
	f->cur_offset = 0;
	f->cur_rec = (fastq_rec) {
		.id=NULL, .descr=NULL, .seq=NULL, .qlty=NULL, .seq_len = 0, .id_offset=0, .seq_offset=0, .qlty_offset=0
	};
	goto SUCCESS;
ERROR:
	FREE(f);
	f = NULL;
SUCCESS:
	result.ok = true;
	result.val.ok = f;
	return result;
}


const char *fastq_path(fastq *self)
{
	return self->src_path;
}


//static bool _goto_next(fastq *self)
//{
//	int c;
//	strread *rdr = strfilereader_as_strread(self->freader);
//	while (!strread_eof(rdr)) {
//		if ((c = (strread_getc(rdr))) == '@') {
//			ungetc(c, self->src);
//			return true;
//		}
//	}
//	return false;
//}


bool fastq_has_next(fastq *self)
{
	strread *reader = strfilereader_as_strread(self->freader);
	int c;
	while ((c = strread_peek(reader)) != EOF) {
		if (c == '\n') {
			c = strread_getc(reader);
			self->cur_offset++;
			self->cur_line++;
		} else if (isspace(c)) {
			c = strread_getc(reader);
			self->cur_offset++;
		} else if (c == '@') {
			return true;
		} else {
			ERROR_ASSERT(false, "Unexpected char at line %zu of fastq file %s: expected '@' but found %c\n", self->cur_line, self->src_path, c);
		}
	}
	return false;
}


//bool fastq_goto(fastq *self, size_t descr_offset)
//{
//	return ((fseek(self->src, descr_offset, SEEK_SET) == 0) &&
//	        _goto_next(self));
//}


void fastq_rewind(fastq *self)
{
	strread_reset(strfilereader_as_strread(self->freader));
	self->cur_line = 1;
	self->cur_offset = 0;
}


fastq_next_res fastq_next(fastq *self)
{
	strread *reader = strfilereader_as_strread(self->freader);

	// load description
	self->cur_rec.id_offset = self->cur_offset;
	strbuf_clear(self->linebuf);
	self->cur_offset += strread_read_strbuf(reader, self->linebuf);
	self->cur_line++;
	WARN_ASSERT(strbuf_get(self->linebuf, 0) == '@',
	            "Expected '@' at line %ld of %s.\n", self->cur_line, self->src_path);
	if (strbuf_get(self->linebuf, 0) != '@') {
		return (fastq_next_res) {
			.ok = false, .val.err = {.err_code = FASTQ_EXPECTED_AT, .line_no = self->cur_line }
		};
	}
	strbuf_clip(self->linebuf, 1, strbuf_len(self->linebuf));
	strbuf_trim(self->linebuf);
	strview line_sv = strbuf_as_strview(self->linebuf);
	size_t id_len = strview_find(&line_sv, " ", 1);
	strview id_sv = strview_substr(&line_sv, 0, id_len);
	self->cur_rec.id = cstr_ensure_capacity(self->cur_rec.id, id_len);
	WARN_ASSERT(id_len > 0,
	            "Empty sequence identifier at line %ld of %s.\n", self->cur_line, self->src_path);
	if (id_len == 0) {
		return (fastq_next_res) {
			.ok = false, .val.err = {.err_code = FASTQ_EMPTY_ID, .line_no = self->cur_line }
		};
	}
	strview_cpy_to_cstr(&id_sv, self->cur_rec.id);
	strbuf_clip(self->linebuf, id_len, strbuf_len(self->linebuf));
	strbuf_trim(self->linebuf);
	self->cur_rec.descr = cstr_reassign(self->cur_rec.descr, strbuf_as_str(self->linebuf));

	// load sequence
	self->cur_rec.seq_offset = self->cur_offset;
	strbuf_clear(self->seqbuf);
	for (bool plusline = false; !plusline; ) {
		strbuf_clear(self->linebuf);
		self->cur_offset += strread_read_strbuf(reader, self->linebuf);
		self->cur_line++;
		strbuf_trim(self->linebuf);
		plusline =  (strbuf_len(self->linebuf)>0 && strbuf_get(self->linebuf, 0) == '+');
		if (!plusline) {
			strbuf_cat(self->seqbuf, self->linebuf);
		}
	}
	WARN_ASSERT(strbuf_len(self->seqbuf) > 0,
	            "Empty sequence at line %ld of %s.\n", self->cur_line, self->src_path);
	if (strbuf_len(self->seqbuf) == 0) {
		return (fastq_next_res) {
			.ok = false, .val.err = {.err_code = FASTQ_EMPTY_SEQUENCE, .line_no = self->cur_line }
		};
	}
	self->cur_rec.seq_len = strbuf_len(self->seqbuf);
	self->cur_rec.seq  = cstr_reassign(self->cur_rec.seq, strbuf_as_str(self->seqbuf));
	WARN_ASSERT(strbuf_get(self->linebuf, 0) == '+',
	            "Expected '+' at line %ld of %s.\n", self->cur_line, self->src_path);
	if (strbuf_get(self->linebuf, 0) != '+') {
		return (fastq_next_res) {
			.ok = false, .val.err = {.err_code = FASTQ_EXPECTED_PLUS, .line_no = self->cur_line }
		};
	}
	strbuf_clip(self->linebuf, 1, strbuf_len(self->linebuf));

	// load qualities
	self->cur_rec.qlty_offset = self->cur_offset;
	strbuf_clear(self->seqbuf);
	while (!strread_eof(reader) && strbuf_len(self->seqbuf) < self->cur_rec.seq_len) {
		strbuf_clear(self->linebuf);
		self->cur_offset += strread_read_strbuf(reader, self->linebuf);
		self->cur_line++;
		strbuf_trim(self->linebuf);
		strbuf_cat(self->seqbuf, self->linebuf);
	}
	WARN_ASSERT(strbuf_len(self->seqbuf) == self->cur_rec.seq_len, "Wrong quality sequence length. Expected %zu != Actual %zu\n", self->cur_rec.seq_len, strbuf_len(self->seqbuf));
	if (strbuf_len(self->seqbuf) != self->cur_rec.seq_len) {
		return (fastq_next_res) {
			.ok = false, .val.err = {.err_code = FASTQ_WRONG_QLTY_SEQUENCE_LENGTH, .line_no = self->cur_line }
		};
	}
	self->cur_rec.qlty  = cstr_reassign(self->cur_rec.qlty, strbuf_as_str(self->seqbuf));

	return (fastq_next_res) {
		.ok = true, .val.ok = &(self->cur_rec)
	};
}


// const fastq_rec_rdr *fastq_next_reader(fastq *self)
// {
// 	if (!_goto_next(self)) {
// 		return NULL;
// 	}
// 	// load description
// 	self->cur_rec_rd.descr_offset = ftell(self->src);
// 	ERROR_ASSERT(fgetc(self->src) == '>',
// 	             "Expected '>' at position %ld of %s.\n", self->cur_rec_rd.descr_offset,
// 	             self->src_path );
// 	cstr_clear(self->cur_rec_rd.descr, self->cur_rec_rd_len[0]);
// 	size_t l = 0;
// 	bool eol = false;
// 	while (!eol) {
// 		if (self->cur_rec_rd_len[0] == l) {
// 			self->cur_rec_rd_len[0] *= 1.66f;
// 			//(size_t)(1.66f * self->cur_rec_rd_len[0]);
// 			self->cur_rec_rd.descr = cstr_resize(self->cur_rec_rd.descr,
// 			                                     self->cur_rec_rd_len[0]);
// 		}
// 		fgets(self->cur_rec_rd.descr + l, self->cur_rec_rd_len[0] - l + 1, self->src);
// 		l = strlen(self->cur_rec_rd.descr);
// 		while (self->cur_rec_rd.descr[l - 1] == '\n'
// 		        || self->cur_rec_rd.descr[l - 1] == '\r') {
// 			self->cur_rec_rd.descr[--l] = '\0';
// 			eol = true;
// 		}
// 	}
// 	// load sequence reader
// 	self->cur_rec_rd.seq_offset = ftell(self->src);
// 	self->rd.file_pos = ftell(self->src);
// 	return &(self->cur_rec_rd);
// }
//

void fastq_close(fastq *self)
{
	FREE(self->src_path);
	strfilereader_free(self->freader);
	strbuf_free(self->linebuf);
	strbuf_free(self->seqbuf);
	FREE(self->cur_rec.id);
	FREE(self->cur_rec.descr);
	FREE(self->cur_rec.seq);
	FREE(self->cur_rec.qlty);
	FREE(self);
}
