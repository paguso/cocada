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

#ifndef AVL_H
#define AVL_H

#include <stdio.h>

#include "coretype.h"
#include "iter.h"
#include "new.h"
#include "order.h"


/**
 * @file    avl.h
 * @brief   AVL tree
 * @author  Paulo Fonseca
 *
 * The AVL tree is a height-balanced search tree with O(lg n) worst-case
 * search, insertion and deletion.
 *
 * The basic of interface of this AVL tree is composed of four
 * basic untyped methods:
 *
 * 1. avl_new() creates a new AVL tree for indirect storage of objects of
 * given size
 * 2. avl_finalise() object destructor (see new.h)
 * 2. avl_get() searches for a node matching a given key
 * 3. avl_ins() inserts an **owned copy** of a given object
 * 4. avl_del() removes the **owned copy** of an object
 *
 * Thus, the plain version of this AVL tree stores pointers to external
 * objects. The following diagram illustrates this situation.
 *
 * ```
 *             AVL tree
 * +--------------------------------------------+
 * |           .                                |
 * |            .                               |
 * |             .                              |         External object
 * |              \                             |         type obj_t
 * |               \  AVL node                  |       +-----------------+
 * |          +--------------+                  |       |   int key       |
 * |          | balance = 0  |          .-------------> |   ...           |
 * |          |              |          |       |       |                 |
 * |          | val --------------------'       |       +-----------------+
 * |          |              |                  |
 * |          | left  right  |                  |
 * |          +--/-------\---+                  |
 * |            /         \                     |
 * |           .           .                    |
 * |          .             .                   |
 * |         .               .                  |
 * +--------------------------------------------+
 *
 * ```
 * Here the AVL nodes stores pointers to external objects of size `S` bytes.
 * The tree interface assumes that the references are **NOT** owned.
 * Therefore `avl_del()` will delete the tree node but it will not try to
 * free up the data pointed at by the pointer stored in the node.
 *
 * In case the user wants to treat external references as owned, she or he
 * should care to provide the appropriate destructor when freeing the tree,
 * for instance
 * ```C
 * DESTROY(tree, finaliser_cons(FNR(avl), ptr_finaliser()));
 * ```
 * if `obj_t` is a flat type with no owned references to external
 * objects; or
 * ```C
 * DESTROY(tree, finaliser_cons(FNR(avl), finaliser_cons(FNR(obj_t), ...));
 * ```
 * if nested destruction is necessary (see new.h).
 *
 * In addition to this plain generic interface, the tree could store any
 * of the primitive (and pointer) types defined in the coretype.h header.
 * The corresponding access/insertion/deletion methods have the one-word type
 * names as suffixes, e.g. `avl_get_int(avl *self, int val)`, or
 * `avl_ins_ullong(avl *self, ullong val)`.
 *
 * Any value inserted via the corresponding typed  method
 * is directly stored in the node. The same applies for
 * the pointer types `rawptr` and `cstr`, that is, the value of the
 * pointer is  directly stored in the node. However this is normally
 * regarded as indirect storage because the data pointed at by these
 * pointers are stored outside the tree. Actually the `_rawptr` methods
 * perform exactly the same as the plain methods.
 *
 * The AVL tree stores elements of a totally ordered set. As such
 * it requires a pointer to a total-order comparison function of
 * type `cmp_func` (see order.h), to be given on construction. For the primitive
 * types with the usual semantics, the default typed comparison
 * functions defined in the order.h (e.g. `cmp_int`, `cmp_double`,...)
 * can be passed to the object constructor. For indirectly stored
 * data, a custom comparator should be provided. Keep in mind, though,
 * that the comparison function receives pointers to whatever is
 * stored in the node. In the case of indirect storage, the pointer
 * is what is going to be stored in the node.
 * So, in the example of the diagram above, in which the tree node
 * value (`val`) is a pointer to an externally stored object of type `obj_t`,
 * let's suppose that `obj_t` has an integer key field which should be
 * used for  comparison. In this case the comparison fuction could
 * be something like
 * ```C
 * int cmp_obj_t (const void *l, const void *r) {
 *      obj_t *lo = *((obj_t **)l);
 *      obj_t *ro = *((obj_t **)r);
 *      if (lo->key == ro->key) return 0;
 *      if (lo->key < ro->key) return -1;
 *      else return +1;
 * }
 * ```
 * Notice that `l` and `r` are pointers to pointers to `obj_t`.
 *
 *
 * The primitive data type values inserted in the tree are naturally
 * owned by it because copies of such values are directly stored
 * in the nodes. In this case, as well as for non-owned indirect data,
 * a simple call to
 * ```C
 * DESTROY_FLAT(tree, avl);
 * ```
 * should suffice for tree disposal.
 *
 */


