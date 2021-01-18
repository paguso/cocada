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
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cstrutil.h"
#include "fasta.h"
#include "new.h"
#include "strread.h"
#include "trait.h"
#include "errlog.h"

typedef struct _fastaread {
	strread _t_strread;
	FILE *src;
	size_t file_pos;
} fastaread;


IMPL_TRAIT(fastaread, strread)


static void _reset(strread *self)
{
	fastaread *fr = (fastaread *)self->impltor;
	fseek(fr->src, fr->file_pos, SEEK_SET);
}


static int _getc(strread *self)
{
	fastaread *fr = (fastaread *)self->impltor;
	int c;
	while (true) {
		c = fgetc(fr->src);
		if (c=='>') {
			ungetc(c, fr->src);
			c = EOF;
			break;
		}
		else if (c=='\n'|| c=='\r') {
			continue;
		}
		else {
			break;
		}
	}
	return c;
}


static size_t _read_str(strread *self, char *dest, size_t n)
{
	FILE *src = ((fastaread *)self->impltor)->src;
	char *origdest = dest;
	memset(dest, '\0', n+1);
	while ( !feof(src) && n > 0 ) {
		fgets(dest, n+1, src);
		size_t l = strlen(dest);
		if (dest[0] == '>') {
			memset(dest, '\0', l );
			break;
		}
		while (l > 0 && ( dest[l-1]=='\n' || dest[l-1]=='\r' )) {
			dest[l-1] = '\0';
			l--;
		}
		dest += l;
		n -= l;
	}
	return dest-origdest;
}


static size_t _read_str_until(strread *self, char *dest, char delim)
{
	FILE *src = ((fastaread *)self->impltor)->src;
	size_t nread = 0;
	char c;
	while ( !feof(src) ) {
		c = fgetc(src);
		if ( c == delim || c == '>' ) {
			ungetc(c, src);
			break;
		}
		else if ( c == '\n' || c=='\r' ) {
			continue;
		}
		dest[nread++] = c;
	}
	dest[nread] = '\0';
	return nread;
}


static strread_vt _strread_vt  = {
	.getc = _getc,
	.read_str = _read_str,
	.read_str_until = _read_str_until,
	.reset = _reset,
};


static void _fastaread_init(fastaread *ret, FILE *src)
{

	ret->src = src;
	ret->_t_strread.impltor = ret;
	ret->_t_strread.vt = &_strread_vt;
}



struct _fasta {
	FILE *src;
	char *src_path;
	fasta_rec cur_rec;
	size_t cur_rec_len[2];
	fastaread rd;
	fasta_rec_rdr cur_rec_rd;
	size_t cur_rec_rd_len[2];
};


fasta *fasta_open(char *filename)
{
	fasta *ret = NEW(fasta);
	ret->src = fopen(filename, "r");
	ret->src_path = cstr_clone(filename);
	_fastaread_init(&(ret->rd), ret->src);
	ret->cur_rec_len[0] = ret->cur_rec_len[1] = 100;
	ret->cur_rec_rd_len[0] = ret->cur_rec_rd_len[1] = 100;
	ret->cur_rec.descr = cstr_new(ret->cur_rec_len[0]);
	ret->cur_rec.seq = cstr_new(ret->cur_rec_len[1]);
	ret->cur_rec_rd.descr = cstr_new(ret->cur_rec_rd_len[0]);
	ret->cur_rec_rd.seqrdr = fastaread_as_strread(&(ret->rd));
	return ret;
}


const char *fasta_path(fasta *self)
{
	return self->src_path;
}


static bool _goto_next(fasta *self)
{
	int c;
	while (!feof(self->src)) {
		if ((c=fgetc(self->src)) == '>') {
			ungetc(c, self->src);
			return true;
		}
	}
	return false;
}


bool fasta_has_next(fasta *self)
{
	long cur = ftell(self->src);
	bool ret = _goto_next(self);
	fseek(self->src, cur, SEEK_SET);
	return ret;
}


