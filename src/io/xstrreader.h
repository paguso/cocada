
#ifndef XSTRREADER_H
#define XSTRREADER_H

#include "trait.h"
#include "xstr.h"

typedef struct _xstrreader xstrreader;

xstrreader *xstrreader_open(xstr *src);

xstrreader *xstrreader_open_str(char *src, size_t len);

void xstrreader_close(xstrreader *self);

DECL_TRAIT(xstrreader, xstrread)


#endif