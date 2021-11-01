#include <string.h>

#include "cfg.h"
#include "cstrutil.h"
#include "env.h"
#include "new.h"
#include "pkg.h"
#include "semver.h"
#include "strbuf.h"


const char* const pkg_type_str[] = {"bin", "lib"};

const char* const tgt_type_str[] = {"bin", "lib", "test", "doc"};


#define IS_LETTER(C) ( ('A' <= C && C <= 'Z') || ( 'a' <= C && C <= 'z') )
#define IS_NUM(C) ( '0' <= C && C <= '9' )


struct tgt {
    char *name;
    tgt_type_t type;
    char *build_path;
    char *dbg_build_cmd;
    vec  *dbg_build_args;
    char *rel_build_cmd;
    vec  *rel_build_args;
};


bool is_valid_tgt_name(const char *str)
{
    size_t n = strlen(str);
    if ( n == 0 ) return false;
    if ( !IS_LETTER(str[0]) ) return false;
    for (char *c = (char *)str; *c != '\0'; c++) {
        if (!IS_LETTER(*c) && !IS_NUM(*c) && (*c != '_') ) return false;
    }
    return true;
}


tgt *tgt_new(char *name, tgt_type_t type)
{
    tgt *ret = NEW(tgt);
    ret->name = name;
    ret->type = type;
    ret->build_path = NULL;
    ret->dbg_build_cmd = NULL;
    ret->dbg_build_args = vec_new(sizeof(char *));
    ret->rel_build_cmd = NULL;
    ret->rel_build_args = vec_new(sizeof(char *));
    return ret;
}


void tgt_finalise(void *ptr, const finaliser *fnr)
{
    tgt *t = (tgt *)ptr;
    FREE(t->name);
    FREE(t->build_path);
    FREE(t->dbg_build_cmd);
    FREE(t->rel_build_cmd);
    DESTROY(t->dbg_build_args, finaliser_cons(FNR(vec), finaliser_new_ptr()));
    DESTROY(t->rel_build_args, finaliser_cons(FNR(vec), finaliser_new_ptr()));
}


const char *tgt_get_name(tgt *t)
{
    return t->name;
}


void tgt_set_name(tgt *t, char *name) 
{
    FREE(t->name);
    t->name = name;
}


tgt_type_t tgt_get_type(tgt *t)
{
    return t->type;
}


void tgt_set_type(tgt *t, tgt_type_t typ)
{
    t->type = typ;
}


const char *tgt_get_build_path(tgt *t)
{
    return t->build_path;
}


void tgt_set_build_path(tgt *t, char *path) 
{
    FREE(t->build_path);
    t->build_path = path;
}


const char *tgt_get_dbg_build_cmd(tgt *t)
{
    return t->dbg_build_cmd;
}


void tgt_set_dbg_build_cmd(tgt *t, char *cmd) 
{
    FREE(t->dbg_build_cmd);
    t->dbg_build_cmd = cmd;
}


const char *tgt_get_rel_build_cmd(tgt *t)
{
    return t->rel_build_cmd;
}


void tgt_set_rel_build_cmd(tgt *t, char *cmd) 
{
    FREE(t->rel_build_cmd);
    t->rel_build_cmd = cmd;
}


void tgt_add_dbg_build_arg(tgt *t, char *arg)
{
    vec_push_cstr(t->dbg_build_args, arg);
}


const vec *tgt_get_dbg_build_args(tgt *t)
{
    return t->dbg_build_args;
}


void tgt_add_rel_build_arg(tgt *t, char *arg)
{
    vec_push_cstr(t->rel_build_args, arg);
}


const vec *tgt_get_rel_build_args(tgt *t)
{
    return t->rel_build_args;
}


struct _pkg {
    char *name;
    pkg_type_t type;
    semver *version;
    char *author;
    char *e_mail;
    char *root_path;
    char *cfg_file_path;
    char *src_path;
    char *build_path;
    vec  *targets;
    /*char *debug_compiler;
    vec *debug_args;
    char *release_compiler;
    vec *release_args;*/
};


bool is_valid_pkg_name(const char *str)
{
    size_t n = strlen(str);
    if ( n == 0 ) return false;
    if ( !IS_LETTER(str[0]) ) return false;
    for (char *c = (char *)str; *c != '\0'; c++) {
        if (!IS_LETTER(*c) && !IS_NUM(*c) && (*c != '_') ) return false;
    }
    return true;
}


