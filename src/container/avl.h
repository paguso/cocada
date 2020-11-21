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
#include "new.h"
#include "order.h"


/**
 * @file avl.h
 * @brief  AVL tree
 * @author Paulo Fonseca
 * 
 * The AVL tree is a height-balanced search tree with O(lg n) 
 * worst-case search, insertion and deletion.
 */


/**
 * Opaque AVL type
 */
typedef struct _avl avl;


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 * @param cmp Key comparison function 
 * @see order.h
 */
avl *avl_new(size_t typesize, cmp_func cmp);


/**
 * @brief Destructor
 */
void avl_dtor(void *ptr, const dtor *dt);


/**
 * @brief Return a pointer to the internal data matching the given *key
 * if it exists. Else returns NULL
 */
const void *avl_get(avl *self, void *key);


/**
 * @brief Inserts a copy of the value pointed at by @p val in the tree.
 * @warning If the tree already has a node matching @p val, nothing is inserted.
 */
void avl_push(avl *self, void *val);


/** 
 * @brief Prints the AVL tree in the given output @p stream.
 * @par prt_val is the function used to print the stored values.
 */
void avl_print(const avl *self, FILE *stream, void (*prt_val)(FILE *, const void *));


#endif
