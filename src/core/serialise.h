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

#ifndef SERIALISE_H
#define SERIALISE_H

#include <stdio.h>

#include "coretype.h"
#include "hashset.h"
#include "vec.h"



#define SOM_T(TYPE,...) som_##TYPE,

typedef enum {
	XX_PRIMITIVES(SOM_T)
	SOM_T(rawptr)
	SOM_T(arr)
	SOM_T(struct)
	SOM_T(proxy)
}
som_t;



typedef struct _som som;

typedef som *(*get_som_func) ();

typedef struct _sub_som {
	size_t off;
	som *chd;
} sub_som;


struct _som {
	som_t  type;
	get_som_func get_som;
	size_t size;
	size_t nchd;
	sub_som *chd;
};


#define GET_SOM_DECL(TYPE, ...) \
	som* get_som_##TYPE();

XX_PRIMITIVES(GET_SOM_DECL)


som *som_arr_new();

som *som_ptr_new();

som *som_struct_new(size_t size, get_som_func get_som);

som *som_proxy_new(get_som_func get_som);

som *som_cons(som *par, size_t off, som *chd);

size_t som_nchd(som *self);

sub_som som_chd(som *self, size_t i);

void serialise(void *obj, som *model, FILE *stream);

#endif