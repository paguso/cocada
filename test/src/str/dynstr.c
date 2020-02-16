/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 * 
 * Copyright (C) 2016  Paulo G S Fonseca
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <string.h>

#include "cocadautil.h"
#include "cstringutil.h"
#include "dynstr.h"


const static size_t DEFAULT_CAPACITY = 8;


typedef struct _dynstr {
    char *str;
    size_t len;
    size_t capacity;
}
dynstr;


dynstr *dynstr_new()
{
    return dynstr_new_with_capacity(DEFAULT_CAPACITY);
}


dynstr *dynstr_new_with_capacity(size_t init_capacity)
{
    dynstr *ret;
    ret = NEW(dynstr);
    ret->capacity = init_capacity;
    ret->len = 0;
    ret->str = cstr_new(ret->capacity);
    return ret;
}


dynstr *dynstr_new_from_str(char *src)
{
    dynstr *ret;
    ret = NEW(dynstr);
    ret->len = ret->capacity = strlen(src);
    ret->str = cstr_new(ret->capacity);
    strcpy(ret->str, src);
    return ret;
}

void dynstr_free(dynstr *dstr)
{
    if (dstr==NULL) return;
    FREE(dstr->str);
    FREE(dstr);
}

size_t dstr_len(dynstr *dstr)
{
    return dstr->len;
}

size_t dstr_capacity(dynstr *dstr)
{
    return dstr->capacity;
}

char dstr_get(dynstr *dstr, size_t pos)
{
    return dstr->str[pos];
}


void dstr_clear(dynstr *dstr)
{
    memset(dstr->str, '\0', dstr->capacity);
    dstr->len = 0;
}


void dstr_set(dynstr *dstr, size_t pos, char c)
{
    dstr->str[pos] = c;
}

void _double(dynstr *dstr)
{
    dstr->capacity = MAX(1, 2*dstr->capacity);
    dstr->str = realloc(dstr->str, (dstr->capacity+1)*sizeof(char));
    cstr_fill(dstr->str, dstr->len, dstr->capacity, '\0');
}

void dstr_append(dynstr *dstr, char *suff)
{
    size_t slen = strlen(suff);
    while (dstr->len + slen >= dstr->capacity) {
        _double(dstr);
    }
    strcpy(dstr->str+dstr->len, suff);
    dstr->len += slen;
    dstr->str[dstr->len] = '\0';

}


void dstr_append_char(dynstr *dstr, char c)
{
    if (dstr->len == dstr->capacity) {
        _double(dstr);
    }
    dstr->str[dstr->len] = c;
    dstr->len++;
    dstr->str[dstr->len] = '\0';
}

const char *dstr_as_str(dynstr *dstr)
{
    return dstr->str;
}


char *dstr_detach(dynstr *dstr)
{
    char *str = dstr->str;
    str = realloc(str, (dstr->len+1));
    FREE(dstr);
    return str;
}

