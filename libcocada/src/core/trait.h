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
 * @file trait.h
 * @brief Trait (interface) basic mechanism
 * @author Paulo Fonseca
 *
 * COCADA implements a simplified Trait mechanism. A Trait (interface)
 * consists in a set of methods to be implemented by an object, called
 * the *implementor*. Usually a trait only declares the method, being
 * the responsability of the object to provide an implementation. Thus
 * trait methods are usually **virtual** methods, although the trait
 * may also provide a default implementation.
 *
 * To illustrate, let's suppose we want to define a trait `shape`
 * with two methods
 * - `dimension get_dimension(shape *)`
 * - `void draw(shape *, canvas *)`
 *
 * and suppose that we want to implement types `circle`, and `rectangle`
 * that implement this trait. Our intent is to be able to write something
 * like
 *
 * ```C
 * 1. circle *c = circle_new(100, 100, 50); // x_origin, y_origin, radius
 * 2. rectangle *r = rectangle_new(5, 10, 40, 90) // x_bot_left, y_bot_left, x_top_right, y_top_right
 * 3. dimension dc = get_dimension(c);
 * 4. dimension dr = get_dimension(r);
 * 5. printf("circle: width=%d height=%d\n", dc.width, dc.height); // prints circle: width=100 height=100
 * 6. printf("rectangle: width=%d height=%d\n", dr.width, dr.height); // prints rectangle: width=35 height=80
 * 7. draw(c);
 * 8. draw(r);
 * ```
 * obviously, the dimensions are calculated differently depending on
 * the actual shape, and similarly for the `draw` method to display
 * the shapes on screen.
 *
 * # Defining a Trait
 *
 * A trait `foo` is declared in a file `foo.h` with some "glue" code given
 * in `foo.c`.
 *
 * To illustrate the general scheme, in our current example the file
 * `shape.h` would be something like
 *
 * ```C
 * // FILE shape.h
 * // I. Virtual table
 * typedef struct _shape shape;
 * typedef struct {
 *     dimension (*get_dimension) (shape *);
 *     void (*draw)(shape *, canvas *);
 * } shape_vt;
 *
 * // II. trait type
 * struct _shape {
 *    void *impltor;
 *    shape_vt *vt;
 * };
 *
 * // III. public trait methods
 * dimension get_dimension(shape *);
 * void draw(shape *, canvas *);
 * ```
 *
 * The corresponding `shape.c` would tipically contain generic
 * implementations for the public methods.
 *
 * ```
 * // FILE shape.c
 * dimension get_dimension(shape *s)
 * {
 *    return s->vt->get_dimension(s);
 * }
 *
 * void draw(shape *s, canvas *c)
 * {
 *    s->vt->draw(s, c);
 * }
 *
 * DECL_TRAIT(circle, shape)
 * ```
 *
 * Let's also have a look at the implementation of the `circle` type that
 * implements the `shape` trait, and explain how it all comes together.
 *
 * ```C
 * // FIle circle.c
 *
 * struct _circle {
 *    shape _t_shape;
 *    int x_orig, y_orig, radius;
 * };
 *
 * static dimension circle_get_dimension(shape *s)
 * {
 *    circle *c = (circle *)s->impltor;
 *    dimension dim = {.width=2*c->radius, .height=2*c->radius};
 *    return dim;
 * }
 *
 * static void circle_draw(shape *s, canvas *cv)
 * {
 *    ...draw circle...
 * }
 *
 * static shape_vt circle_vt = {.get_dimension=circle_get_dimension, .draw=circle_draw};
 *
 * circle *circle_new(int x, int y, int r)
 * {
 *    circle *c = NEW(circle);
 *    // init trait
 *    c->_t_shape.impltor = c;
 *    c->_t_shape.vt = &circle_vt;
 *    c->x_orig = x;
 *    c->y_orig = y;
 *    c->radius = r;
 * }
 *
 * IMPL_TRAIT(circle, shape)
 * ```
 *
 * The basic trick to implement a trait is to have an implementor object contain
 * a *virtual table* with the set of functions corresponding to the
 * trait. The first section of the `shape.h` file is the specification of
 * such a virtual table, defined as the `shape_vt` type.  Every implementor type
 * will have to provide a specific implementation for these functions.
 * The `shape` trait type, defined in the second section of this file,
 * represents an object implementing the interface. It contains a pointer
 * to the object `impltor` plus a pointer to the concrete virtual table
 * with the actual functions that should be executed on the object when
 * regarded as a `shape`. This is the general form for any trait.
 * In the third section of the file, we have the declarations for the public
 * versions of the trait methods.
 *
 * In order to implement a trait, the implementor has to include a trait object
 * as one of its members. **This trait has to be called `_t_<name of trait>`**.
 * For example, the `circle` type has a member `_t_shape`. This trait object
 * two components. The `impltor` reference must be set to point to the
 * implementor itself. The virtual table `vt` must point to a virtual table
 * with custom functions to the implementor type. Each implementor type typically
 * has a static virtual table used for all its instances, to avoid duplicating
 * these objects unecessarily. This can be seen in the `circle_new` constructor
 * in the example above.
 *
 * Now, to call a trait method on an implementor object, we need only call
 * the appropriate function of its virtual table. For example, if we have
 * a `circle` object `c`, then
 * ```C
 * dimension dc = c->_t_shape->get_dimension(&c->_t_shape);
 * ```
 * would call its `circle_get_dimension()` method.
 * However, this form is very inconvenient and very far from that of
 * line 3 in the very first example above. To get closer to that form, we
 * could use the public trait function
 * ```C
 * dimension dc = get_dimension(c->_t_shape);
 * ```
 * which is much cleaner and closer to the initially intended form. However,
 * this is still slightly inconvenient for it requires the user of the
 * `circle` type to know implementation details about the type, such as
 * that it has a member `_t_shape`. What we want is to say
 * 'get the dimension of circle `c` seen AS a `shape` TRAIT implementor'.
 * For that, we should used the preferred form
 * ```C
 * dimension dc = get_dimension( AS_TRAIT(c, circle, shape) );
 * ```
 * using the general ::AS_TRAIT macro.
 * Alternatively, we could define a function
 * ```C
 * shape *circle_as_shape(circle *c);
 * ```
 * to return the `shape` representation of a `circle`, and use
 * ```C
 * dimension dc = get_dimension( circle_as_shape(c) );
 * ```
 * This method is declared via the ::DECL_TRAIT macro in `circle.h`
 * and its implementation is generated via the ::IMPL_TRAIT macro in `circle.c`.
 *
 */


