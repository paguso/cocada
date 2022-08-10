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

#ifndef NEW_H
#define NEW_H

#include <stdbool.h>
#include <stdlib.h>

#include "coretype.h"
#include "memdbg.h"

/**
 * @file new.h
 * @brief  Basic definitions, macros and utility functions for creating and
 *        destroying objects
 * @author Paulo Fonseca
 *
 *
 * # Object lifecycle
 *
 * COCADA objects are typically implemented as structs with associated semantics.
 * Each object has a type. In general the type is declared in the header file
 * named `type.h`, and defined (implemented) in the corresponding
 * `type.c`. Most type declarations are made via opaque structs to hide
 * implementation details from the library user.
 *
 * ```
 * In type.h:
 *
 * typedef struct _type type;
 *
 *
 * In type.c:
 *
 * struct _type {
 *     ...
 * };
 *
 * ```
 *
 * The normal lifecycle of memory objetcs is composed of five stages
 *
 * 1. Memory allocation
 * 2. Initialisation
 * 3. Object use
 * 4. Finalisation
 * 5. Memory deallocation
 *
 * where the first two phases (1-2) comprise the object *creation*, and the two
 * last (4-5) comprise the object *destruction*.
 *
 * In COCADA, Step 1 is usually done through the macro ::NEW, which allocate heap
 * memory for an object of a given type and returns a typed pointer to this position.
 * Object creation (steps 1-2) is normally done with a single call to one of the object
 * constructors `type *type_new(...)`. Ocasionally, these two steps can be done
 * separately. The function `type_init(type *obj,...)` can be used to initialise
 * a previously allocatted object `obj`. This is particularly useful for initialising
 * objects living in the stack, like in
 *
 * ```
 * type obj;
 * type_init(&obj);
 * ```
 *
 * Object destruction, on the other hand, can be a lot trickier, because an object may
 * contain or refer to other objects. The crucial question is whether to destroy
 * a '*child* ' object as part of the '*parent* ' object destruction. Two
 * main problems may arise.
 * - If a child object is not destroyed and no furhter references to it exist after
 * the parent object is destroyed, we have a *memory leak*.
 * - If a child object is destroyed and another reference to it exists outside the
 * parent object (shared reference), we have a *dangling reference*. In particular,
 * any subsequent attempt to destroy that same object will lead to '*double free*'
 * problems.
 *
 * So basically, we have to ensure that
 *
 * 1. Every object is eventually destroyed,
 * 2. Every object is destroyed at most once.
 *
 * This is a complex problem in general. Some languages offer automatic memory
 * management systems (garbage collection) whereas others, enforce
 * strict *ownership* rules. Ownership is the principle whereby every object has
 * at  most one owner, and destroying the owner automatically triggers the
 * destruction of its owned components. Shared references integrity is maintained
 * via borrow and lifetime management mechanisms.
 *
 * Although we do not have native language support, in COCADA we struggle to
 * maintain a simpler object dependency structure, trying to make clear
 * <b>in the documentation</b> when the ownership of an object is transferred to
 * another. Likewise, the documentation of the destructors (see below) should
 * indicate which child objects should be destroyed. **As a rule of thumb, we
 * should destroy a referenced object as part of the destruction of its parent
 * if, and only if, it is owned**.
 *
 * # Object composition
 *
 * An object may *contain* or *refer* to other objects. In particular, we
 * distinguish the following patterns.
 *
 * I. Object `A` has a physical (flat) copy of object `B` within its internal
 * memory representation.
 * ```
 * CASE I)
 *
 * struct _A {         +-------------+
 *     ...             |   A         |
 *     B b;            | +---------+ |
 *     ...             | |    B    | |
 * }                   | |         | |
 *                     | +---------+ |
 *                     |             |
 *                     +-------------+
 * ```
 *
 * II. Object `A` has a pointer to an object `B` that lives elsewhere in memory.
 * ```
 * CASE II)
 *
 * struct _A {         +-------------+
 *     ...             |   A         |
 *     B *b;           |             |     ,....> +---------+
 *     ...             |   +----+    |    .       |    B    |
 * }                   |   | b..|........´        |         |
 *                     |   +----+    |            +---------+
 *                     |             |
 *                     +-------------+
 * ```
 *
 * Object *containers* implement generalised versions of those relations.
 * A container is called *flat* when the contained objects are
 * physically stored within the container buffer (array). If the container
 * has a maximum number of elements known at compile time, then it could
 * be implemented as
 *
 * ```code
 * CASE III)
 *
 * #define NMAX = 4;
 * struct _A {         +-----------------------------------+
 *     ...             |   A                               |
 *     size_t size;    |                                   |
 *     B b[NMAX];      | +------+-------+-------+-------+  |
 *     ...             | | b[0] | b[1]  |  b[2] |  b[3] |  |
 * }                   | |      |       |       |       |  |
 *                     | +------+-------+-------+-------+  |
 *                     |                                   |
 *                     +-----------------------------------+
 * ```
 *
 * More frequently, we have arbitrary size containers whose
 * elements live outside, in an array of other dynamic
 * data strucutre, something like
 *
 * ```
 * CASE IV)
 *
 * struct _A {         +------------+
 *     ...             |   A        |
 *     size_t size;    |            |
 *     size_t cap;     | b +----+   |     ,...> +------+-------+-   -+----------+
 *     B *b.           |   |    |   |    .      | b[0] | b[1]  | ... | b[cap-1] |
 * }                   |   | ...........´       |      |       |     |          |
 *                     |   +----+   |           +------+-------+-   -+----------+
 *                     |            |
 *                     +------------+
 * ```
 *
 * In this case, the container is still considered 'flat' since the contained
 * `B`-type objects are physically stored within the buffer, even though the
 * buffer itself lies somewhere outside the container, which physically contains
 * only a pointer to its memory location.
 *
 * Finally, the buffer could contain pointers to objects that are located
 * elsewhere in memory, something like
 *
 * ```
 * CASE V)
 *                                                  ,....> +---------+
 *                                                  .      |         |
 *                                                  .      |         |
 *                                                  .      +---------+
 *                                                  .
 * struct _A {         +------------+               .
 *     ...             |   A        |               .
 *     size_t size;    |            |               .
 *     size_t cap;     | b +----+   |     ,...> +---.--+------+-   -+----------+
 *     B **b.          |   |    |   |    .      | b[0] | b[1] | ... | b[cap-1] |
 * }                   |   | ...........´       +------+--.---+-   -+----------+
 *                     |   +----+   |                     .
 *                     |            |                     .
 *                     +------------+                     .
 *                                                        ` ....> +---------+
 *                                                                |         |
 *                                                                |         |
 *                                                                +---------+
 *
 * ```
 *
 * We will call containers like this *reference containers*.
 * Although a reference container conceptually stores external objects,
 * they are nothing but flat containers of pointers. So there is only one
 * implementation for both cases.
 * In both IV-V cases, the buffer memory is often allocated and initialised
 * during the (container) object initialisation (Step 2), although
 * it can be modified (e.g. resized) later.
 *
 * # Object destruction
 *
 * As mentioned above, destroying an object requires, first, `finalising´ it
 * (Step 4), that is cleaning its internal state and releasing the resources
 * (memory, file handlers, network connexions, etc) held by the object in reverse
 * order of allocation, and then freeing the memory used by the struct (Step 5).
 * Finalising a flat container might include freeing the memory used by
 * its buffer plus some fixed-sized memory. For example,
 * the buffer of a flat vector with capacity for 100 4-byte integers will occupy
 * 400 bytes of heap memory. However, by deallocating the buffer of a reference
 * container, we are not freeing the referenced objects, which might cause a
 * to memory leak. The situation can become quite complex if we have deep
 * hierarchies of nested objects. COCADA provides some infrastructure for
 * dealing with the proper disposal of  complex hierarchies of objects.
 *
 * The basic concept is that of a **finaliser** (::finaliser) which encapsulates and
 * provides a way of nesting **finalise functions** (::finalise_func) used for
 * finalising object hierarchies. A finaliser is a  *closure* object
 * composed of
 * - A reference to a *finalise function* ::finalise_func; and
 * - An array of child finalisers.
 *
 * A *finalise function* is a function used to dispose of the memory used by an
 * object, which would become otherwise unreachable after the object destruction
 * (memory leak). It receives a pointer to the object to be finalised
 * and a finaliser object mirroring its composition.  The finalise function of
 * a `type` is named `type_finalise` (example ::vec_finalise).
 *
 * The implementation of a finalise function of a parent type uses the
 * provided finaliser to call the finalise functions of the child (referenced)
 * objects, if any. For example, consider a generic container of type `C`
 * which contains objects of type `B` which are themselves containers of
 * objects of type `A`.
 *
 * ```
 * +-----+      +-----+      +-----+
 * |  C  |<>----|  B  |<>----|  A  |
 * +-----+      +-----+      +-----+
 * ```
 * For now let us suppose these are flat containers (we´ll come back to that later).
 *
 * The finalisation of `C` would go roughly as follows.  First, the finalise function
 *
 * ```C
 * void C_finalise( void *ptr, finaliser *c_fr )
 * ```
 *
 * checks the finaliser `c_fr` to see if contains a reference to a
 * finaliser for its child objects, in this case of type `B`. Let us suppose
 * this is the case, that is `b_fr = c_fr->chd[0]` is a destructor for
 * `B` objects.  Then a call to the corresponding finalise function
 *
 * ```C
 * b_fr->fn(b, b_fr)
 * ```
 *
 * will be made for each child `b` of type `B` held by the `C`-type container.
 * Likewise the `B`-type finalise function, `B_finalise(void *ptr, dstr *b_fr)`,
 * will check whether `b_fr` contains references to a finaliser for type
 * `A`-objects. After all `A`-objects of a type-`B` container are properly
 * disposed of, then its buffer can be deallocated.  Similarly, after all
 * `B`-objects of a `C` container are properly finalised, then its buffer
 * can be freed.
 *
 * ### IMPORTANT: **Finalise functions SHOULD NOT deallocate the object**
 *
 * A finalise function corresponds to step 3 of the object lifecycle
 * mentioned above. Therefore it should **NOT** attempt to deallocate
 * the memory pointed by the provided object handler, which need not even be a
 * dynamically-allocated heap location. This is why we use the term "finaliser"
 * instead of "destructor".
 *
 *
 * ## Pointer finalisers
 *
 * The situation above is slightly different for reference containers.
 * Let us suppose that the container `C` is a reference  container. Then each
 * element of `C` is actually an owned pointer to a type-`B` container.
 * This indirection of reference containers must be reflected in the
 * finaliser structure. You can think of a reference container, as mentioned
 * above, as container of pointers, each pointer being itself a special case
 * container of just one element (like in CASE II above).
 * Hence, if `C` and `B` were reference containers in our running example
 * we´d have that
 *
 * <b>C</b> has **Pointers** to **B** which has **Pointers** to **A**.
 *
 * So, the `C` finaliser should not have a `B` finaliser as child, but
 * rather a *pointer finaliser*, which then will have a `B` finaliser
 * as child. The difference is subtle but crucial.
 *
 * COCADA provides a function for obtaining a pointer finaliser ::ptr_finaliser.
 * The corresponding finalise function will call the nested finaliser in the
 * pointed object and then free the memory pointed to. If the pointed type is
 * a simple type with no further external references, then we can simply use a
 * pointer finaliser with no nested finalisers.
 *
 *
 * ## Composing finalisers
 *
 * In more complex cases an object may contain references to objects of
 * multiple types. As an example, a (flat) hashmap stores  keys of type `K`,
 * and values of type `V`. In such cases, a hashmap finaliser `h_fr`
 * should have `K` and `V` type finaliser as children, that is
 * `h_fr->nchd==2`, and `h_fr->chd[0]` and `h_fr->chd_fr[1]` should be `K` and
 * `V`-type finalisers respectively.
 *
 * In general, we could have any tree-like finaliser hierarchy. Although a specific
 * finaliser could be implemented by the library user, for example, a function
 *
 * `finaliser *C_of_B_of_A_get_finaliser()`
 *
 * could be implemented to return a finaliser to the previous example, this would
 * be rather tedious. Instead, COCADA provides a more ergonomic way to construct
 * finaliser hierarchies as follows.
 *
 * - The macro ::FNR is used to return a basic finaliser to a given type.
 * That is `FNR(type)` returns a finaliser with finalise function
 * `type_finalise` (which must be provided for each particular type), and
 * whith no nested finalisers (yet);
 *
 * - Existing finalisers can be composed via the ::finaliser_cons function. This
 * function takes a pointer to a parent finaliser `par` and a child finaliser
 * `chd`. It appends `chd` to the list of child finalisers of `par`, and returns the
 * reference to the modified `par`. This can be used to create arbitrary tree-like
 * hierarchies. For example,
 *
 * ```
 *          +--- B
 *          |
 *     A ---+         +--- D
 *          |         |
 *          +--- C ---+--- E
 *                    |
 *                    +--- F
 *
 * ```
 *
 * could be created with
 *
 * ```C
 * finaliser_cons(finaliser_cons(FNR(A), FNR(B)), finaliser_cons(finaliser_cons(finaliser_cons(FNR(C), FNR(D)), FNR(E)), FNR(F)))
 * ```
 *
 * ## Empty finalisers
 *
 * In the last example above, it could be that we did not wish to destroy the `B`-type
 * child objects. For instance, they could be shared objects whose deletion would cause
 * dangling pointer problems.
 * An *empty finaliser* obtained via ::finaliser_new_empty can be used in such cases to
 * signal that the  corresponding child  objects should not be destroyed.
 * In this case, all the objects downstream the empty finaliser point are also
 * left untouched.
 *
 *
 * ## Finalising and destroying objects
 *
 * An object can be *finalised* (Step 4 *only*)  via the ::FINALISE macro.
 * This will not deallocate the object, and is typically what would be used from
 * whithin a flat container destructor to clean memory used by its elements. It is also
 * used to finalise stack objects. The finaliser object is also not destroyed
 * in the process.
 *
 * An object can be completely destroyed (Steps 4-5) with the ::DESTROY macro.
 * In addition to finalising the object, it also deallocates its memory **and**
 * consumes the finaliser. If the default, childless finaliser is to be used,
 * we can simply use `DESTROY_FLAT(obj, type)` which is equivalent, but slightly more
 * convenient than `DESTROY(obj, FNR(type))`.
 *
 * Finally, we may have simpler objects without child objects, or objects whose inner
 * references are fixed and known at compile time. The destructor functions of these
 * objects don't need to look at runtime-built finalisers to know which
 * components to destroy. In such case, although we can still define a finalise function
 * which ignores the `fnr` argument, we might rather define a simple destructor
 *
 * ```
 * void type_free(type *obj)
 * {
 *     // finalise and deallocate obj
 * }
 * ```
 *
 *
 * # Limitations: Backward/Cyclic references
 *
 * No check is performed for backward of self-references, which can cause *double free*
 * problems.
 *
 *
 * # Summary
 *
 * <table>
 * <tr>
 *     <td>Single object operation</td>
 *     <td>Code</td>
 *     <td>Combined object operation</td>
 *     <td>Code</td>
 * </tr>
 * <tr>
 *     <td>1. Allocation </td>
 *     <td>`type *obj = NEW(type)`</td>
 *     <td rowspan=2>1+2. Construction</td>
 *     <td rowspan=2>`type *obj = type_new(...)`</td>
 * </tr>
 * <tr>
 *     <td>2. Initialisation</td>
 *     <td>`type_init(type *obj, ...)`</td>
 * </tr>
 * <tr>
 *     <td>4. Finalisation</td>
 *     <td>`FINALISE(obj, finaliser )`</td>
 *     <td rowspan=3>4+5. Destruction</td>
 *     <td rowspan=3>`DESTROY(obj, finaliser)`<br>
 *     `DESTROY_FLAT(obj, type)`<br>
 *     `type_free(obj)`</td>
 * </tr>
 * <tr>
 *     <td rowspan=2>5. Deallocation</td>
 *     <td rowspan=2>`FREE(obj)`</td>
 * </tr>
 * <tr>
 * </tr>
 * </table>
 *
 */


