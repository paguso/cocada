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

/**
@file new.h
@author Paulo Fonseca
@brief  Basic definitions, macros and utility functions for creating and 
        destroying objects.


Object lifecycle
-------------------------------------------------------------------------------

Cocada objects are typically implemented as structs with associated semantics.
Each object has a type. In general the type is declared in the header file
named `type.h`, and defined (implemented) in the corresponding 
`type.c`. Most type declarations are made via anonymous structs to hide
implementation details from the library user.

```
In type.h:

typedef struct _type type;

-----------------------------------

In type.c:

struct _type {
    ...
};

```

The lifecycle of memory objetcs composed of five stages
1. Memory allocation
2. Initialisation
3. Normal use
4. Finalisation
5. Memory deallocation
where the first two phases (1-2) comprise the object *creation*, and the two 
last (4-5) comprise the object *destruction*.

In COCADA, Step 1 usually consists in a call to `malloc`, which is
normally done through the macro ::NEW
which allocates heap memory for an object of a given type and returns a typed
pointer to this position. More commonly though, object creation (steps 1-2) 
is done with a single call to one of the object constructors 
`type *type_new(...)`

An object can conceptually contain (or refer to) another object. We distinguish
two kinds of object composition with regards to how it is physically realised.

I. Object `A` has a physical (flat) copy of object `B` within its internal
memory representation.
```
CASE I)

struct _A {         +-------------+          
    ...             |   A         |
    B b;            | +---------+ |
    ...             | |    B    | |
}                   | |         | |
                    | +---------+ |
                    |             |
                    +-------------+
```

II. Object `A` has a pointer to an object `B` that lives elsewhere in memory.
```
CASE II)

struct _A {         +-------------+                                   
    ...             |   A         |                        
    B *b;           |             |     ,....> +---------+ 
    ...             |   +----+    |    .       |    B    | 
}                   |   | b..|........´        |         | 
                    |   +----+    |            +---------+ 
                    |             |                        
                    +-------------+                          
```

In the first case, the memory for holding the internal `B` instance is 
allocated as part of the memory for holding `A`. In the second case, the
memory for `B` must be allocated during object initialisation (Step 2).

Object *containers* implement generalised versions of those relations.
A container is called *flat* when the contained objects are 
physically stored within the container buffer (array). If the container
has a maximum number of elements known at compile time, then it could
be implemented as

```code
CASE III)

#define NMAX = 4;
struct _A {         +-----------------------------------+          
    ...             |   A                               |
    size_t size;    |                                   |
    B b[NMAX];      | +------+-------+-------+-------+  |
    ...             | | b[0] | b[1]  |  b[2] |  b[3] |  |
}                   | |      |       |       |       |  |
                    | +------+-------+-------+-------+  |
                    |                                   |
                    +-----------------------------------+
```

More typically, though, we have arbitrary size containers whose
elements live outside the container, in an array of other dynamic
data strucutre, something like

```
CASE IV)

struct _A {         +------------+          
    ...             |   A        |
    size_t size;    |            |
    size_t cap;     | b +---+    |     ,...> +------+-------+-  - +----------+  
    B *b.           |   |    |   |    .      | b[0] | b[1]  | ... | b[cap-1] |  
}                   |   | ...........´       |      |       |     |          |  
                    |   +----+   |           +------+-------+-   -+----------+  
                    |            |
                    +------------+
```

In this case, the container is still considered `flat´ since the contained
`B`-type objects are physically stored within the buffer, even though the
buffer itself lies somewhere outside the container, which physically contains
only a pointer to its memory location.

Finally, the buffer could contain pointers to objects that are located 
elsewhere in memory, something like

```
CASE V)
                                                 ,....> +---------+
                                                 .      |         |
                                                 .      |         |
                                                 .      +---------+
                                                 .
struct _A {         +------------+               .         
    ...             |   A        |               .
    size_t size;    |            |               .
    size_t cap;     | b +----+   |     ,...> +---.--+------+-   -+----------+  
    B **b.          |   |    |   |    .      | b[0] | b[1] | ... | b[cap-1] |  
}                   |   | ...........´       +------+--.---+-   -+----------+   
                    |   +----+   |                     .
                    |            |                     .
                    +------------+                     .
                                                       ` ....> +---------+
                                                               |         |
                                                               |         |
                                                               +---------+

```                 

We will call containers like this *reference containers*.
Although a reference container conceptually stores external objects, 
they are nothing but flat containers of pointers. So there is only one
implementation for both cases.  
In whichever case, the buffer memory is normally allocated and initialised
during the (container) object initialisation (Step 2).
This is important for understanding the discussion on object destruction below.


Object destructor infrastructure
-------------------------------------------------------------------------------

As mentioned above, destroying an object requires, first, `finalising´ it 
(Step 4), that is cleaning its internal state and releasing the resources 
(in particular, memory) held by the object in reverse order of allocation, 
and then freeing the memory used by the struct (Step 5).
In particular, finalising a flat container amounts to freeing the memory used by
its buffer plus some constant-sized housekeeping memory. For example, 
the buffer of a flat vector with capacity for 100 4-byte integers will occupy 
400 bytes of heap memory. However, by deallocating the buffer of a reference
container, we are not freeing the referenced objects, which might cause a
to memory leak. The situation can become quite complex if we have deep
hierarchies of nested objects. COCADA provides some infrastructure for 
dealing with the proper disposal of  complex hierarchies of objects. 

The basic concept is that of a **destructor** ::dtor which encapsulates and
provides a way of nesting **destructor functions** ::dstr_func used for 
finalising object hierarchies. A destructor is a  *closure* object 
composed of
- A reference to a *destructor function* ::dstr_func; and
- An array of child destructors.

A *destructor function* is a function used to dispose of the  memory used by an 
object, which would become otherwise unreachable after the object destruction
(memory leak). It receives a pointer to the object to be finalised
and a destructor object mirroring its composition.  The destructor function of 
a `type` is named `type_dispose` (example ::vec_dispose). 

The implementation of a destructor function of a parent type uses the 
provided destructor to call the destructor functions of the child (referenced)
objects, if any. For example, consider a generic container of type `C`
which contains objects of type `B` which are themselves containers of
objects of type `A`. 

```
+-----+      +-----+      +-----+
|  C  |<>----|  B  |<>----|  A  |
+-----+      +-----+      +-----+
```
For now let us suppose these are flat containers. We´ll come back to that later.

The destruction of C would go roughly as follows.  First, the destructor function 

```
void C_dispose( void *ptr, dstr *c_dt )
```

checks the destructor `c_dt` to see if contains a reference to a 
destructor for its child objects, in this case of type `B`. Let us suppose 
this is the case, that is `b_dt = c_dt->chd[0]` is a destructor for 
`B` objects.  Then a call to the corresponding destructor function 

```
b_dt->df(b, b_dt)
```

will be made for each child `b` of type `B` held by the `C`-type container.
Likewise the `B`-type destructor function, `B_dispose(void *ptr, dstr *b_dt)`, 
will check whether `b_dt` contains references to a destructor for type 
`A`-objects. After all `A`-objects of a type-`B` container are properly
disposed of, then its buffer can be deallocated.  Similarlty, after all 
`B`-objects of a C` container are properly destroyed, then its buffer 
can be freed.  


## Pointer destructors

The situation above is slightly different for reference containers. 
Let us suppose that the container `C` is a reference  container. Then each 
element of `C` is actually a pointer to a `B`-type container. 
After finalising one such child containers, we also typically want to have 
the its corresponding object memory deallocated.
If `C` is a flat container as before, then its type-`B` container 
elements structs are directly stored within its buffer. Thus we want
to finalise one such type-`B` container (to delete its `A`-elements
and release their buffers), but we cannot call a ::free on them individually.
All the type-`B` structs memory will be released at once when the buffer
of the `C` container is released.

This indirection of reference containers must be reflected in the
destructor structure. You can think of a reference container, as mentioned
above, as container of pointers, each pointer being itself a special case 
container of just one element (like in CASE II above). 
Hence, if `C` and `B` were reference containers in our running example 
we´d have that

 <b>C</b> has **Pointers** to **B** which has **Pointers** to **A**.

So, the `C` destructor needs to have not a `B` destructor as child, but
rather a *pointer destructor*, which then will have a `B` destructor
as child. The difference is subtle but crucial. 

COCADA provides a function for obtaining a pointer destructor ::ptr_dtor. 
The corresponding destructor function will call the nested destructor in the 
pointed object and then free the memory pointed to. If the pointed type is 
a simple type with no further external references, then we can simply use a  
pointer destructor whith no nested destructors.


## Composing destructors

In more complex cases an object may contain references to objects of 
multiple types. As an example, a (flat) hashmap stores  keys of type `K`, 
and values of type `V`. In such cases, a hashmap destructor `h_dt` 
should have `K` and `V` type destructors as children, that is 
`h_dt->nchd==2`, and `h_dt->chd[0]` and `h_dt->chd_dt[1]` should be `K` and 
`V`-type destructors respectively.

In general, we could have any tree-like destructor hierarchy. Although a specific 
destructor could be implemented by the library user, for example, a function 

`dstr *C_of_B_of_A_dstr()`

could be implemented to return a destructor to the previous example, this would 
be rather tedious. Instead, COCADA provides a more ergonomic way to construct 
destructor hierarchies as follows. 

- The macro ::DTOR is used to return a basic destructor to a given type.
That is `DTOR(type)` returns a destructor with destructor function
`type_dispose` (which must be provided for each particular type), and
whith no nested child destructors (yet);

- Existing destructors can be composed via the ::dtor_cons function. This 
function takes a pointer to a parent destructor `par`, a child destructor 
`chd`, appends `chd` to the list of child destructors of `par`, and returns the 
reference to the modified `par`. This can be used to create arbitrary tree-like
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
dtor_cons(dtor_cons(DTOR(A), DTOR(B)), dtor_cons(dtor_cons(dtor_cons(DTOR(C), DTOR(D)), DTOR(E)), DTOR(F))) 
```

## Empty destructors

In the last example it could be that we do not wish to destroy the `B`-type 
child objects of the root `A`-type object. For instance, they could be 
shared objects whose deletion would cause dangling pointer problems. 
An *empty destructor* obtained via ::empty_dtor can be used in such cases to 
signal that the  corresponding child  objects should not be destroyed.
Notice that any object down the empty destructor point will be left untouched.

## Finalising and destroying objects 

An object can be *finalised* (Step 4 *only*) with a destructor via the ::FINALISE macro. 
This will not deallocate the object. This is typically what would be used from
whithin a flat container destructor to clean memory used by its elements. 
The destructor object is also not destroyed.

An object can be completely destroyed (Steps 4-5) with the ::DESTROY macro.
In addition to finalising the object, it also deallocates its memory **and**
also consumes the destructor.

Finally, if

## Limitations: Backward/Cyclic references

No check is performed for backward references, which can cause *double free*  
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
 * The maximum value of a pointer
 */
