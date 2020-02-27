#ifndef MMINDEX_H
#define MMINDEX_H


#include "alphabet.h"
#include "xstring.h"

typedef struct _mmindex mmindex;


mmindex *mmindex_new(alphabet *ab);

void *mmindex_index_xstr(mmindex *mmidx, xstring *xs);


#endif