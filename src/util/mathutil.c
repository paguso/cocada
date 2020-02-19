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

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "mathutil.h"


double multfloor(double value, double base)
{
    if (base>0) {
        if (value>=0) {
            return floor(value/base);
        }
        else {
            return -ceil(-value/base);
        }
    }
    else if (base<0) {
        if (value<=0) {
            return ceil(value/base);
        }
        else {
            return -floor(-value/base);
        }
    }
    else { // base==0
        if (value>=0) {
            return 0;
        }
        else {
            return -INFINITY;
        }
    }
}

double multceil(double value, double base)
{
    if (base>0) {
        if (value>=0) {
            return ceil(value/base);
        }
        else {
            return -floor(-value/base);
        }
    }
    else if (base<0) {
        if (value<=0) {
            return floor(value/base);
        }
        else {
            return -ceil(-value/base);
        }
    }
    else { // base==0
        if (value<=0) {
            return 0;
        }
        else {
            return INFINITY;
        }
    }
}



#define POW2CEIL( TYPE )\
TYPE pow2ceil_##TYPE( TYPE val ) {\
    TYPE pow = 1;\
    while (pow < val) pow *= 2;\
    return pow;\
}

POW2CEIL(int)
POW2CEIL(long)
POW2CEIL(size_t)
