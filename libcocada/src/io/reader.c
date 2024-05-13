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

#include "reader.h"

#include <stddef.h>
#include <stdbool.h>



void reader_reset(Reader *self)
{
	self->vt->reset(self);
}


int reader_getc(Reader *self)
{
	return self->vt->getc(self);
}


int reader_ungetc(Reader *self)
{
	return self->vt->ungetc(self);
}


usize reader_read_str(Reader *self, char *dest, usize n)
{
	return self->vt->read_str(self, dest, n);
}


usize reader_read_str_until(Reader *self, char *dest, char delim)
{
	return self->vt->read_str_until(self, dest, delim);
}





