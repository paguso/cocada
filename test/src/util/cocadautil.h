/*
 * 
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

#ifndef COCADAUTIL_H
#define COCADAUTIL_H

#include <stdbool.h>
#include <stdlib.h>

/**
 * @file cocadautil.h
 * @author Paulo Fonseca
 *
 * @brief Assorted definitions, macros and utility functions.
 */


/**
 * NULL pointer constant.
 */
#if !defined(NULL)
#define NULL ((void *)0)
#endif


/**
 * Min, max
 */
#define MIN( A, B )   ( (A)<(B) ? (A) : (B) )
#define MAX( A, B )   ( (A)>(B) ? (A) : (B) )

#define MIN3( A, B, C ) ((A) < (B) ? MIN(A, C) : MIN(B, C))
#define MAX3( A, B, C ) ((A) > (B) ? MAX(A, C) : MAX(B, C))


/**
 * Allocate, deallocate
 */
#define NEW( TYPE )   ((TYPE*)(malloc(sizeof(TYPE))))
#define FREE( PTR )   free(PTR)


/**
 * The maximum value of a pointer
 */
#define PTR_MAX SIZE_MAX


#endif