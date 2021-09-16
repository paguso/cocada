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
#include <stddef.h>
#include <stdlib.h>

#include "errlog.h"

#if defined(__GNUC__)

#include "execinfo.h"

void print_trace(FILE *out)
{
	void *array[20];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace (array, 20);
	strings = backtrace_symbols (array, size);

	fprintf (out, "Last %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
		fprintf (out, "%s\n", strings[i]);

	free (strings);
}

#else

void print_trace(FILE *out)
{
	fprintf(out, "Cannot print stack trace.\n");
}

#endif
