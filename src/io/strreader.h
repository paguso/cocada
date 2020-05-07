
#ifndef STRREADER_H
#define STRREADER_H

#include "trait.h"

typedef struct _strreader strreader;

strreader *strreader_open(char *src, size_t len);

void strreader_close(strreader *rdr);

DECL_TRAIT(strreader, strread)


#endif