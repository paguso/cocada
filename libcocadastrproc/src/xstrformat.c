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
#include "strbuf.h"
#include "xstr.h"
#include "xstrformat.h"


struct _xstrformat {
	format _t_format;
	const xstr *src;
};


IMPL_TRAIT(xstrformat, format)



#define PRINT_ASCII(TYPE)\
	int ret = 0;\
	const xstr *xs = ((xstrformat *)(self->impltor))->src;\
	WARN_ASSERT(xstr_sizeof_char(xs)==sizeof(char),\
	            "Formatting xstr @%p with char size = %zubytes as ASCII.\n", xs,\
	            xstr_sizeof_char(xs));\
	ret += TYPE##printf(out, "xstr@%p {\n", xs);\
	ret += TYPE##printf(out, "  len : %zu\n", xstr_len(xs) );\
	ret += TYPE##printf(out, "  sizeof_char: %zu\n", xstr_sizeof_char(xs));\
	ret += TYPE##printf(out, "  str: %s\n", (char *)xstr_as_bytes(xs));\
	ret += TYPE##printf(out, "}\n");\
	return ret;


static int fprint_ascii(format *self, FILE *out)
{
	PRINT_ASCII(f)
}

static int sprint_ascii(format *self, char *out)
{
	PRINT_ASCII(s)
}


static int sbprint_ascii(format *self, strbuf *out)
{
	PRINT_ASCII(sb)
}


static format_vt xstrformat_ascii_vt = {.fprint = fprint_ascii, .sprint = sprint_ascii, .sbprint = sbprint_ascii};


#define PRINT_XCHAR(TYPE)\
	int ret = 0;\
	const xstr *xs = ((xstrformat *)(self->impltor))->src;\
	WARN_ASSERT(xstr_sizeof_char(xs)==XCHAR_BYTES,\
	            "Formatting xstr @%p with char size = %zu bytes as a sequence of xchars, whereas XCHAR_BYTES=%d\n",\
	            xs, xstr_sizeof_char(xs), XCHAR_BYTES);\
	ret += TYPE##printf(out, "xstr@%p {\n", xs);\
	ret += TYPE##printf(out, "  len : %zu\n", xstr_len(xs) );\
	ret += TYPE##printf(out, "  sizeof_char: %zu\n", xstr_sizeof_char(xs));\
	ret += TYPE##printf(out, "  str:");\
	FOREACH_IN_XSTR(c, xs) {\
		ret += TYPE##printf(out, "[%"XCHAR_FMT"]", c);\
	}\
	ret += TYPE##printf(out, "\n");\
	ret += TYPE##printf(out, "}\n");\
	return ret;


static int fprint_xchar(format *self, FILE *out)
{
	PRINT_XCHAR(f)
}


static int sprint_xchar(format *self, char *out)
{
	PRINT_XCHAR(s)
}


static int sbprint_xchar(format *self, strbuf *out)
{
	PRINT_XCHAR(sb)
}


static format_vt xstrformat_xchar_vt = {.fprint = fprint_xchar, .sprint = sprint_xchar, .sbprint = sbprint_xchar};


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
