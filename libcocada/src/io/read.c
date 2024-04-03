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

#include "read.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>



Read_vt read_vt_new()
{
	Read_vt vt;
	memset(&vt, 0x0, sizeof(Read_vt)); // set all functions to NULL
	return vt;
}


void read_reset(Read *self)
{
	self->vt->reset(self);
}


int read_getc(Read *self)
{
	return self->vt->getc(self);
}


int read_ungetc(Read *self)
{
	return self->vt->ungetc(self);
}


size_t read_read_str(Read *self, char *dest, size_t n)
{
	return self->vt->read_str(self, dest, n);
}


size_t read_read_str_until(Read *self, char *dest, char delim)
{
	return self->vt->read_str_until(self, dest, delim);
}





