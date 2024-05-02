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

#ifndef __XSTRHASH_H__
#define __XSTRHASH_H__

#include <stddef.h>
#include <stdint.h>

#include "alphabet.h"
#include "new.h"
#include "xstr.h"

typedef struct _xstrHash xstrHash;


/**
 * @param ab (move) alphabet
 */
xstrHash *xstrhash_new(Alphabet *ab);

void xstrhash_finalise(void *ptr, const Finaliser *fnr);

uint64 xstrhash_lex(const xstrHash *self, const xstr *s);

uint64 xstrhash_lex_sub(const xstrHash *self, const xstr *s, usize from,
                        usize to);

uint64 xstrhash_roll_lex(const xstrHash *self, const xstr *s, uint64 hash,
                         xchar c);

uint64 xstrhash_roll_lex_sub(const xstrHash *self, const xstr *s, usize from,
                             usize to, uint64 hash, xchar c);


#endif
