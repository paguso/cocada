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
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "strbuf.h"
#include "strread.h"
#include "strfilereader.h"
#include "new.h"


#define _BUFSZ 128
char _buf[_BUFSZ];


struct _strfilereader {
	strread _t_strread;
	FILE *src;
	size_t pos;
	bool own_stream;
};


IMPL_TRAIT(strfilereader, strread)


static void _reset(strread *self)
{
	rewind( ((strfilereader *)self->impltor)->src );
}


static int _getc(strread *self)
{
	return fgetc( ((strfilereader *)self->impltor)->src );
}


static int _peek(strread *self)
{
	FILE *f = ((strfilereader *)self->impltor)->src;
	int c;
	if ((c = fgetc(f)) == EOF) {
		return EOF;
	} else {
		ungetc(c,f);
		return c;
	}
}


static size_t _read_str(strread *self, char *dest, size_t n)
{
	return fread(dest, sizeof(char), n, ((strfilereader *)self->impltor)->src);
}


static size_t _read_strbuf(strread *self, strbuf *dest)
{
	FILE *src = ((strfilereader *)self->impltor)->src;
	size_t ret = 0;
	while(true) {
		size_t nread = 0;
		_buf[0] = '\0';
		_buf[_BUFSZ - 2] = '\0';
		if (fgets(_buf, _BUFSZ, src) != NULL) {
			nread =  strlen(_buf);
			if (nread > 0) {
				// something read
				strbuf_nappend(dest, _buf, nread);
				ret += nread;
				if (_buf[nread - 1] == '\n') {
					// whole line read
					break;
				} else {
					assert(_BUFSZ - 1 == nread);
				}
			}
		}
		if (nread == 0) {
			break;
		}
	}
	return ret;
}


static size_t _read_str_until(strread *self, char *dest, char delim)
{
	FILE *src = ((strfilereader *)self->impltor)->src;
	size_t nread;
	char c;
	for (nread = 0; !feof(src); nread++) {
		if ( (c = fgetc(src)) == delim ) {
			fseek(src, -1, SEEK_CUR);
			break;
		}
		dest[nread] = c;
	}
	return nread;
}


static size_t _read_strbuf_until(strread *self, strbuf *dest, char delim)
{
	FILE *src = ((strfilereader *)self->impltor)->src;
	size_t nread = 0;
	char c;
	while (!feof(src)) {
		if ( (c = fgetc(src)) == delim ) {
			fseek(src, -1, SEEK_CUR);
			break;
		}
		_buf[nread % _BUFSZ] = c;
		nread++;
		if (nread % _BUFSZ == 0) {
			strbuf_nappend(dest, _buf, nread % _BUFSZ);
		}
	}
	strbuf_nappend(dest, _buf, nread % _BUFSZ);
	return nread;
}


static bool _eof(strread *self)
{
	return feof(((strfilereader *)self->impltor)->src);
}


static strread_vt _strread_vt  = {
	.getc = _getc,
	.peek = _peek,
	.read_str = _read_str,
	.read_str_until = _read_str_until,
	.read_strbuf = _read_strbuf,
	.read_strbuf_until = _read_strbuf_until,
	.reset = _reset,
	.eof = _eof
};


strfilereader *strfilereader_new_from_path(const char *path)
{
	FILE *src = fopen(path, "r");
	if (!src) {
		return NULL;
	}
	strfilereader *ret = NEW(strfilereader);
	ret->_t_strread.impltor = ret;
	ret->_t_strread.vt = &_strread_vt;
	ret->src = src;
	ret->own_stream = true;
	ret->pos = 0;
	return ret;
}


strfilereader *strfilereader_new(FILE *stream)
{
	strfilereader *ret = NEW(strfilereader);
	ret->_t_strread.impltor = ret;
	ret->_t_strread.vt = &_strread_vt;
	ret->src = stream;
	ret->own_stream = false;
	ret->pos = 0;
	return ret;
}


bool strfilereader_goto(strfilereader *self, size_t offset)
{
	return fseek(self->src, offset, SEEK_SET) == 0;
}


void strfilereader_free(strfilereader *self)
{
	if (self->own_stream) {
		fclose(self->src);
	}
	FREE(self);
}
