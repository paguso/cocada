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

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "read.h"
#include "filereader.h"
#include "new.h"



struct _FileReader {
	Read _t_Read;
	FILE *src;
	usize pos;
	bool own_stream;
};


IMPL_TRAIT(FileReader, Read)


static void _reset(Read *self)
{
	rewind( ((FileReader *)self->impltor)->src );
}


static int _getc(Read *self)
{
	return fgetc( ((FileReader *)self->impltor)->src );
}


static usize _read_str(Read *self, char *dest, usize n)
{
	return fread(dest, sizeof(char), n, ((FileReader *)self->impltor)->src);
}


static usize _read_str_until(Read *self, char *dest, char delim)
{
	FILE *src = ((FileReader *)self->impltor)->src;
	usize nread;
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


static Read_vt _strfilereader_vt  = {
	.getc = _getc,
	.read_str = _read_str,
	.read_str_until = _read_str_until,
	.reset = _reset
};


FileReader *filereader_new_from_path(const char *path)
{
	FILE *src = fopen(path, "r");
	if (!src) {
		return NULL;
	}
	FileReader *ret = NEW(FileReader);
	ret->_t_Read.impltor = ret;
	ret->_t_Read.vt = &_strfilereader_vt;
	ret->src = src;
	ret->own_stream = true;
	ret->pos = 0;
	return ret;
}


FileReader *filereader_new(FILE *stream)
{
	FileReader *ret = NEW(FileReader);
	ret->_t_Read.impltor = ret;
	ret->_t_Read.vt = &_strfilereader_vt;
	ret->src = stream;
	ret->own_stream = false;
	ret->pos = 0;
	return ret;
}



void filereader_free(FileReader *self)
{
	if (self->own_stream) {
		fclose(self->src);
	}
	FREE(self);
}
