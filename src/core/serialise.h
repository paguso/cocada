#ifndef SERIALISE_H
#define SERIALISE_H

#include <stdio.h>

#include "coretype.h"
#include "vec.h"

/// serialisation template


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
    ser_arr,
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
    ser_struct, 
    ser_ptr
} ser_t;          


typedef struct _ser {
    ser_t  type;
    void  *addr;
    size_t size;
    size_t nchd;
    struct _ser **chd;
} ser;


ser *ser_new(ser_t type, void *addr, size_t size);

ser *ser_cons(ser *par, const ser *chd);

size_t ser_nchd(ser *self);

const ser *ser_chd(ser *self, size_t i);



///////// serialisable trait

typedef struct _serialisable serialisable;

typedef struct {
    ser* (*get_ser)(serialisable *trait_obj);
} serialisable_vt;


struct _serialisable {
    serialisable_vt *vt;
    void *impltor;
};


ser *get_ser(serialisable *trait_obj);




void serialise(ser *st, FILE *stream); 


#endif