const fasta_rec *fasta_next(fasta *self)
{
	if (!_goto_next(self)) {
		return NULL;
	}
	// load description
	self->cur_rec.descr_offset = ftell(self->src);
	ERROR_ASSERT(fgetc(self->src) == '>',
	             "Expected '>' at position %ld of %s.\n", self->cur_rec.descr_offset,
	             self->src_path );
	cstr_clear(self->cur_rec.descr, self->cur_rec_len[0]);
	size_t l = 0;
	bool eol = false;
	while (!eol) {
		if (self->cur_rec_len[0] == l) {
			self->cur_rec_len[0] *= 1.66f;
			self->cur_rec.descr = cstr_resize(self->cur_rec.descr, self->cur_rec_len[0]);
		}
		fgets(self->cur_rec.descr + l, self->cur_rec_len[0] - l + 1, self->src);
		l = strlen(self->cur_rec.descr);
		while (self->cur_rec.descr[l-1]=='\n' || self->cur_rec.descr[l-1]=='\r') {
			self->cur_rec.descr[--l] = '\0';
			eol = true;
		}
	}
	// load sequence
	self->cur_rec.seq_offset = ftell(self->src);
	cstr_clear(self->cur_rec.seq, self->cur_rec_len[1]);
	l = 0;
	while ( !feof(self->src) ) {
		if (self->cur_rec_len[1] == l) {
			self->cur_rec_len[1] *= 1.66f;
			//(size_t)(1.66f * self->cur_rec_len[1]);
			self->cur_rec.seq = cstr_resize(self->cur_rec.seq, self->cur_rec_len[1]);
		}
		fgets(self->cur_rec.seq + l, self->cur_rec_len[1] - l + 1, self->src);
		if (self->cur_rec.seq[l] == '>') {
			fseek(self->src, l - strlen(self->cur_rec.seq), SEEK_CUR);
			self->cur_rec.seq[l] = '\0';
			break;
		}
		l = strlen(self->cur_rec.seq);
		while (self->cur_rec.seq[l-1]=='\n' || self->cur_rec.seq[l-1]=='\r') {
			self->cur_rec.seq[--l] = '\0';
		}
	}
	return &(self->cur_rec);
}


const fasta_rec_rdr *fasta_next_reader(fasta *self)
{
	if (!_goto_next(self)) {
		return NULL;
	}
	// load description
	self->cur_rec_rd.descr_offset = ftell(self->src);
	ERROR_ASSERT(fgetc(self->src) == '>',
	             "Expected '>' at position %ld of %s.\n", self->cur_rec_rd.descr_offset,
	             self->src_path );
	cstr_clear(self->cur_rec_rd.descr, self->cur_rec_rd_len[0]);
	size_t l = 0;
	bool eol = false;
	while (!eol) {
		if (self->cur_rec_rd_len[0] == l) {
			self->cur_rec_rd_len[0] *= 1.66f;
			//(size_t)(1.66f * self->cur_rec_rd_len[0]);
			self->cur_rec_rd.descr = cstr_resize(self->cur_rec_rd.descr,
			                                     self->cur_rec_rd_len[0]);
		}
		fgets(self->cur_rec_rd.descr + l, self->cur_rec_rd_len[0] - l + 1, self->src);
		l = strlen(self->cur_rec_rd.descr);
		while (self->cur_rec_rd.descr[l-1]=='\n' || self->cur_rec_rd.descr[l-1]=='\r') {
			self->cur_rec_rd.descr[--l] = '\0';
			eol = true;
		}
	}
	// load sequence reader
	self->cur_rec_rd.seq_offset = ftell(self->src);
	self->rd.file_pos = ftell(self->src);
	return &(self->cur_rec_rd);
}


void fasta_close(fasta *self)
{
	fclose(self->src);
	FREE(self->src_path);
	FREE(self->cur_rec.descr);
	FREE(self->cur_rec.seq);
	FREE(self->cur_rec_rd.descr);
}