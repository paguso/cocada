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
 * @brief COCADA Serialisation/deserialisation
 * @author Paulo Fonseca
 *
 * COCADA provides basic infrastructure for the serialisation/deserialisation
 * of complex in-memory interconnected **data** objects. That means
 * writing the **state** of an object to a byte stream, and to
 * rebuild a copy of such an object from a stream.
 *
 * # Basic concepts
 *
 * COCADA distinguishes five types of "objects" that can be (de)serialised:
 * 1. Primitive type values: any atomic numeric or character type defined
 * in the coretype.h header
 * 2. Raw pointers: values containing a memory address, that is a value
 * of the `rawptr` type. Specialised (typed) pointers can be
 * serialised/deserialised as raw pointers since they also hold just a
 * memory address.
 * 3. Arrays:  A contiguous block of values.
 * 4. Strings:  A null-terminated '\0' char string.
 * 5. Structs: Any user-defined struct.
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
 * functions to build/obtain SOMs for any of the five types of
 * target objects.
 *
 * ### Primitive-type SOMs
 *
 * A SOM for a primitive type object can be obtained by the
 * corresponding `get_som_TYPE()` function. For example, a SOM for
 * an int value can be obtained by `get_som_int()`.
 * **Primitive type SOMs are singleton objects** since any primitive value
 * of a given type can be described in the same manner.
 *
 * ### Pointer SOMs
 *
 * A rawptr SOM can be obtained via a call to som_ptr_new().
 * Serialising a pointer value simply causes the corresponding
 * memory address (the value of the pointer) to be written to
 * the stream. However, we often are also interested in
 * saving the pointed memory contents. We signal this
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
 * som *foo = som_cons(som_ptr_new(), 0, get_som_int());
 * ```
 *
 * **Remark:** Notice that the pointer SOM is not a
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
 * only be applied to arrays created with the sa_arr_calloc functions
 * defined in arrays.h**, which are created in such a way that
 * the physical array size is stored immediately before the
 * useful part of the array itself.
 *
 * Any array SOM must also contain a unique nested sub-SOM
 * which describes the type of the elements of the array.
 * Notice also that the array SOM describes the array object
 * that is, the region of the memory where the data is stored.
 * A heap-allocated array, such as those created by the
 * sa_arr_calloc() functions of arrays.h is usually represented
 * by a pointer which points to the start of the array.
 * Thus this situation must be modelled as a pointer SOM
 * containing an array SOM.
 *
 * #### Example
 * Suppose we want to (de)serialise an int array. In this case
 * we need
 *
 * ```C
 * som *foo = som_cons(som_ptr_new(), 0, som_cons(som_arr_new(), 0, get_som_int()));
 * ```
 *
 * ### String SOMs
 *
 * String SOMs are treated very similarly to array SOMs since
 * a string is a special case of an array. The difference is
 * that, with a string SOM, we do not indicate the type of
 * the element because it is naturally supposed to be `char`.
 * Also, because strings are null-terminated ('\0'), their
 * size can be determined with the `strlen` function, and so
 * we can use them without having to have their size known
 * at a specific location, as with other type of arrays.
 * Hence a string SOM has no distinctive characteristic apart from its
 * type, which is why, like the primitive type SOMs, we need only a
 * singleton string SOM returned by the `get_som_cstr()` function.
 *
 *
 * ### Struct SOMs
 *
 * Structs are used to define the state of complex heterogenous
 * objects. A struct can contain fields of several types, each
 * described by a unique name. Apart from primitive, atomic types,
 * a field can itself be of an structured type. A struct field can
 * also be a pointer to an external object, in which case we say
 * that it is a '*reference*'. In any case, the struct must have
 * a fixed size which can be returned by the sizeof operator.
 *
 * For each serialisable struct, a get_som_func function must be defined
 * that returns a **singleton** SOM describing the structure.
 *
 * ### Example
 *
 * ```C
 * // (1) Struct type declaration
 * typedef struct {
 * 		float x;
 * 		float y;
 * 		float radius;
 * } circle;
 *
 * // (2) Struct SOM singleton. Should not be accessed directly
 * static som *circle_som = NULL;
 *
 * // (3) Returns the singleton struct SOM
 * som *get_circle_som() {
 * 		if (circle_som == NULL) {
 * 			circle_som = som_struct_new(sizeof(circle), get_circle_som);
 * 			circle_som = som_cons(circle_som, STR_OFFSET(circle, x), get_float_som());
 * 			circle_som = som_cons(circle_som, STR_OFFSET(circle, y), get_float_som());
 * 			circle_som = som_cons(circle_som, STR_OFFSET(circle, radius), get_float_som());
 * 		}
 * 		return circle_som;
 * }
 *
 * ```
 *
 * The example above illustrates the four components required for
 * providing a SOM for a struct object.
 *
 * 1. The struct type declaration itself
 * 2. The STR_SOM_INFO is a required preparation for being able to use the STR_OFFSET macro
 * for composing the struct SOM with its fields' SOMs.
 * 3. The struct SOM singleton object. This should not be accessed directly but rather via
 * the get_som_func function defined for this struct on step 4
 * 4. The definition of the get_som_func that returns the struct SOM singleton
 * after initialisation, if needed.
 *
 * Notice that when composing the struct SOM with the SOMs of its fields via calls
 * to som_cons, we have to inform the relative offset of the field's memory location
 * w.r.t. the struct's start location. This information is required in particular for
 * deserialisation, and can be obtained via the STR_OFFSET macro.
 *
 *
 * ### Self-referential (recursive) objects
 *
 * Some objects may include references to objects of the same type. For example, a typical
 * linked list node object would be defined as
 *
 * ```C
 * typedef struct _node {
 * 		int val;
 * 		struct _node *next; // recursive reference
 * }
 * ```
 *
 * Notice that the `next` pointer is a recursive reference to another `node` object.
 * Now, consider how a SOM for one such an object could be (wrongly) defined.
 *
 * ``` C
 * STR_SOM_INFO(node)
 *
 * som *node_som = NULL;
 *
 * // wrongly defined
 * som *get_node_som() {
 * 		if (node_som == NULL) {
 * 			node_som = som_struct_new(sizeof(node));
 * 			node_som = som_cons(node_som, STR_OFFSET(node, val), get_int_som());
 * 			node_som = som_cons(node_som, STR_OFFSET(node, next),
 * 							som_cons(som_ptr_new(), 0, get_node_som())); // INFINITE LOOP!
 * 		}
 * 		return node_som;
 * }
 * ```
 *
 * The problem arises in the line indicated by the "INFINITE LOOP" comment.
 * Because the node has a pointer to another node, the corresponding SOM
 * should somehow include a pointer SOM composed with a node SOM.
 * However, if we simply include a call to get_node_som, as shown, this would
 * result in an infinite loop. Likewise, we cannot include a new node SOM at
 * this point because then this new node SOM would also need a reference to
 * another node SOM, and so forth.
 *
 * To cope with situations like that, COCADA provides a **proxy SOM**
 * which contains a reference to get_som_func function such that a
 * reference to a SOM could be obtained 'on demand', eliminating the loop
 * in the SOM definition. The corrected version of the function is shown below
 *
 * ```C
 * som *get_node_som() {
 * 		if (node_som == NULL) {
 * 			node_som = som_struct_new(sizeof(node));
 * 			node_som = som_cons(node_som, STR_OFFSET(node, val), get_int_som());
 * 			node_som = som_cons(node_som, STR_OFFSET(node, next),
 * 							som_cons(ptr_som_new(), 0, proxy_som_new(get_node_som)));
 * 		}
 * 		return node_som;
 * }
 * ```
 *
 * Notice that the recursive reference to the get_node_som method has been
 * encapsulated inside a proxy_som object which prevents it from being
 * immediately called.
 *
 * The bottom line is that **a struct SOM should not include direct references
 * to other struct SOMs**. Instead, they should normally be inserted as
 * proxy SOMs with references to the corresponding get_som_func functions
 * that return singleton struct SOM objects.
 *
 *
 * # LIMITATIONS
 *
 * The correct deserialisation of an object depends on the type sizes and
 * encoding (in particular endianess) being the same or, at least, compatible.
 * Therefore the serialisation/deserialisation is not portable and
 * is actually a *very unsafe memory operation*.
 *
 * <b>Be warned and use it at your own risk!</b>
 *
 * However, by default COCADA writes the object sizes and checks whether
 * they are compatible during deserialisation and issues a **warning**
 * if they are not. Notice that this requires the application to
 * have been compiled with `DEBUG_LVL >= 2` (see errlog.h).
 *
 * Other noteworthy limitations are the following.
 *
 * 1. We cannot have references (pointers) to internal locations within other
 * objects, including structs or arrays. That is, if we serialise a pointer
 * to a memory location inside a struct (e.g. a struct field), or a position
 * inside an array, the result is unpredictable. Chances are that the memory
 * will be duplicated on (de)serialisation but other more serious memory
 * corruption is possible. So *be careful to have pointers only to
 * entire objects*.
 *
 * 2. On the same token, *be careful with uinitialised pointers*, and *always*
 * have a serialisable reference set to a valid object or NULL.
 * COCADA will try to follow the pointer and serialise
 * the pointed memory contents if the SOM tells it to. It has no way
 * of knowing if the pointer is valid or not. Particular care has to be
 * taken with pointer arrays. Make sure to have the unused elements set
 * to NULL.
 *
 * 3. Although it is possible to serialise objects living in the stack,
 * all deserialised objects will be heap allocated.
 *
 */

typedef struct _sub_som sub_som;

/**
 * SOM object type
 */
typedef struct _som som;


/**
 * @brief get_som function type, required for the proxy SOM.
 */
typedef som *(*get_som_func) ();



#define GET_SOM_DECL(TYPE, ...) \
	som* get_som_##TYPE();

XX_PRIMITIVES(GET_SOM_DECL)
GET_SOM_DECL(cstr)



#define STR_OFFSET(STRUCT, FIELD)	((size_t)(&(((STRUCT *)NULL)->FIELD)))


/**
 * @brief Returns a new SOM to an array.
 * @warning This SOM can only be used with arrays created via the
 * constructors defined in arrays.h
 * @see arrays.h
 */
som *som_arr_new();

/**
 * @brief Returns a new SOM to a pointer value.
 */
som *som_ptr_new();


/**
 * @brief Returns a new SOM to a struct value.
 * @warning Only one such struct SOM should normally be
 * created for each struct type, and this singleton should be
 * returned by the corresponding @p get_som function.
 * @see Module documentation
 */
som *som_struct_new(size_t size, get_som_func get_som);


/**
 * @brief Returns a new proxy SOM.
 * A proxy SOM contains only a pointer to a function
 * returning a SOM. They are necessary to prevent endless
 * recursion when definining self-referential SOMs.
 * @see Module documentation.
 */
som *som_proxy_new(get_som_func get_som);


/**
 * @brief Composes the SOM or a parent object with the SOM of
 * one of its members.
 * @par par The parent SOM
 * @par offset The memory offset of the child member, relative
 * to the start position of the parent object.
 * @par chd The child SOM, i.e. the model of the object member.
 * @warning If the parent is but a struct SOM, the @p offset is ignored.
 */
som *som_cons(som *par, size_t offset, som *chd);


/**
 * @brief Returns the number of 'child' sub-SOMs
 */
size_t som_nchd(som *self);


/**
 * @brief Returns a 'child' sub-SOM by its index.
 */
sub_som som_chd(som *self, size_t i);


/**
 * @brief Writes a serialised form of an object @p obj and its
 * directly and indirectly referred objects, according to a SOM
 * @p model, to a binary @p stream.
 *
 * @see deserialise
 */
void serialise(void *obj, som *model, FILE *stream);

/**
 * @brief Reads the serialised form of an object @p obj and its
 * directly and indirectly referred objects, according to a SOM
 * @p model, from a binary @p stream.
 *
 * @returns A pointer to a heap allocated copy of the
 * deserialised object (and the objects referred by it).
 *
 * @see serialise
 */
void *deserialise(som *model, FILE *stream);

#endif