/**
 * Allocates a new non-initialised object of a given @p TYPE in the heap
 * and returns a pointer to it.
 */
#define NEW( TYPE ) ((TYPE*)(malloc(sizeof(TYPE))))


/**
 * The maximum value of a pointer
 */
#define PTR_MAX SIZE_MAX


/**
 * Finaliser type
 * @see _finaliser
 */
typedef struct _finaliser finaliser;


/**
 * Finaliser function type
 */
typedef void (*finalise_func) (void *ptr, const finaliser *fnr);


/**
 * @brief Creates a new destructor with destructof function.
 */
finaliser *finaliser_new(finalise_func fn);


/**
 * @brief Recursively lones a finaliser
 */
finaliser *finaliser_clone(const finaliser *src);


/**
 * @brief Recursively frees a destructor.
 */
void finaliser_free(finaliser *self);


/**
 * @brief Calls the finaliser function on @p ptr
 */
void finaliser_call(const finaliser *self, void *ptr);


/**
 * @brief Returns the number of nested child destructors of @p dst.
 */
size_t finaliser_nchd(const finaliser *self);


/**
 * @brief Returns the child destructor @p par with the given @p index
 */
const finaliser *finaliser_chd(const finaliser *par, size_t index);


/**
 * @brief Composes two destructor by appending @p chd to the children list of @p par.
 * Returns a reference to the modified @p par
 */
