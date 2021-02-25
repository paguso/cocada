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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "arrays.h"
#include "cstrutil.h"
#include "coretype.h"
#include "deque.h"
#include "errlog.h"
#include "hash.h"
#include "hashmap.h"
#include "hashset.h"
#include "mathutil.h"
#include "memdbg.h"
#include "new.h"
#include "serialise.h"

extern int errno;


#define SOM_T(TYPE,...) som_##TYPE,

typedef enum {
	//SOM_T(error) // should be first = 0
	XX_PRIMITIVES(SOM_T)
	SOM_T(rawptr)
	SOM_T(arr)
	SOM_T(cstr)
	SOM_T(struct)
	SOM_T(proxy)
} som_t;


struct _sub_som {
	size_t off;
	som *chd;
};


struct _som {
	som_t  type;
	get_som_func get_som;
	size_t size;
	size_t nchd;
	sub_som *chd;
};


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
GET_SOM_IMPL(cstr)



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



static bool contains_addr(vec *chunks, size_t addr)
{
	size_t l = 0, r = vec_len(chunks);
	// ans in [l,r)
	while ((r - l) > 0) {
		size_t m = (l + r) / 2;
		mem_chunk *chk = (mem_chunk *)vec_get(chunks, m);
		if ( addr < chk->start) {
			r = m;
		}
		else if ( chk->start <= addr  && addr < chk->start + chk->size ) {
			return true;
		}
		else {
			l = m + 1;
		}
	}
	return false;
}


