#include <stdlib.h>
#include <stdio.h>

#include "hash.h"
#include "hashmap.h"
#include "new.h"
#include "serialise.h"

typedef struct {
    hashmap *ser_to_cur_mem_map;
    hashmap *cur_to_ser_mem_map;
}
mem_context;


mem_context *mem_context_new() 
{
    mem_context *ret = NEW(mem_context);
    ret->ser_to_cur_mem_map = hashmap_new(sizeof(size_t), sizeof(size_t), ident_hash_size_t, eq_size_t); 
    ret->cur_to_ser_mem_map = hashmap_new(sizeof(size_t), sizeof(size_t), ident_hash_size_t, eq_size_t); 
    return ret;
}


typedef enum {
    ser_char,           
    ser_uchar,          
    ser_short,          
    ser_ushort,         
    ser_int,            
    ser_uint,           
    ser_long,           
    ser_ulong,          
    ser_llong,          
    ser_ullong,         
    ser_float,          
    ser_double,         
    ser_ldouble,        
    ser_bool,          
    ser_size_t,         
    ser_int8_t,         
    ser_uint8_t,        
    ser_int16_t,        
    ser_uint16_t,       
    ser_int32_t,        
    ser_uint32_t,       
    ser_int64_t,        
    ser_uint64_t,       
    ser_byte_t,        
    //ARRAY TYPES
    ser_arr_char,           
    ser_arr_uchar,          
    ser_arr_short,          
    ser_arr_ushort,         
    ser_arr_int,            
    ser_arr_uint,           
    ser_arr_long,           
    ser_arr_ulong,          
    ser_arr_llong,          
    ser_arr_ullong,         
    ser_arr_float,          
    ser_arr_double,         
    ser_arr_ldouble,        
    ser_arr_bool,          
    ser_arr_size_t,         
    ser_arr_int8_t,         
    ser_arr_uint8_t,        
    ser_arr_int16_t,        
    ser_arr_uint16_t,       
    ser_arr_int32_t,        
    ser_arr_uint32_t,       
    ser_arr_int64_t,        
    ser_arr_uint64_t,       
    ser_arr_byte_t,        
    ser_arr_rawptr,
    ser_arr_struct,
} ser_arr;          


typedef enum {
    ser_prim_ptr,
    ser_flat_arr_ptr,
    ser_ptr_arr_ptr,
    ser_struct_ptr,
} ser_ptr;

#define SERIALISE_PRIM_IMPL(TYPE, ...)\
void serialise_##TYPE( TYPE *src , FILE *stream)\
{\
    byte_t type_ind = (byte_t)(ser_##TYPE);\
    fwrite(&type_ind, sizeof(byte_t), 1, stream);\
    fwrite(src, sizeof(TYPE), 1, stream);\
}

#define DESERIALISE_PRIM_IMPL(TYPE, ...)\
void deserialise_##TYPE( TYPE *dest , FILE *stream)\
{\
    byte_t type_ind;\
    fread(&type_ind, sizeof(byte_t), 1, stream);\
    fread(dest, sizeof(TYPE), 1, stream);\
}

XX_PRIMITIVES(SERIALISE_PRIM_IMPL)
XX_PRIMITIVES(DESERIALISE_PRIM_IMPL)

#define SERIALISE_ARR(TYPE, ...)\
void serialise_arr_of_##TYPE(TYPE *src, size_t nmemb, FILE *stream)\
{\
    byte_t type_ind = (byte_t)(ser_arr_##TYPE);\
    fwrite(&type_ind, sizeof(byte_t), 1, stream);\
    size_t size = nmemb * sizeof(TYPE);\
    fwrite(&size, sizeof(size_t), 1, stream);
    fwrite(src, sizeof(TYPE), nmemb, stream);\
}

#define DESERIALISE_ARR(TYPE, ...)\
void deserialise_arr_of_##TYPE(TYPE *dest, FILE *stream)\
{\
    byte_t type_ind;\
    fread(&type_ind, sizeof(byte_t), 1, stream);\
    size_t size;\
    fread(&size, sizeof(size_t), 1, stream);\
    fread(dest, size, 1, stream);\
}

XX_PRIMITIVES(SERIALISE_ARR)
XX_PRIMITIVES(DESERIALISE_ARR)

// Pointer to primitives
#define SERIALISE_PTR(TYPE, ...)
void serialise_ptr_to_##TYPE(void *src, FILE *stream, mem_context *ctx)\
{\
    size_t cur_mem_addr = (size_t)(*((void **)src));\
    if (hashmap_has_key(ctx->cur_to_ser_mem_map, &cur_mem_addr)) {\
    } else {\
        serialise_##TYPE(*((TYPE **)src), stream );
    }
}

