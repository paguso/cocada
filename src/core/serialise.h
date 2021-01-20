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

/**
 * @file serialise.h
 * @brief COCADA Serialisation/deserialisation base infrastructure
 * @author Paulo Fonseca
 * 
 * COCADA provides basic infrastructure for the serialisation/deserialisation
 * of complex in-memory interconnected **data** objects, that is, to
 * (partially) write the **state** of an object to a byte stream, and to
 * rebuild a copy of such an object from its serialised form read from
 * a stream.
 * 
 * # Basic concepts
 * 
 * COCADA distinguishes four types of "objects" that can be (de)serialised:
 * 1. Primitive type values: any atomic numeric or character type defined
 * in the coretype.h header
 * 2. Raw pointers: values containing a memory address, that is a value 
 * of the `rawptr` (void *) type. Specialised (typed) pointers can be 
 * serialised/deserialised as raw pointers since they also hold just a 
 * memory location.
 * 3. Arrays:  A contiguous block of bytes in memory (more below)
 * 4. Structs: Any user-defined struct.
 * 
 * The serialise()/deserialise() function writes/reads an object, and
 * other objects directly and indirectly referred to by this first object 
 * to/from a binary stream. In order to do so, these functions require an 
 * object model, named **Serialisable Object Model - SOM** which
 * describes the physical structure of the target object network.
 * 
 * ## The Serialisable Object Model - SOM
 * 
 * A SOM is a runtime-built object that describes the structure of
 * another object, called the **target object**. COCADA provides 
 * functions to build/obtain SOMs for any of the four types of 
 * target objects.
 * 
 * ### Primitive-type SOMs
 * 
 * A SOM for a primitive type object can be obtained by the
 * corresponding `get_som_TYPE()` function. For example, a SOM for
 * an int value can be obtained by `get_som_int()`. These are
 * usually singleton objects because any primitive value of
 * a given type can be described in the same manner. 
 * 
 * ### Pointer SOMs
 * A rawptr SOM can be obtained via a call to som_ptr_new().
 * Serialising a pointer value simply caused the corresponding
 * memory address (the value of the pointer) to be written to
 * disk. Although this is possible, we often are interested in 
 * also saving the pointed memory content. We inform this 
 * situation in the library by composing SOMs with the aid
 * of the som_cons() function.
 * 
 * #### Example
 * Suppose we have a pointer to an integer and
 * we want to serialise not only the pointer but also the
 * int pointee so that this scenario can be reconstructed
 * later. In this case, we'd use
 * 
 * ```C
 * som *foo = som_cons(som_ptr_new(), get_som_int());
 * ```
 * 
 * **Remark:** Notice that the SOM to a pointer is not a 
 * singleton, because we can have pointers to different objects
 * and so they must be uniquely described. 
 * 
 * 
 * ### Array SOMs
 * 
 * An array SOM can be built with the som_arr_new() function.
 * This SOM is used to describe an array, i.e., a contiguous
 * memory chunk containing some data. However, we have here
 * an important limitation. Because the underlying mechanism
 * has to know the physical size of the array, **this SOM can
 * only be applied to arrays created with the arr_new functions
 * defined in arrays.h**, which are created in such a way that
 * the physical array size is stored immediately before the 
 * useful part of the array itself. 
 * 
 * Any array SOM must also contain a unique nested sub-SOM
 * which describes the type of the elements of the array. 
 * Notice also that the array SOM describes the array object
 * that is, the region of the memory where the data is stored.
 * A heap-allocated array, such as those created by the
 * arr_new() functions of arrays.h is usually represented 
 * by a pointer which points to the start of the array. 
 * Thus this situation must be modelled as a pointer SOM
 * containing an array SOM.
 * 
 * #### Example
 * Suppose we want to (de)serialise an int array. In this case
 * we need
 * 
 * ```C
 * som *foo = som_cons(som_ptr_new(), som_cons(som_arr_new(), 0, get_som_int()));
 * ```
 *
 * ### Struct SOM
 * 
 * Structs are used to define the state of complex heterogenous
 * objects. A struct can contain fields of several types, each
 * described by a unique name. Apart from primitive, atomis types,
 * a field can itselt be of an structured type. A struct field can
 * also be a pointer to an external object, in which case we say
 * that it is a '*reference*'. In any case, the struct must have
 * a fixed size which can be returned by the sizeof operator. 
 * 
 * 
 * ### Self-referential (recursive) objects
 * 
 * ### Strings
 * 
 * 
 * 
 */


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

/**
 * @brief Returns a SOM to a pointer value.
 */
som *som_ptr_new();

som *som_struct_new(size_t size, get_som_func get_som);

som *som_proxy_new(get_som_func get_som);

/**
 * @brief Composes two SOMs
 */
som *som_cons(som *par, size_t off, som *chd);

size_t som_nchd(som *self);

sub_som som_chd(som *self, size_t i);

void serialise(void *obj, som *model, FILE *stream);

void *deserialise(som *model, FILE *stream);

#endif