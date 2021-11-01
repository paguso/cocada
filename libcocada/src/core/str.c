#include <string.h>

#include "cstrutil.h"
#include "str.h"


struct str {
    const char *s;
    size_t len;
};


str str_new(const char *src)
{
    return str_new_len(src, strlen(src));
}


str str_new_len(const char *src, size_t len)
{
    str ret = {};
    ret.len = len;
    ret.s = cstr_clone_len(src, len);
    return ret;
}


str cstr_to_str(char *src)
{
    return cstr_to_str_len(src, strlen(src));
}


str cstr_to_str_len(char *src, size_t len)
{
    return (str){.s = src, .len = len};
}


size_t str_len(str s) {
    return s.len;
}


const char *str_as_cstr(str s) {
    return s.s;
}


void str_finalise(void *ptr, const finaliser *fnr)
{
    FREE((void *)(((str *)ptr)->s));
}