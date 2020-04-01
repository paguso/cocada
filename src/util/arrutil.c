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

#include <stdlib.h>

#include "arrutil.h"
#include "iter.h"


static bool _arr_iter_has_next(void *it)
{
	arr_iter *ait = (arr_iter *)it;
	return ait->index < ait->len;
}

static const void * _arr_iter_next(void *it)
{
	arr_iter *ait = (arr_iter *)it;
	return ait->src + (ait->typesize * ait->index++);
}

static iter_vt _arr_iter_vt = {_arr_iter_has_next, _arr_iter_next};


arr_iter *arr_get_iter(void *arr, size_t len, size_t typesize)
{
	arr_iter *ret = NEW(arr_iter);
	ret->_t_iter.impltor = &ret;
	ret->_t_iter.vt = &_arr_iter_vt;
	ret->src = arr;
	ret->len = len;
	ret->typesize = typesize;
	ret->index = 0;
	return ret;
}

IMPL_TRAIT(arr_iter, iter);