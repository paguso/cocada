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

#include "strread.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>



strread_vt strread_vt_new()
{
	strread_vt vt;
	memset(&vt, 0x0, sizeof(strread_vt)); // set all functions to NULL
	return vt;
}


void strread_reset(strread *self)
{
	self->vt->reset(self);
}


int strread_getc(strread *self)
{
	return self->vt->getc(self);
}


int strread_ungetc(strread *self)
{
	return self->vt->ungetc(self);
}


size_t strread_read_str(strread *self, char *dest, size_t n)
{
	return self->vt->read_str(self, dest, n);
}


size_t strread_read_str_until(strread *self, char *dest, char delim)
{
	return self->vt->read_str_until(self, dest, delim);
}





