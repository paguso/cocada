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

#include "strread.h"
#include "strfileread.h"
#include "new.h"

struct _strfileread {
	strread _t_strread;
	FILE *src;
	size_t pos;
};


IMPL_TRAIT(strfileread, strread)



static void _reset(void *self)
{
	rewind( ((strfileread *)self)->src );
}


static char _getc(void *self)
{
	return fgetc( ((strfileread *)self)->src );
}


static size_t _read_str(void *self, char *dest, size_t n)
{
	return fread(dest, sizeof(char), n, ((strfileread *)self)->src);
}


static size_t _read_str_until(void *self, char *dest, char delim)
{
	FILE *src = ((strfileread *)self)->src;
	size_t nread;
	char c;
	for (nread=0; !feof(src); nread++) {
		if ( (c=fgetc(src)) == delim ) {
			fseek(src, -1, SEEK_CUR);
			break;
		}
		dest[nread] = c;
	}
	return nread;
}


static strread_vt _strread_vt  = {
	.getc = _getc,
	.read_str = _read_str,
	.read_str_until = _read_str_until,
	.reset = _reset
};


strfileread *strfileread_open(char *filename)
{
	strfileread *ret = NEW(strfileread);
	ret->_t_strread.impltor = ret;
	ret->_t_strread.vtbl = &_strread_vt;
	ret->src = fopen(filename, "r");
	ret->pos = 0;
	return ret;
}


void strfileread_close(strfileread *self)
{
	fclose(self->src);
	FREE(self);
}