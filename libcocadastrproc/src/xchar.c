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

#include <byteswap.h>
#include <stdint.h>

#include "xchar.h"

void xchar_flip_bytes(xchar_t *c)
{
#if XCHAR_BYTESIZE==2
	*c = bswap_16(*c);
#elif XCHAR_BYTESIZE==4
	*c = bswap_32(*c);
#elif XCHAR_BYTESIZE==8
	*c = bswap_64(*c);
#endif
}