static void add_chunk(vec *chunks, mem_chunk ck)
{
	size_t pos;
	size_t addr = ck.start;
	if (vec_len(chunks) == 0) {
		pos = 0;
	}
	else if (addr <= ((mem_chunk *)(vec_first(chunks)))->start) {
		pos = 0;
	}
	else if (((mem_chunk *)(vec_last(chunks)))->start < addr) {
		pos =  vec_len(chunks);
	}
	else {
		size_t l = 0, r = vec_len(chunks) - 1;
		while (r - l > 1) { // l < ans <= r
			size_t m = (l + r) / 2;
			if (((mem_chunk *)(vec_get(chunks, m)))->start < addr) {
				l = m;
			}
			else {
				r = m;
			}
		}
		pos = r;
	}
	vec_ins(chunks, pos, &ck);
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


static void write_obj(som *model, void *ptr, FILE *stream, deque *dq,
                      vec *written, bool check_if_written);


static void read_obj(som *model, void *ptr, FILE *stream, deque *dq, vec *read,
                     hashmap *mem_map);


static void write_prim(som *model, void *ptr, FILE *stream, vec *written)
{
	write_type(model->type, stream);
	write_addr(ptr, stream);
	write_size(model->size, stream);
	fwrite(ptr, model->size, 1, stream);
	mem_chunk chunk = {.start = (size_t)ptr, .size = model->size};
	//vec_push(written, &chunk);
	add_chunk(written, chunk);
}


static void read_prim(som *model, void *dest, FILE *stream, vec *read,
                      hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type,
	            "Type mismatch. Expected SOM type %d; found %d.\n",
	            (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	WARN_ASSERT(model->size == size, "Primitive type size mismatch."
	            "SOM (in-memory) type size is %zu; serialised type size is %zu bytes.\n",
	            model->size, size);
	fread(dest, model->size, 1, stream);
	mem_chunk chunk = {.start = addr, .size=model->size};
	//vec_push(read, &chunk);
	add_chunk(read, chunk);
	hashmap_set(mem_map, &addr, &dest);
}


void write_rawptr(som *model, void *ptr, FILE *stream, deque *dq, vec *written)
{
	write_type(som_rawptr, stream);
	write_addr(ptr, stream);
	write_size(sizeof(rawptr), stream);
	fwrite(ptr, sizeof(rawptr), 1, stream);
	mem_chunk chunk = {.start = (size_t) ptr, .size=sizeof(rawptr)};
	//vec_push(written, &chunk);
	add_chunk(written, chunk);
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
	ERROR_ASSERT(hashmap_has_key(mem_map, &base),
	             "Cannot map already read address %p.\n", base);
	base = hashmap_get_rawptr(mem_map, &base);
	return base + off;
}


void read_rawptr(som *model, void *ptr, FILE *stream, deque *dq, vec *read,
                 hashmap *mem_map)
{
	rawptr *dest = (rawptr *)ptr;
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type,
	            "Pointer type mismatch. Expected SOM type %d; found %d.\n",
	            (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	WARN_ASSERT(model->size == size, "Pointer type size mismatch."
	            "SOM (in-memory) type size is %zu; serialised type size is %zu bytes.\n",
	            model->size, size);
	fread(dest, size, 1, stream);
	mem_chunk chunk = {.start = addr, .size = size};
	//vec_push(read, &chunk);
	add_chunk(read, chunk);
	hashmap_set(mem_map, &addr, &dest);
	if ( contains_addr(read, (size_t)(*dest)) ) {
		*dest = map_addr(mem_map, read, *dest);
		return;
	}
	if (som_nchd(model) > 0) {
		som *pointee_model = som_chd(model, 0).chd;
		while (pointee_model->type == som_proxy) {
			pointee_model = pointee_model->get_som();
		}
		void *new_obj = NULL;
		if (pointee_model->type == som_arr || pointee_model->type == som_cstr) {
			new_obj = ptr;
		}
		else {
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
	//vec_push(written, &chunk);
	add_chunk(written, chunk);
	for (size_t i = 0; i < som_nchd(model); i++) {
		sub_som field_som_chd = som_chd(model, i);
		som *field_som = field_som_chd.chd;
		write_obj(field_som, obj + field_som_chd.off, stream, dq, written, false);
	}
}


void read_struct(som *model, void *dest, FILE *stream, deque *dq, vec *read,
                 hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type,
	            "Struct type mismatch. Expected SOM type %d, found %d.\n",
	            (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	WARN_ASSERT(model->size == size,
	            "Struct size error. Expected %zu, found %zu.\n",
	            model->size, size);
	mem_chunk chunk = {.start = addr, .size = size};
	//vec_push(read, &chunk);
	add_chunk(read, chunk);
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
	ERROR_ASSERT(som_nchd(model) == 1, "Array SOM requires one nested child.\n");
	write_type(som_arr, stream);
	write_addr(arr, stream);
	size_t size = arr_sizeof(arr);
	write_size(size, stream);
	mem_chunk chunk = {.start = (size_t) arr, .size = size};
	//vec_push(written, &chunk);
	add_chunk(written, chunk);
	som *elt_som = som_chd(model, 0).chd;
	size_t elt_size = elt_som->size;
	switch (elt_som->type) {
	case som_arr:
		ERROR("Unsupported array of array serialisation. See module documentation.\n");
		break;
	case som_cstr:
		ERROR("Unsupported array of string serialisation. See module documentation.\n");
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


void write_string(som *model, void *arr, FILE *stream, deque *dq, vec *written)
{
	write_type(model->type, stream);
	write_addr(arr, stream);
	size_t size = strlen(arr) + 1;
	write_size(size, stream);
	mem_chunk chunk = {.start = (size_t) arr, .size = size};
	//vec_push(written, &chunk);
	add_chunk(written, chunk);
	write_blob(arr, size, stream);
}


static void read_blob(void *ptr, size_t size, FILE *stream)
{
	fread(ptr, size, 1, stream);
}


// ptr_addr contains the address of the pointer to the array to be read in
// the space taken by the array is known only when it is read,
// so the array could not have been allocated before
void read_arr(som *model, void *ptr_addr, FILE *stream, deque *dq, vec *read,
              hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type,
	            "Array type mismatch. Expected SOM type %d; found %d\n",
	            (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	mem_chunk chunk = {.start = addr, .size = size};
	//vec_push(read, &chunk);
	add_chunk(read, chunk);

	som *elt_som = som_chd(model, 0).chd;
	size_t elt_size = elt_som->size;
	while (elt_som->type == som_proxy) {
		elt_som = elt_som->get_som();
	}
	WARN_ASSERT((size % elt_som->size) == 0,
	            "Incompatible array size for the element size.\n");
	size_t len = size / elt_size;
	void *arr = arr_calloc(len, elt_size);
	hashmap_set(mem_map, &addr, &arr);
	*((rawptr *)ptr_addr) = arr;

	switch (elt_som->type) {
	case som_arr:
		ERROR("Unsupported array of array serialisation. See module documentation.\n");
		break;
	case som_cstr:
		ERROR("Unsupported array of cstr serialisation. See module documentation.\n");
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


void read_string(som *model, void *ptr_addr, FILE *stream, deque *dq, vec *read,
                 hashmap *mem_map)
{
	size_t type = read_type(stream);
	WARN_ASSERT(model->type == type,
	            "String type mismatch. Expected SOM type %d; found %d\n",
	            (int)model->type, (int)type);
	size_t addr = read_addr(stream);
	size_t size = read_size(stream);
	mem_chunk chunk = {.start = addr, .size = size};
	//vec_push(read, &chunk);
	add_chunk(read, chunk);

	void *str = cstr_new(size-1);
	hashmap_set(mem_map, &addr, &str);
	*((rawptr *)ptr_addr) = str;
	read_blob(str, size, stream);
}



static void write_obj(som *model, void *obj, FILE *stream, deque *dq,
                      vec *written, bool check_if_written)
{
	while (model->type == som_proxy) {
		model = model->get_som();
	}
	if (check_if_written && contains_addr(written, (size_t)obj)) {
		return;
	}
	switch (model->type) {
	case som_rawptr:
		write_rawptr(model, obj, stream, dq, written);
		break;
	case som_arr:
		write_arr(model, obj, stream, dq, written);
		break;
	case som_cstr:
		write_string(model, obj, stream, dq, written);
		break;
	case som_struct:
		write_struct(model, obj, stream, dq, written);
		break;
	default:
		write_prim(model, obj, stream, written);
		break;
	}
}


static void read_obj(som *model, void *dest, FILE *stream, deque *dq, vec *read,
                     hashmap *mem_map)
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
	case som_cstr:
		read_string(model, dest, stream, dq, read, mem_map);
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
	//vec_push(written, &nullchunk);
	add_chunk(written, nullchunk);
	deque *dq = deque_new(sizeof(obj_model));
	obj_model om = {.obj=obj, .model=model};
	deque_push_back(dq, &om);
	while (!deque_empty(dq)) {
		deque_pop_front(dq, &om);
		write_obj(om.model, om.obj, stream, dq, written, true);
	}
	DESTROY_FLAT(written, vec);
	DESTROY_FLAT(dq, deque);
}


static void *bfs_read(som *model, FILE *stream)
{
	mem_chunk nullchunk = {.start=(size_t)NULL, .size=1};
	vec *read = vec_new(sizeof(mem_chunk));
	//vec_push(read, &nullchunk);
	add_chunk(read, nullchunk);
	hashmap *mem_map = hashmap_new( sizeof(size_t), sizeof(size_t),
	                                ident_hash_size_t, eq_size_t );
	void *nullptr = NULL;
	hashmap_set(mem_map, &nullptr, &nullptr);
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
	DESTROY_FLAT(read, vec);
	DESTROY_FLAT(dq, deque);
	DESTROY_FLAT(mem_map, hashmap);
	return ret;
}


void serialise(void *obj, som *model, FILE *stream)
{
	bfs_write(model, obj, stream);
}


void *deserialise(som *model, FILE *stream)
{
	return bfs_read(model, stream);
}