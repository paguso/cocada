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


#ifndef XSTRREADER_H
#define XSTRREADER_H

#include "trait.h"
#include "xstr.h"

typedef struct _xstrreader xstrreader;

xstrreader *xstrreader_open(xstr *src);

xstrreader *xstrreader_open_str(char *src, size_t len);

void xstrreader_close(xstrreader *self);

DECL_TRAIT(xstrreader, xstrread)


#endif