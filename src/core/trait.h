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

#ifndef TRAIT_H
#define TRAIT_H


/**

COCADA implements a simplified Trait mechanism. A Trait (interface)
consists in a set of methods to be implemented by an object, called
the *implementor*. Usually a traits only declares the method, being
the responsability of the object to provide an implementation. Thus
trait methods are usually **virtual** methods, although the trait
may also provide a default implementation.

To illustrate, let´s suppose we what to implement a trait `shape`
with two methods
- `dimension get_dim(shape *)`
- `void draw(shape *, canvas *)`
and suppose that we want to implement types `circle`, and `rectangle`
that implement this trait. Our intent is to be able to write something
like

```C
circle *c = circle_new(100, 100, 50); // x_origin, y_origin, radius
rectangle *r = rectangle_new(5, 10, 40, 90) // x_bot_left, y_bot_left, x_top_right, y_top_right
dimension dc = dimension(c);
dimension dr = dimension(r);
printf("circle: width=%d height=%d\n", dc.width, dc.height); // prints circle: width=100 height=100
printf("rectangle: width=%d height=%d\n", dr.width, dr.height); // prints rectangle: width=35 height=80
draw(c);
draw(r);
```

obviously, the dimensions are calculated differently depending on
the actual shape, and similarly for the `draw` method to display
the shapes on screen.


# Defining a Trait

A trait is declared in a file `trait.h` with some "glue" code given
in `trait.c`.

In our current example the file `shape.h` would be something like

```C
typedef struct _shape shape;

typedef struct {
    dimension (*get_dim) (void *);
    void (*draw)(void *, canvas *);
} shape_vt;

struct _shape {
  void *impltor;
  shape_vt vt;
}

dimension shape_get_dim(shape *);
void shape_draw(shape *, canvas *);
```

Every object implementing the


*/



#define DECL_TRAIT(TYPE, TRAIT)\
TRAIT * TYPE##_##TRAIT( TYPE *self );

#define IMPL_TRAIT(TYPE, TRAIT)\
TRAIT * TYPE##_##TRAIT( TYPE *self ) {\
  return &(self->_t_##TRAIT);\
}


#define ASTRAIT(PTR, TYPE, TRAIT) (&(((TYPE *)(PTR))->_t_##TRAIT))


#endif