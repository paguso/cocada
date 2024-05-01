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


#include "xstrread.h"


void xstrread_reset(xstrRead *trait)
{
	trait->vt->reset(trait);
}


xwchar xstrread_getc(xstrRead *trait)
{
	return trait->vt->getch(trait);
}


size_t xstrread_read(xstrRead *trait, xstr *dest, size_t n)
{
	return trait->vt->read(trait, dest, n);
}


size_t xstrread_read_until(xstrRead *trait,  xstr *dest, xchar delim)
{
	return trait->vt->read_until(trait, dest, delim);
}



