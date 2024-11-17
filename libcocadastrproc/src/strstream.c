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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "new.h"
#include "mathutil.h"
#include "strstream.h"
#include "xchar.h"
#include "xstr.h"


typedef enum {
	SSTR_STR   = 0,
	SSTR_FILE  = 1,
	SSTR_XSTR  = 2,
	SSTR_XFILE = 3,
} StrStreamType;


struct _StrStream {
	union {
		FILE    *file;
		char    *str;
		xstr *xstr;
	} src;
	StrStreamType type;
	usize bytes_per_char;
	usize pos;
	usize slen;
};


static xchar _getchar_from_str(void *str)
{
	StrStream *sst = (StrStream *)str;
	if (sst->pos >= sst->slen)
		return (xchar)EOF;
	else
		return (xchar)sst->src.str[sst->pos++];
}


StrStream *strstream_open_str(char *str, usize slen)
{
	StrStream *sst;
	sst = NEW(StrStream);
	sst->type = SSTR_STR;
	sst->src.str = str;
	sst->pos = 0;
	sst->slen = slen;
	sst->bytes_per_char = sizeof(char);
	return sst;
}


StrStream *strstream_open_xstr(xstr *xstr)
{
	StrStream *sst;
	sst = NEW(StrStream);
	sst->type = SSTR_XSTR;
	sst->src.xstr = xstr;
	sst->pos = 0;
	sst->slen = xstr_len(xstr);
	sst->bytes_per_char = xstr_sizeof_char(xstr);
	return sst;

}

StrStream *strstream_open_file(char *filename)
{
	StrStream *sst;
	sst = NEW(StrStream);
	sst->type = SSTR_FILE;
	sst->src.file = fopen(filename, "r");
	sst->pos = 0;
	sst->bytes_per_char = sizeof(char);
	return sst;
}


StrStream *strstream_open_xfile(char *filename, usize bytes_per_char)
{
	StrStream *sst;
	sst = NEW(StrStream);
	sst->type = SSTR_XFILE;
	sst->bytes_per_char = bytes_per_char;
	sst->src.file = fopen(filename, "rb");
	sst->pos = 0;
	return sst;
}


usize strstream_sizeof_char(StrStream *sst)
{
	return sst->bytes_per_char;
}


void strstream_reset(StrStream *sst)
{
	switch (sst->type) {
	case SSTR_STR:
		sst->pos = 0;
		break;
	case SSTR_FILE:
		rewind(sst->src.file);
		break;
	case SSTR_XSTR:
		sst->pos = 0;
		break;
	case SSTR_XFILE:
		rewind(sst->src.file);
		break;
	}
}

bool strstream_end(StrStream *sst)
{
	switch (sst->type) {
	case SSTR_STR:
		return sst->slen <= sst->pos;
		break;
	case SSTR_FILE:
		return feof(sst->src.file);
		break;
	case SSTR_XSTR:
		return sst->slen <= sst->pos;
		break;
	case SSTR_XFILE:
		return feof(sst->src.file);
		break;
	default:
		return true;
	}
}

xchar strstream_getc(StrStream *sst)
{
	switch (sst->type) {
	case SSTR_STR:
		if (sst->pos >= sst->slen)
			return (xchar)EOF;
		else
			return (xchar)sst->src.str[sst->pos++];
		break;
	case SSTR_FILE:
		return fgetc(sst->src.file);
		break;
	case SSTR_XSTR:
		if (sst->pos >= xstr_len(sst->src.xstr))
			return XEOF;
		else
			return xstr_get(sst->src.xstr, sst->pos++);
		break;
	case SSTR_XFILE:
		;
		xchar ret = 0;
		fread(&ret, sizeof(xchar),  1, sst->src.file);
		return ret;
		break;
	default:
		return '\0';
	}
}

usize strstream_reads(StrStream *sst, char *dest, usize n)
{
	usize nread;
	switch (sst->type) {
	case SSTR_STR:
		nread = MIN(n, (sst->pos < sst->slen) ? (sst->slen - sst->pos) : 0);
		strncpy(dest, sst->src.str + sst->pos, nread);
		sst->pos += nread;
		//dest[nread] = '\0';
		return nread;
		break;
	case SSTR_FILE:
		return fread(dest, 1, n, sst->src.file);
		break;
	default:
		return 0;
	}
}



usize strstream_readxs(StrStream *sst, xstr *dest, usize n)
{
	usize nread;
	switch (sst->type) {
	case SSTR_XSTR:
		nread = MIN(n, (sst->pos < sst->slen) ? (sst->slen - sst->pos) : 0);
		xstr_ncpy(dest, 0, sst->src.xstr, sst->pos, nread);
		sst->pos += nread;
		return nread;
		break;
	case SSTR_XFILE:
		;
		xchar c;
		usize bpc = xstr_sizeof_char(dest);
		usize nread = 0;
		while (nread < n) {
			if (fread(&c, bpc, 1, sst->src.file) == bpc)
				xstr_set(dest, nread++, c);
			else
				break;
		}
		return nread;
		break;
	default:
		return 0;
	}
}


void strstream_close(StrStream *sst)
{
	if (sst == NULL) return;
	switch (sst->type) {
	case SSTR_STR:
		break;
	case SSTR_FILE:
		fclose(sst->src.file);
		break;
	default:
		break;
	}
	FREE(sst);
}
