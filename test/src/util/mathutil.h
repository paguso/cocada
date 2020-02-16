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

#ifndef MATHUTIL_H
#define MATHUTIL_H

/**
 * @file mathutil.h
 * @author Paulo Fonseca
 * 
 * @brief Math utility functions.
 */


#define IS_EVEN( N ) (1-((N)%2))
#define IS_ODD( N ) ((N)%2)


/**
 * @brief Computes the generalised multiplicative floor function defined as
 * the integral multiplicative factor corresponding to largest multiple of a
 * given fixed (finite) base no greater than a given (finite) threshold value.
 * If the base is zero, the function is ill-defined since all its multiples
 * will be equally zero and hence either all of them will be no greater
 * than the threshold, or none of them will. In the latter case, the function
 * shall be define as -INFINITY whereas in the former case it will be defined
 * as zero. In the other cases, the function is well-defined and has
 * a finite value (possibly negative, if value and base have opposite signs).
 * Note that mult_floor(n, 1)==floor(n).
 *
 * @param value  The limiting finite value.
 * @param base The fixed finite base.
 * @return  The integral multiplicative factor corresponding to largest
 * multiple of "base" no greater than "value".
 */
double multfloor(double value, double base);

/**
 * @brief Computes the generalised multiplicative ceil function defined as
 * the integral multiplicative factor corresponding to smallest multiple of a
 * given fixed (finite) base no smaller than a given (finite) threshold value.
 * If the base is zero, the function is ill-defined since all its multiples
 * will be equally zero and hence either all of them will be no smaller
 * than the threshold, or none of them will. In the latter case, the function
 * shall be define as +INFINITY whereas in the former case it will be defined
 * as zero. In the other cases, the function is well-defined and has
 * a finite value (possibly negative, if value and base have opposite signs).
 * Note that mult_ceil(n, 1)==ceil(n).
 *
 * @param value  The limiting finite value.
 * @param base The fixed finite base.
 * @return  The integral multiplicative factor corresponding to smallest
 * multiple of "base" no smaller than "value".
 */
double multceil(double val, double fact);


#endif