finaliser *finaliser_cons(finaliser *par, const finaliser *chd);


/**
 * @brief Returns a new empty finaliser with no children.
 * @see Module documentation for details.
 */
finaliser *finaliser_new_empty();


/**
 * @brief Returns a new raw-pointer finaliser with no children.
 * @see Module documentation for details.
 */
finaliser *finaliser_new_ptr();


/**
 * @brief Shortcut for creating frequently-used finalisers for object references
 * (pointers). Same as finaliser_cons(finaliser_new_ptr(), chd).
 * @param chd Pointed object finaliser
 */
finaliser *finaliser_new_ptr_to_obj(const finaliser *chd);


/**
 * Returns a default finaliser for a given type with no nested destructor.
 */
#define FNR( TYPE ) finaliser_new(TYPE##_finalise)


/**
 * Returns a default finaliser for a pointer to an object
 * of a given type with no other nested destructor.
 * Same as finaliser_new_ptr_to_obj(FNR(TYPE)).
 */
#define FNR_PTR_TO_OBJ( TYPE ) finaliser_new_ptr_to_obj(FNR(TYPE))


/**
 * Finalises an object @p OBJ (and its referenced objects) based on a given
 * finaliser @p FNR.
 * The object is not deallocated, and neither is the finaliser object destroyed.
 */
