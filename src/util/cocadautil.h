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
@file cocadautil.h
@author Paulo Fonseca
@brief  Basic definitions, macros and utility functions, including facilities
        for creating and destroying objects.

Object destructor infrastructure.
-------------------------------------------------------------------------------

A cocada container is encapsulates one (or more) heap-allocated buffer of 
contained  "objects". Conceptually, such objects can be directly stored in the 
buffer, or the buffer may contain references (pointers) to (heap) objects.
We call containers of the first type "flat containers", and those of the 
second type "pointer containers". Notice that this is just a conceptual 
distinction since in the latter case we actually have a flat container of 
physical pointers whose values (memory addresses) are stored in the buffer.

Destroying a flat container basically amounts to freeing the memory used by
its buffer plus some constant-sized memory used for housekeeping. For example, 
the buffer of a flat vector with capacity for 100 4-byte integers will occupy 
400 bytes of heap memory. However, by deallocating the buffer of a pointer
container, we are not freeing the referenced objects, which might cause a
to memory leak. The situation can become quite complex if we have deep
hierarchies of nested objects.

Cocada provides some infrastructure for dealing with the proper disposal of 
complex hierarchies of objects. 

The basic concept is that of a **destructor** ::dstr which encapsulates and
provides a way of nesting **destructor functions** ::dstr_func used for 
deallocating nested object hierarchies. A destructor is a  *closure* object 
composed of
- A reference to a *destructor function* ::dstr_func; and
- An array of child destructors.

A *destructor function* is a function used to free (deallocate) the memory 
used by an object. It receives a pointer to the object to be deallocated
and a destructor mirroring its composition. Cocada types typically provide 
- A function for obtaining a basic (flat) destructor named `<typename>_dstr` 
  (example ::vec_dstr); and
- A destructor function named `<typename>_free` (example ::vec_free). 

The implementation of a destructor function of a parent type uses the 
provided destructor to call the destructor functions of the child (referenced)
objects, if any. For example, consider a generic container of type `C`
which happens to be holding references to objects of type `B` which are, 
in their turn, generic containers holding references to objects of type `A`.

```
+-----+      +-----+      +-----+
|  C  |<>----|  B  |<>----|  A  |
+-----+      +-----+      +-----+
```

The destructor function 

```
void C_free( void *ptr, dstr *c_dst )
```

checks the destructor `c_dst` to see if contains a reference to a 
destructor of its child objects, in this case of type `B`. Let us suppose 
this is the case, that is `b_dst = c_dst->chd[0]` is a destructor for 
`B` objects.  Then a call to the corresponding destructor function 

```
b_dst->freer(b, b_dst)
```

will be made for each child `b` of type `B` held by the `C`-type container.
Likewise the `B`-type destructor function, `B_free(void *ptr, dstr *b_dst)`, 
will check whether `b_dst` contains references to a destructor for type 
`A`-objects.

## Raw pointer destructors

If `A` is a simple type with no nested members, then its destructor function 
could consist of a simple call to `void free(void *d)` with no need to check
for nested destructors. For such cases, COCADA provides a basic desctructors 
obtained via ::raw_dstr.

## Composing destructors

In more complex cases an object may contain references to objects of 
multiple types. A classic example is a hashmap, which contains references
to keys of type `K`, and values of type `V`. In such cases, a hashmap
destructor `h_dst` would contain references to `K` and `V` type destructors,
that is `h_dst->nchd==2`, and `h_dst->chd_dsts[0]` and `h_dst->chd_dsts[1]`
would be reference to `K` and `V`-type destructors respectively.

By extrapolating the current example, we could have any tree-like destructor
hierarchy. However the `<typename>_dstr` gives only a basic flat destructor
with no children, and it is impossible to provide a function for each possible
use case beforehand. Although a specific destructor could be implemented by
the library user, for example, a function 

`dstr *C_of_B_of_A_dstr()`

could be implemented to return a destructor to the example above, this would 
be unnecessarily tedious. Instead, COCADA provides a way to compose 
existing destructors via the ::dstr_cons function. This function takes a
pointer to a parent destructor `par`, a child destructor `chd`, appends
`chd` to the list of child destructors of `par`, and returns the reference
to the modified `par`. This can be used to create arbitrary tree-like
hierarchies. For example, 

```
         +--- B
         |
    A ---+         +--- D
         |         |
         +--- C ---+--- E
                   |
                   +--- F

```

could be created with

```
dstr_cons(dstr_cons(A_dstr(), B_dstr()), dstr_cons(dstr_cons(dstr_cons(C_dstr(), D_dstr()), E_dstr()), F_dstr())) 
```

## Empty destructors

In the last example it could be that we do not wish to destroy the `B`-type 
child objects of the root `A`-type object. For instance, they could be 
shared objects whose deletion would cause dangling pointer problems. 
An *empty destructor* obtained via ::empty_dstr can be used in such cases to 
signal that the  corresponding child  objects should not be destroyed.
Notice that this destructor may be given non-empty nested destructors, which
would cause child objects to be destroyed while preserving the parent object.

## Limitations: Cyclic dependencies

No check is performed for cyclic dependencies, which can cause *double free*  
problems.
*/


/**
 * NULL pointer constant.
 */
#if !defined(NULL)
#define NULL ((void *)0)
#endif


/**
 * Allocate memory 
 */
#define NEW( TYPE )   ((TYPE*)(malloc(sizeof(TYPE))))

/**
 * Deallocate memory 
 */
#define FREE( PTR )   free(PTR)


/**
 * The maximum value of a pointer
 */
#define PTR_MAX SIZE_MAX


/**
 * Destructor type
 */
typedef struct _dstr dstr;

/**
 * Destructor function type
 */
typedef void (*dstr_func) (void *, dstr *);

struct _dstr {
    dstr_func freer;
    size_t nchd;
    struct _dstr **chd_dsts;
};


void consume_dstr(dstr *d);


/**
 * @brief Returns the number of nested child destructors of @p dst.
 */
size_t dstr_nchd(dstr *dst); 

/**
 * @brief Returns the child destructor @p par with the given @p index
 */
dstr *dstr_chd(dstr *par, size_t index);

/**
 * @brief Composes two destructor by appending @p chd to the children list of @p par.
 * Returns a reference to the modified @p par
 */
dstr *dstr_cons(dstr *par, dstr *chd);


/**
 * @brief Returns a new empty destructor with no children.
 * @see Module documentation for details.
 */
dstr *empty_dstr();

/**
 * @brief Returns a new raw-pointer destructor with no children.
 * @see Module documentation for details.
 */
dstr *raw_dstr();

#define DSTR_DECL( TYPE ) \
dstr *TYPE##_dstr(); 

#define DSTR_IMPL( TYPE ) \
dstr *TYPE##_dstr() {\
    dstr *dst = NEW(dstr);\
    dst->freer = TYPE##_dispose;\
    dst->nchd = 0;\
    dst->chd_dsts = calloc(1, sizeof(dstr *));\
    return dst;\
}

#define DESTROY( OBJ, DESTRUCTOR ) {\
    void *_obj = (OBJ);\
    dstr *_dst = (DESTRUCTOR);\
    _dst->freer(_obj, _dst);}


#define DESTROY_AND_CONSUME( OBJ, DESTRUCTOR ) {\
    void *_obj = (OBJ);\
    dstr *_dst = (DESTRUCTOR);\
    _dst->freer(_obj, _dst);\
    consume_dstr(_dst);}

#endif