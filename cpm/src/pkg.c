#include <string.h>

#include "new.h"
#include "pkg.h"
#include "cstrutil.h"
#include "strbuf.h"
#include "env.h"

pkg_ver parse_version(const char *vstr)
{
    
}






pkg *pkg_new(const char *name,  pkg_type_t type, const char *base_dir)
{
    pkg *ret = NEW(pkg);
    ret->name = NULL;
    ret->root_path = NULL;
    ret->src_path = NULL;
    ret->type = BIN_PKG;

    pkg_set_name(ret, name);
    pkg_set_type(ret, type);
    pkg_set_root_path(ret, cstr_join(DIR_SEP, 2, base_dir, name));
    pkg_set_src_path(ret, cstr_join(DIR_SEP, 2, ret->root_path, SRC_DIR));

    FREE(base_dir);
    return ret;
}


void pkg_free(pkg *p) 
{
    if (p == NULL) return;
    FREE(p->name);
    FREE(p->root_path);
    FREE(p->src_path);
    FREE(p);
}


void pkg_set_name(pkg *p, const char *name)
{
    FREE(p->name);
    p->name = name;
    //p->name = cstr_reassign(p->name, name);
}


void pkg_set_type(pkg *p, pkg_type_t type)
{
    p->type = type;
}


void pkg_set_root_path(pkg *p, const char *path)
{
    FREE(p->root_path);
    p->root_path = path;
    //p->root_path = cstr_reassign(p->root_path, path);
}


void pkg_set_src_path(pkg *p, const char *path)
{
    FREE(p->src_path);
    p->src_path = path;
    //p->src_path = cstr_reassign(p->src_path, path);
}
