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

#include "errlog.h"
#include "format.h"
#include "xstr.h"
#include "xstrformat.h"


struct _xstrformat {
	format _t_format;
	const xstr *src;
};


IMPL_TRAIT(xstrformat, format)


static void fprint_ascii(format *self, FILE *stream)
{
	const xstr *xs = ((xstrformat *)(self->impltor))->src;
	WARN_ASSERT(xstr_sizeof_char(xs)==sizeof(char),
	            "Formatting xstr @%p with char size = %zubytes as ASCII.\n", xs,
	            xstr_sizeof_char(xs));
	fprintf(stream, "xstr@%p {\n", xs);
	fprintf(stream, "  len : %zu\n", xstr_len(xs) );
	fprintf(stream, "  sizeof_char: %zu\n", xstr_sizeof_char(xs));
	fprintf(stream, "  str: %s\n", (char *)xstr_as_bytes(xs));
	fprintf(stream, "}\n");
}

static format_vt xstrformat_ascii_vt = {.fprint = fprint_ascii};


static void fprint_xchar(format *self, FILE *stream)
{
	const xstr *xs = ((xstrformat *)(self->impltor))->src;
	WARN_ASSERT(xstr_sizeof_char(xs)==XCHAR_BYTESIZE,
	            "Formatting xstr @%p with char size = %zu bytes as a sequence of xchars, whereas XCHAR_BYTESIZE=%d\n",
	            xs, xstr_sizeof_char(xs), XCHAR_BYTESIZE);
	fprintf(stream, "xstr@%p {\n", xs);
	fprintf(stream, "  len : %zu\n", xstr_len(xs) );
	fprintf(stream, "  sizeof_char: %zu\n", xstr_sizeof_char(xs));
	fprintf(stream, "  str:");
	FOREACH_IN_XSTR(c, xs) {
		fprintf(stream, "[%"XCHAR_FMT"]", c);
	}
	fprintf(stream, "\n");
	fprintf(stream, "}\n");
}

static format_vt xstrformat_xchar_vt = {.fprint = fprint_xchar};


xstrformat *xstrformat_new(const xstr *src)
{
	xstrformat *ret = NEW(xstrformat);
	ret->_t_format = (format) {
		.impltor=ret, .vt=xstrformat_xchar_vt
	};
	ret->src = src;
	return ret;
}


xstrformat *xstrformat_new_ascii(const xstr *src)
{
	xstrformat *ret = NEW(xstrformat);
	ret->_t_format = (format) {
		.impltor=ret, .vt=xstrformat_ascii_vt
	};
	ret->src = src;
	return ret;
}



void xstrformat_free(xstrformat *self)
{
	FREE(self);
}
