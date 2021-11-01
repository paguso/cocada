#ifndef STR_H
#define STR_H

/**
 * @file str.h
 * @author Paulo Fonseca
 * Unmutable string
 */

#include <stddef.h>

#include "new.h"

typedef struct str str;

str str_new(const char *src);

str str_new_len(const char *src, size_t len);

str cstr_to_str(char *src);

str cstr_to_str_len(char *src, size_t len);

void str_finalise(void *ptr, const finaliser *fnr);

size_t str_len(str s);

const char *str_as_cstr(str s);

#endif