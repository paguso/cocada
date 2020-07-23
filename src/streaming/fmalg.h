#ifndef FMALG_H
#define FMALG_H

#include <inttypes.h>


typedef struct _fmalg fmalg; 


fmalg *fmalg_init(uint64_t maxval);


void fmalg_free(fmalg *fm);


void fmalg_reset(fmalg *fm);


void fmalg_process(fmalg *fm, uint64_t val);


uint64_t fmalg_query(fmalg *fm);


#endif