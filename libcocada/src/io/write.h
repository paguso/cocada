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

#ifndef WRITE_H
#define WRITE_H

#include <stdio.h>

#include "trait.h"


typedef struct _write write;

/**
 * @brief Writer virtual table
 */
typedef struct {
	int (*write) (write *self, void *buf);
	int (*write_n) (write *self, void *buf, size_t n);
} write_vt;


/**
 * @brief writer trait type
 */
struct _write {
	write_vt *vt;
	void *impltor;
};


int write_write (write *self, void *buf);

int write_write_n (write *self, void *buf, size_t n);



#endif