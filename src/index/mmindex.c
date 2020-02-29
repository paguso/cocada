#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "alphabet.h"
#include "new.h"
#include "minqueue.h"
#include "hashmap.h"
#include "vec.h"
#include "xstring.h"

#include "mmindex.h"

struct _mmindex {
    size_t nstr;
    vec *w;
    vec *k;
    vec *tbls;
    vec *offs;
};

