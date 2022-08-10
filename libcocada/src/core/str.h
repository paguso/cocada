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

#ifndef STR_H
#define STR_H

/**
 * @file str.h
 * @author Paulo Fonseca
 * Unmutable string
 */

#include <stddef.h>

#include "new.h"

typedef struct str str;

str str_new(const char *src);

str str_new_len(const char *src, size_t len);

str cstr_to_str(char *src);

str cstr_to_str_len(char *src, size_t len);

void str_finalise(void *ptr, const finaliser *fnr);

size_t str_len(str s);

const char *str_as_cstr(str s);

#endif