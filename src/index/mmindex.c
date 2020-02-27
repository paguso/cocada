#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "alphabet.h"
#include "cocadautil.h"
#include "minqueue.h"
#include "hashmap.h"
#include "vector.h"
#include "xstring.h"

#include "mmindex.h"

struct _mmindex {
    size_t nstr;
    vector *w;
    vector *k;
    vector *tbls;
    vector *offs;
};