pkg *pkg_new(char *name,  pkg_type_t type, semver *ver, char *root_dir)
{
    pkg *ret = NEW(pkg);
    ret->name = NULL;
    ret->root_path = NULL;
    ret->src_path = NULL;
    ret->cfg_file_path = NULL;
    ret->type = BIN_PKG;
    ret->author = NULL;
    ret->e_mail = NULL;
    ret->targets = vec_new(sizeof(tgt *));

    pkg_set_name(ret, name);
    pkg_set_type(ret, type);
    pkg_set_version(ret, ver);

    pkg_set_root_path(ret, cstr_join(DIR_SEP, 2, root_dir, name));

    FREE(root_dir);
    return ret;
}


void pkg_free(pkg *p) 
{
    if (p == NULL) return;
    FREE(p->name);
    semver_free(p->version);
    FREE(p->root_path);
    FREE(p->src_path);
    FREE(p->build_path);
    FREE(p->cfg_file_path);
    FREE(p->author);
    FREE(p->e_mail);
    DESTROY(p->targets, finaliser_cons(FNR(vec), finaliser_new_ptr_to_obj(FNR(tgt))));

    /*
    FREE(p->debug_compiler);
    FREE(p->release_compiler);
    DESTROY(p->debug_args, finaliser_cons(FNR(vec), finaliser_new_ptr()));
    DESTROY(p->release_args, finaliser_cons(FNR(vec), finaliser_new_ptr()));
    */
    FREE(p);
}


const char *pkg_get_name(const pkg *p)
{
    return p->name;
}


void pkg_set_name(pkg *p, char *name)
{
    FREE(p->name);
    p->name = name;
}


pkg_type_t pkg_get_type(const pkg *p) {
    return p->type;
}


void pkg_set_type(pkg *p, pkg_type_t type)
{
    p->type = type;
}


const semver *pkg_get_version(const pkg *p) 
{
    return p->version;
}


void pkg_set_version(pkg *p, semver *ver)
{
    p->version = ver;
}


const char *pkg_get_author(const pkg *p)
{
    return p->author;
}


void pkg_set_author(pkg *p, char *author)
{
    FREE(p->author);
    p->author = author;
}


const char *pkg_get_e_mail(const pkg *p)
{
    return p->e_mail;
}


void pkg_set_e_mail(pkg *p, char *e_mail)
{
    FREE(p->e_mail);
    p->e_mail = e_mail;
}


const char *pkg_get_root_path(const pkg *p)
{
    return p->root_path;
}


void pkg_set_root_path(pkg *p, char *path)
{
    FREE(p->root_path);
    p->root_path = path;
    FREE(p->src_path);
    p->src_path = cstr_join(DIR_SEP, 2, p->root_path, SRC_DIR);
    p->build_path = cstr_join(DIR_SEP, 2, p->root_path, BUILD_DIR);
    p->cfg_file_path = cstr_join(DIR_SEP, 2, p->root_path, CONFIG_FILE);
}


const char *pkg_get_src_path(const pkg *p)
{
    return p->src_path;
}


const char *pkg_get_build_path(const pkg *p)
{
    return p->build_path;
}


const char *pkg_get_cfg_file_path(const pkg *p)
{
    return p->cfg_file_path;
}


void pkg_add_target(pkg *p, tgt *t)
{
    vec_push_rawptr(p->targets, t);    
}


const vec *pkg_get_targets(pkg *p)
{
    return p->targets;
}



/*
const char *pkg_get_debug_compiler(pkg *p)
{
    return p->debug_compiler;
}


void pkg_set_debug_compiler(pkg *p, char *cmd)
{
    FREE(p->debug_compiler);
    p->debug_compiler = cmd;
}


const vec *pkg_get_debug_args(pkg *p)
{
    return p->debug_args;
}


void pkg_add_debug_arg(pkg *p, char *arg)
{
    vec_push_cstr(p->debug_args, arg);
}


const char *pkg_get_release_compiler(pkg *p)
{
    return p->release_compiler;
}


void pkg_set_release_compiler(pkg *p, char *cmd)
{
    FREE(p->release_compiler);
    p->release_compiler = cmd;
}


const vec *pkg_get_release_args(pkg *p)
{
    return p->release_args;
}


void pkg_add_release_arg(pkg *p, char *arg)
{
    vec_push_cstr(p->release_args, arg);
}
*/