#define FINALISE( OBJ, FNR ) \
	{\
		void *__OBJ = (void *)(OBJ);\
		const finaliser *__FNR = (const finaliser *)(FNR);\
		if (__OBJ) 	finaliser_call(__FNR, __OBJ);\
	}


/**
 * Finalises an object @p OBJ with the default plain typed finaliser
 * TYPE_finalise(). Same as FINALISE(OBJ, FNR(TYPE)).
 */
#define FINALISE_FLAT( OBJ, TYPE ) FINALISE(OBJ, FNR(TYPE))


/**
 * Destroys an object @p OBJ, that is finalises it (and its referenced objects)
 * based on a given finaliser @p FNR **and** deallocates its memory.
 * The finaliser @p FNR is **also** destroyed.
 */
#define DESTROY( OBJ, FNR ) \
	{\
		void *__OBJ = (void *)(OBJ);\
		finaliser *__FNR = (finaliser *)(FNR);\
		if ((__OBJ)) {\
			finaliser_call((const finaliser *)(__FNR), __OBJ);\
			free(__OBJ);\
		}\
		finaliser_free((void *)(__FNR));\
	}


/**
 * Destroys an object @p OBJ with the default plain typed finaliser
 * TYPE_finalise(). Same as DESTROY(OBJ, FNR(TYPE)).
 */
#define DESTROY_FLAT( OBJ, TYPE ) DESTROY(OBJ, FNR(TYPE))



/**
 * Deallocates an object @p OBJ after checking that it is non-null
 * by calling `stdlib free()`
 */
#define FREE( OBJ ) if((OBJ)) free((void *)(OBJ))


#endif