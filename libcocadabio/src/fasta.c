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
#include "fasta.h"
#include "new.h"
#include "strbuf.h"
#include "strfilereader.h"
#include "strread.h"
#include "strview.h"
#include "trait.h"
#include "errlog.h"


const char *const fasta_err_msg[3] = {
	"Expected record header start >",
	"Empty record id",
	"Empty sequence",
};

struct _fasta {
	char *path;
	strfilereader *freader;
	//size_t cur_line;
	size_t off;
	strbuf *linebuf;
	strbuf *seqbuf;
	fasta_rec rec;
};


fasta_open_res fasta_open(const char *filename)
{
	fasta_open_res result = {.ok = true};
	fasta *f = NEW(fasta);
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
	f->path = cstr_clone(filename);
	f->linebuf = strbuf_new();
	f->seqbuf = strbuf_new();
	//f->cur_line = 1;
	f->off = 0;
	f->rec = (fasta_rec) {
		.id=NULL, .descr=NULL, .seq=NULL, .seq_len = 0, .hdr_off=0, .seq_off=0
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


const char *fasta_path(fasta *self)
{
	return self->path;
}


bool fasta_has_next(fasta *self)
{
	strread *reader = strfilereader_as_strread(self->freader);
	int c;
	while ((c = strread_peek(reader)) != EOF) {
		if (c == '\n') {
			c = strread_getc(reader);
			self->off++;
			//self->cur_line++;
		} else if (isspace(c)) {
			c = strread_getc(reader);
			self->off++;
		} else if (c == '>') {
			return true;
		} else {
			ERROR_ASSERT(false, "Unexpected char at offset %zu of fasta file %s: expected '>' but found %c\n", self->off, self->path, c);
		}
	}
	return false;
}


void fasta_rewind(fasta *self)
{
	strread_reset(strfilereader_as_strread(self->freader));
	self->off = 0;
}


bool fasta_goto(fasta *self, size_t offset)
{
	if (strfilereader_goto(self->freader, offset) &&
	        strread_peek(strfilereader_as_strread(self->freader)) == '>') {
		self->off = offset;
		return true;
	} else {
		fasta_rewind(self);
		return false;
	}
}


fasta_next_res fasta_next(fasta *self)
{
	strread *reader = strfilereader_as_strread(self->freader);

	// load description
	//self->cur_rec.line = self->cur_line;
	self->rec.hdr_off = self->off;
	strbuf_clear(self->linebuf);
	self->off += strread_read_strbuf(reader, self->linebuf);
	//self->cur_line++;
	WARN_ASSERT(strbuf_get(self->linebuf, 0) == '>',
	            "Expected header start '>' at offset  %zu of %s.\n", self->off, self->path);
	if (strbuf_len(self->linebuf) == 0 || strbuf_get(self->linebuf, 0) != '>') {
		return (fasta_next_res) {
			.ok = false, .val.err = {.code = FASTA_MISSING_HDR, .off = self->rec.hdr_off }
		};
	}
	strbuf_clip(self->linebuf, 1, strbuf_len(self->linebuf));
	strbuf_trim(self->linebuf);
	strview line_sv = strbuf_as_strview(self->linebuf);
	size_t id_len = strview_find(&line_sv, " ", 1);
	strview id_sv = strview_substr(&line_sv, 0, id_len);
	self->rec.id = cstr_ensure_capacity(self->rec.id, id_len);
	WARN_ASSERT(id_len > 0,
	            "Empty sequence identifier at offset %ld of %s.\n", self->off, self->path);
	if (id_len == 0) {
		return (fasta_next_res) {
			.ok = false, .val.err = {.code = FASTA_EMPTY_ID, .off = self->off }
		};
	}
	strview_cpy_to_cstr(&id_sv, self->rec.id);
	strbuf_clip(self->linebuf, id_len, strbuf_len(self->linebuf));
	strbuf_trim(self->linebuf);
	self->rec.descr = cstr_reassign(self->rec.descr, strbuf_as_str(self->linebuf));

	// load sequence
	self->rec.seq_off = self->off;
	strbuf_clear(self->seqbuf);
	int c;
	while ((c=strread_peek(reader))!=EOF && c != '>') {
		strbuf_clear(self->linebuf);
		self->off += strread_read_strbuf(reader, self->linebuf);
		//self->cur_line++;
		strbuf_trim(self->linebuf);
		strbuf_cat(self->seqbuf, self->linebuf);
	}
	WARN_ASSERT(strbuf_len(self->seqbuf) > 0,
	            "Empty sequence at offset %ld of %s.\n", self->off, self->path);
	if (strbuf_len(self->seqbuf) == 0) {
		return (fasta_next_res) {
			.ok = false, .val.err = {.code = FASTA_EMPTY_SEQ, .off = self->off }
		};
	}
	self->rec.seq_len = strbuf_len(self->seqbuf);
	self->rec.seq  = cstr_reassign(self->rec.seq, strbuf_as_str(self->seqbuf));

	return (fasta_next_res) {
		.ok = true, .val.ok = &(self->rec)
	};
}


void fasta_close(fasta *self)
{
	FREE(self->path);
	strfilereader_free(self->freader);
	strbuf_free(self->linebuf);
	strbuf_free(self->seqbuf);
	FREE(self->rec.id);
	FREE(self->rec.descr);
	FREE(self->rec.seq);
	FREE(self);
}
