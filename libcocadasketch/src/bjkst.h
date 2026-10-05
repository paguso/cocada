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

#ifndef BJKST_H
#define BJKST_H

#include <stdint.h>


/**
 * @file bjkst.h
 * @author Paulo Fonseca
 * @brief Approximate number of distinct values in a stream (BJKST algorithm).
 *
 * Implements the algorithm of Bar-Yossef, Jayram, Kumar, Sivakumar and
 * Trevisan (BJKST) to estimate the number of distinct values in a stream of
 * integers of a given number of bits. The accuracy is set by an error
 * parameter `eps` and a failure probability `delta` (see #bjkst_init); the
 * memory used grows as 1/`eps`^2.
 */

typedef struct _bjkst bjkst;


/**
 * @param nbits Number of bits of elements in the stream
 * @param eps Error parameter
 * @param delta Error probability parameter s.t. Pr[ |estimate - real|>=eps] < delta
 */
bjkst *bjkst_init(size_t nbits, double eps, double delta);

void bjkst_process(bjkst *counter, uint64_t val);

uint64_t bjkst_qry(bjkst *counter);


#endif
