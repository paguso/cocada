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

#include <stdlib.h>
#include <stdio.h>

#include "arrays.h"
#include "errlog.h"
#include "hash.h"
#include "hashmap.h"
#include "hashset.h"
#include "mathutil.h"
#include "new.h"
#include "serialise.h"
#include "deque.h"


static som *_som_new(som_t type, size_t size, get_som_func get_som)
{
	som *ret = NEW(som);
	ret->type = type;
	ret->get_som = get_som;
	ret->size = size;
	ret->nchd = 0;
	ret->chd = NEW_ARR(sub_som, 1) ;
	return ret;
}


#define GET_SOM_IMPL(TYPE, ...)\
	\
	static som* _som_##TYPE = NULL;\
	\
	som *get_som_##TYPE() {\
		if (_som_##TYPE==NULL) {\
			_som_##TYPE = _som_new(som_##TYPE, sizeof(TYPE), NULL);\
		}\
		return _som_##TYPE;\
	}

XX_PRIMITIVES(GET_SOM_IMPL)




som *som_arr_new()
{
	return _som_new(som_arr, 0, NULL);
}


som *som_ptr_new()
{
	return _som_new(som_rawptr, sizeof(rawptr), NULL);
}


som *som_struct_new(size_t size, get_som_func get_som)
{
	return _som_new(som_struct, size, get_som);
}


som *som_proxy_new(get_som_func get_som)
{
	return _som_new(som_proxy, 0, get_som);
}


som *som_cons(som *par, size_t off,  som *chd)
{
	if (IS_POW2(par->nchd)) {
		par->chd = (sub_som *) realloc(par->chd, ( 2 * par->nchd) * sizeof(sub_som));
	}
	sub_som ss = {.off = off, .chd=chd};
	par->chd[par->nchd++] = ss;
	return par;
}


size_t som_nchd(som *self)
{
	return self->nchd;
}


sub_som som_chd(som *self, size_t i)
{
	return self->chd[i];
}


typedef struct {
	void *obj;
	som *model;
} obj_model;


typedef struct {
	size_t start;
	size_t size;
} mem_chunk;


static bool is_written(vec *written, void *addr)
{
	for (size_t i=0, l=vec_len(written); i<l; i++) {
		mem_chunk *chk = (mem_chunk *)vec_get(written, i);
		if ( (size_t)chk->start <= (size_t)addr  &&
		        (size_t)addr < (size_t)chk->start + (size_t)chk->size ) {
			return true;
		}
	}
	return false;
}


static void write_type(som_t typ, FILE *stream)
{
	byte_t btyp = (byte_t) typ;
	fwrite(&btyp, 1, 1, stream);
}


static som_t read_type(FILE *stream)
{
	byte_t btyp;
	fread(&btyp, 1, 1, stream);
	return (som_t)btyp;
}


static void write_addr(void *ptr, FILE *stream)
{
	size_t addr = (size_t) ptr;
	fwrite(&addr, sizeof(size_t), 1, stream);
}


static size_t read_addr(FILE *stream)
{
	size_t addr;
	fread(&addr, sizeof(size_t), 1, stream);
	return addr;
}


static void write_size(size_t size, FILE *stream)
{
	fwrite(&size, sizeof(size_t), 1, stream);
}


static size_t read_size(FILE *stream)
{
	size_t size;
	fread(&size, sizeof(size_t), 1, stream);
	return size;
}


static void write_obj(som *model, void *ptr, FILE *stream, deque *dq, vec *written);


static void read_obj(som *model, void *ptr, FILE *stream, deque *dq, vec *read, hashmap *mem_map);


static void write_prim(som *model, void *ptr, FILE *stream, vec *written)
{
    write_type(model->type, stream);
    write_addr(ptr, stream);
    write_size(model->size, stream);
    fwrite(ptr, model->size, 1, stream);
    mem_chunk chunk = {.start = (size_t)ptr, .size = model->size};
    vec_push(written, &chunk);
}