/**
 * Opaque AVL type
 */
typedef struct _avl avl;


/**
 * @brief Constructor.
 * @param cmp Key comparison function
 * @see order.h
 * @warning The comparison function receives pointers to whatever
 * is stored in the tree nodes. If the nodes store pointers to
 * external objects, which is the case for all but the primitive
 * types, then @p cmp receives pointers to pointers as arguments.
 * See the header file documentation for an example.
 */
avl *avl_new(cmp_func cmp);


/**
 * @brief Destructor
 * @see new.h
 */
void avl_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Searches for a node matching a given @p key
 * (according to the AVL comparison function) , if any, and
 * copies of the reference stored therein into @p dest.
 * @returns A boolean indicating whether the search was
 * successful.
 */
bool avl_get(avl *self, void *key, void **dest);


/**
 * @brief Inserts a node containing the reference @p val,
 * if it doesn't already contain a node matching this
 * value (according to thw comparison function). Otherwise
 * this operation has no effect.
 * @returns A boolean indicating whether the insertion was
 * successful.
 */
bool avl_ins(avl *self, void *val);


/**
 * @brief Deletes the node matching a given @p key
 * (according to the AVL comparison function) , if any, and
 * copies of the reference stored therein prior to removal
 * into @p dest. If no such node is found, the operation
 * has no effect.
 * @returns A boolean indicating whether the deletion was
 * successful.
 */
bool avl_del(avl *self, void *key, void **dest);


/**
 * @brief Prints the AVL tree in the given output @p stream.
 *
 * @par prt_val is the function used to print the stored values.
 * Similarly to the comparator function, the @p prt_val
 * function receives a pointer to whatever is stored in the
 * tree nodes.
 * ## Example
 * If the node stores references to objects of type `obj_t`
 * ```C
 * typedef struct {
 *      int key;
 *      double value;
 * } obj_t;
 * ```
 *
 * Then the print function could be something like
 * ```C
 * void print_obj_t(FILE *stream, const void *ptr) {
 *      fprintf(stream, "(key=%d value=%f)", (*((obj_t **)ptr))->key, (*((obj_t **)ptr))->value);
 * }
 * ```
 */
void avl_print( const avl *self, FILE *stream,
                void (*prt_val)(FILE *, const void *));



#define AVL_DECLARE_ALL(TYPE, ...)\
	bool avl_get_##TYPE(avl *self, TYPE val, TYPE *dest);\
	bool avl_ins_##TYPE(avl *self, TYPE val);\
	bool avl_del_##TYPE(avl *self, TYPE val, TYPE *dest);


XX_CORETYPES(AVL_DECLARE_ALL)


/**
 * @brief AVL traversal order
 */
typedef enum {
	PRE_ORDER  = 0,
	IN_ORDER   = 1,
	POST_ORDER = 2,
} avl_traversal_order;


/**
 * @brief AVL iterator. Implements the iter trait.
 * @see trait.h iter.h
 */
typedef struct _avl_iter avl_iter;


/**
 * @brief Returns an iterator for the AVL nodes in the given
 * traversal order.
 */
avl_iter *avl_get_iter(avl *self, avl_traversal_order order);


/**
 * @brief AVL iterator destructor
 */
void avl_iter_free(avl_iter *self);


DECL_TRAIT(avl_iter, iter);


#endif
