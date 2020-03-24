#include "strread.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "new.h"
#include "xchar.h"
#include "xstring.h"



strread_vt strread_vt_new()
{
    strread_vt vt;
    memset(&vt, 0x0, sizeof(strread_vt)); // set all functions to NULL
    return vt;
}

/*
strread strread_init_trait(void *impltor)
{
    strread ret;// = NEW(strread);
    ret.impltor = impltor;
    ret.vtbl = _strread_vt_init();
    return ret;
}
*/

void strread_reset(strread *trait) 
{
    trait->vtbl->reset(trait->impltor);
}


xchar_t strread_getc(strread *trait)
{
    return trait->vtbl->getc(trait->impltor);
}


size_t strread_read_str_until(strread *trait, char *dest, char delim)
{
    return trait->vtbl->read_str_until(trait->impltor, dest, delim);
}

/*
size_t strread_read_xstr(strread *trait, xstring *dest, size_t n)
{
    return trait->vtbl->read_xstr(trait->impltor, dest, n);
}

size_t strread_read_xstr_until(strread *trait, xstring *dest, xchar_t delim)
{
    return trait->vtbl->read_xstr_until(trait->impltor, dest, delim);
}

void strread_close(strread *trait)
{
    trait->vtbl->close(trait->impltor);
}
*/

size_t strread_sizeof_char(strread *trait)
{
    return trait->vtbl->sizeof_char(trait->impltor);
}