static void read_prim(som *model, void *dest, FILE *stream, vec *read, hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type, "Wrong type. Expected %d, found %d", (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	WARN_ASSERT(model->size == size, "Wrong size. Expected %zu, found %zu", model->size, size);
    fread(dest, model->size, 1, stream);
    mem_chunk chunk = {.start = addr, .size=model->size};
    vec_push(read, &chunk);
	hashmap_set(mem_map, &addr, &dest);
}


void write_rawptr(som *model, void *ptr, FILE *stream, deque *dq, vec *written)
{
	write_type(som_rawptr, stream);
	write_addr(ptr, stream);
	write_size(sizeof(rawptr), stream);
	fwrite(ptr, sizeof(rawptr), 1, stream);
	mem_chunk chunk = {.start = (size_t) ptr, .size=sizeof(rawptr)};
	vec_push(written, &chunk);
	if (som_nchd(model) > 0) {
		sub_som chd = som_chd(model, 0);
		obj_model om = {.model = chd.chd, .obj = *((rawptr *)ptr)};
		deque_push_back(dq, &om);
	}
}


void *map_addr(hashmap *mem_map, vec *read, void *addr) 
{
	if (addr == NULL) return NULL;
	void *base = NULL;
	for (size_t i=0, l=vec_len(read); i<l; i++) {
		mem_chunk *chk = (mem_chunk *)vec_get(read, i);
		if ( (size_t)chk->start <= (size_t)addr  &&
		        (size_t)addr < (size_t)chk->start + (size_t)chk->size ) {
			base = (void *) chk->start;
			break;
		}
	}
	size_t off = (size_t)addr - (size_t)base;
	ERROR_ASSERT(hashmap_has_key(mem_map, &base), "Cannot map already read address %p.", base);
	base = hashmap_get_rawptr(mem_map, &base);
	return base + off;
}


void read_rawptr(som *model, void *ptr, FILE *stream, deque *dq, vec *read, hashmap *mem_map)
{
	rawptr *dest = (rawptr *)ptr;
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type, "Wrong type. Expected %d, found %d", (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	WARN_ASSERT(model->size == size, "Wrong size. Expected %zu, found %zu", model->size, size);
	fread(dest, size, 1, stream);
	mem_chunk chunk = {.start = addr, .size = size};
	vec_push(read, &chunk);
	hashmap_set(mem_map, &addr, &dest);
	if (is_written(read, *dest)) {
		*dest = map_addr(mem_map, read, dest);
		return;
	} 
	if (som_nchd(model) > 0) {
		som *pointee_model = som_chd(model, 0).chd;
		while (pointee_model->type == som_proxy) {
			pointee_model = pointee_model->get_som();
		}
		void *new_obj = NULL;
		if (pointee_model->type == som_arr) {
			new_obj = ptr;
		} else {
			new_obj = malloc(pointee_model->size);
		}
		*dest = new_obj;
		obj_model om = {.model = pointee_model, .obj = new_obj};
		deque_push_back(dq, &om);
	}
}


void write_struct(som *model, void *obj, FILE *stream, deque *dq, vec *written)
{
    write_type(som_struct, stream);
    write_addr(obj, stream);
    write_size(model->size, stream);
	mem_chunk chunk = {.start = (size_t) obj, .size = model->size};
	vec_push(written, &chunk);
	for (size_t i = 0; i < som_nchd(model); i++) {
		sub_som field_som_chd = som_chd(model, i);
		som *field_som = field_som_chd.chd;
        write_obj(field_som, obj + field_som_chd.off, stream, dq, written);
	}
}


void read_struct(som *model, void *dest, FILE *stream, deque *dq, vec *read, hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type, "Wrong type. Expected %d, found %d", (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	WARN_ASSERT(model->size == size, "Wrong size. Expected %zu, found %zu", model->size, size);
	mem_chunk chunk = {.start = addr, .size = size};
	vec_push(read, &chunk);
	hashmap_set(mem_map, &addr, &dest);
	for (size_t i = 0; i < som_nchd(model); i++) {
		sub_som field_som_chd = som_chd(model, i);
		som *field_som = field_som_chd.chd;
        read_obj(field_som, dest + field_som_chd.off, stream, dq, read, mem_map);
	}
}


static void write_blob(void *ptr, size_t size, FILE *stream)
{
	fwrite(ptr, size, 1, stream);
}


void write_arr(som *model, void *arr, FILE *stream, deque *dq, vec *written)
{
	ERROR_ASSERT(som_nchd(model) == 1, "Array SOM requires one nested child.");
	write_type(som_arr, stream);
	write_addr(arr, stream);
	size_t size = arr_size(arr);
	write_size(size, stream);
	mem_chunk chunk = {.start = (size_t) arr, .size = size};
	vec_push(written, &chunk);
	som *elt_som = som_chd(model, 0).chd;
	size_t elt_size = elt_som->size;
	switch (elt_som->type) {
	case som_arr:
		ERROR("Unsupported array of array serialisation");
		break;
	case som_rawptr:
		for (void *elt = arr; elt < arr + size; elt += elt_size) {
			write_rawptr(elt_som, elt, stream, dq, written);
		}
		break;
	case som_struct:
		for (void *elt = arr; elt < arr + size; elt += elt_size) {
			write_struct(elt_som, elt, stream, dq, written);
		}
		break;
	default: // primitive type: write blob
		write_blob(arr, size, stream);
		break;
	}
}


static void read_blob(void *ptr, size_t size, FILE *stream)
{
	fread(ptr, size, 1, stream);
}


// ptr_addr contains the address of the pointer to the array to be read in
// the space taken by the array is known only when it is read,
// so the array could not have been allocated before
void read_arr(som *model, void *ptr_addr, FILE *stream, deque *dq, vec *read, hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type, "Wrong type. Expected %d, found %d\n", (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	mem_chunk chunk = {.start = addr, .size = size};
	vec_push(read, &chunk);
	
	som *elt_som = som_chd(model, 0).chd;
	size_t elt_size = elt_som->size;
	while (elt_som->type == som_proxy) {
		elt_som = elt_som->get_som();
	}
	WARN_ASSERT((size % elt_som->size) == 0, "Incompatible array size for the element size.\n");
	size_t len = size / elt_size;
	void *arr = arr_new(elt_size, len);
	hashmap_set(mem_map, &addr, &arr);
	*((rawptr *)ptr_addr) = arr;

	switch (elt_som->type) {
	case som_arr:
		ERROR("Unsupported array of array serialisation.\n");
		break;
	case som_rawptr:
		for (void *elt = arr; elt < arr + size; elt += elt_size) {
			read_rawptr(elt_som, elt, stream, dq, read, mem_map);
		}
		break;
	case som_struct:
		for (void *elt = arr; elt < arr + size; elt += elt_size) {
			read_struct(elt_som, elt, stream, dq, read, mem_map);
		}
		break;
	default: // primitive type: write blob
		read_blob(arr, size, stream);
		break;
	}
}


static void write_obj(som *model, void *obj, FILE *stream, deque *dq, vec *written)
{
    while (model->type == som_proxy) {
        model = model->get_som();
    }
    if (is_written(written, obj)) {
        return;
    }
    switch (model->type) {
    case som_rawptr:
        write_rawptr(model, obj, stream, dq, written);
        break;
    case som_arr:
        write_arr(model, obj, stream, dq, written);
        break;
    case som_struct:
        write_struct(model, obj, stream, dq, written);
        break;
    default:
        write_prim(model, obj, stream, written);
        break;
    }
}


static void read_obj(som *model, void *dest, FILE *stream, deque *dq, vec *read, hashmap *mem_map)
{
    while (model->type == som_proxy) {
        model = model->get_som();
    }
    switch (model->type) {
    case som_rawptr:
        read_rawptr(model, dest, stream, dq, read, mem_map);
        break;
    case som_arr:
        read_arr(model, dest, stream, dq, read, mem_map);
        break;
    case som_struct:
        read_struct(model, dest, stream, dq, read, mem_map);
        break;
    default:
        read_prim(model, dest, stream, read, mem_map);
        break;
    }
}


static void bfs_write(som *model, void *obj, FILE *stream)
{
	mem_chunk nullchunk = {.start=(size_t)NULL, .size=1};
    vec *written = vec_new(sizeof(mem_chunk));
	vec_push(written, &nullchunk);
	deque *dq = deque_new(sizeof(obj_model));
	obj_model om = {.obj=obj, .model=model};
	deque_push_back(dq, &om);
	while (!deque_empty(dq)) {
		deque_pop_front(dq, &om);
        write_obj(om.model, om.obj, stream, dq, written);
	}
    FREE(written, vec);
    FREE(dq, deque);
}


static void *bfs_read(som *model, FILE *stream)
{
	mem_chunk nullchunk = {.start=(size_t)NULL, .size=1};
    vec *read = vec_new(sizeof(mem_chunk));
	vec_push(read, &nullchunk);
	hashmap *mem_map = hashmap_new( sizeof(size_t), sizeof(size_t), 
									ident_hash_size_t, eq_size_t );
	deque *dq = deque_new(sizeof(obj_model));

	rawptr *ptr = NEW(rawptr);
	*ptr = (model->type == som_arr) ? ptr : malloc(model->size);
	obj_model om = {.model = model, .obj = *ptr};
	deque_push_back(dq, &om);
	while (!deque_empty(dq)) {
		deque_pop_front(dq, &om);
        read_obj(om.model, om.obj, stream, dq, read, mem_map);
	}
	rawptr ret = *ptr;
	FREE(ptr);
    FREE(read, vec);
    FREE(dq, deque);
	FREE(mem_map, hashmap);
	return ret;
}


void serialise(void *obj, som *model, FILE *stream)
{
    bfs_write(model, obj, stream);
}


void *deserialise(som *model, FILE *stream) {
	return bfs_read(model, stream);
}