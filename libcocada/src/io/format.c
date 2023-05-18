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

#include <stdio.h>

#include "format.h"
#include "strbuf.h"


int format_print(format *self)
{
	return self->vt.fprint(self, stdout);
}


int format_fprint(format *self, FILE *stream)
{
	return self->vt.fprint(self, stream);
}


int format_sprint(format *self, char *dest)
{
	return self->vt.sprint(self, dest);
}


int format_sbprint(format *self, strbuf *buf)
{
	return self->vt.sbprint(self, buf);
}