/**
 * @brief Declares that a TYPE implements a TRAIT.
 * You should include this at the end of the header file of a
 * type implementing a trait.
 * It declares a method
 * ```C
 * TRAIT *TYPE_as_TRAIT(TYPE *t);
 * ```
 * to get the trait representation of a type.
 *
 * @see IMPL_TRAIT
 * @see Module documentation
 */
#define DECL_TRAIT(TYPE, TRAIT)\
	TRAIT * TYPE##_as_##TRAIT( TYPE *self );


/**
 * Expands into the definition of the method
 * ```C
 * TRAIT *TYPE_as_TRAIT(TYPE *t) {
 *    return t->_t_TRAIT;
 * }
 * ```
 * to get the trait representation of a type.
 * You should include this in the implementation of the type.
 *
 * @see DECL_TRAIT
 * @see Module documentation
 */
#define IMPL_TRAIT(TYPE, TRAIT)\
	TRAIT * TYPE##_as_##TRAIT( TYPE *obj) {\
		return &(obj->_t_##TRAIT);\
	}


/**
 * Gets the trait representation of an implementor object.
 * @param IMPLTOR A pointer to the implementor object
 * @param TYPE The concrete type of the implementor object
 * @param TRAIT The name of the trait
 */
#define AS_TRAIT(IMPLTOR, TYPE, TRAIT) (&(((TYPE *)(IMPLTOR))->_t_##TRAIT))


#endif