#ifndef STRFILEREAD_H
#define STRFILEREAD_H


#include "trait.h"

typedef struct _strfileread strfileread;

strfileread *strfileread_open(char *filename);


DECL_TRAIT(strfileread, strread)



#endif