#define PTR_MAX SIZE_MAX


/**
 * Destructor type
 */
typedef struct _dtor dtor;


/**
 * Destructor function type
 */
typedef void (*dstr_func) (void *, const dtor *);


struct _dtor {
    dstr_func df;
    size_t nchd;
    struct _dtor **chd;
};

/**
 * @brief Creates a new destructor with destructof function.
 */
dtor *dtor_new_with_func(dstr_func df); 

/**
 * @brief Recursively frees a destructor.
 */
void dtor_free(dtor *dt);

/**
 * @brief Returns the number of nested child destructors of @p dst.
 */
size_t dtor_nchd(const dtor *dt); 

/**
 * @brief Returns the child destructor @p par with the given @p index
 */
const dtor *dtor_chd(const dtor *par, size_t index);

/**
 * @brief Composes two destructor by appending @p chd to the children list of @p par.
 * Returns a reference to the modified @p par
 */
dtor *dtor_cons(dtor *par, const dtor *chd);


/**
 * @brief Returns a new empty destructor with no children.
 * @see Module documentation for details.
 */
dtor *empty_dtor();

/**
 * @brief Returns a new raw-pointer destructor with no children.
 * @see Module documentation for details.
 */
dtor *ptr_dtor();

/**
 * Returns a default destructor for a given type with no nested destructor.
 */
#define DTOR( TYPE ) dtor_new_with_func(TYPE##_dispose)

/**
 * Finalises an object (and its referenced objects) based on a given
 * destructor. 
 * The object is not deallocated, and the destructor is not destroyed.
 */
#define FINALISE( OBJ, DTOR ) {\
    void *__obj = (OBJ);\
    dtor *__dt = (DTOR);\
    __dt->df(__obj, __dt);}

/**
 * Destroys an object, that is finalises it (and its referenced objects) 
 * based on a given destructor **and** deallocates its memory.
 * The destructor is **also** destroyed.
 */
#define DESTROY( OBJ, DTOR ) \
    void *__obj = (OBJ);\
    dtor *__dt = (DTOR);\
    __dt->df(__obj, __dt);\
    free(__obj);\
    dtor_free(__dt);


#define FREE1( OBJ ) free(OBJ)

#define FREE2( OBJ, TYPE ) DESTROY(OBJ, DTOR(TYPE))

#define _SELECT_FREE(_1, _2, NAME,...) NAME

#define FREE(...) _SELECT_FREE(__VA_ARGS__, FREE2, FREE1)(__VA_ARGS__)


#endif