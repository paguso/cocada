#ifndef PKG_H
#define PKG_H

#include "vec.h"


#define SRC_DIR "src"
#define BUILD_DIR "build"
#define ERRBUF_SIZE 200
#define MAIN_BIN_C  "main.c"
#define MAIN_LIB_C  "lib.c"
#define MAIN_LIB_H  "lib.h"


typedef struct {
    int major;
    int minor;
    int patch;
    //char *pre_rel;
    //char *build;
} pkg_ver;


pkg_ver parse_version(const char *vstr);


typedef struct {
    char *name;
    char *local_repo;
    char *remote_repo;
} srcdep;


typedef enum {
    BIN_PKG,
    LIB_PKG
} pkg_type_t;

#define TYPE_LBL(i) ((i==BIN_PKG)?"bin":(i==LIB_PKG)?"lib":"")


typedef struct {
    const char *name;
    pkg_type_t type;
    const char *root_path;
    const char *src_path;
    vec *src_deps;
    vec *lib_deps;
} pkg;


pkg *pkg_new(const char *name, pkg_type_t type, const char *base_dir);

void pkg_free(pkg *p);

void pkg_set_name(pkg *p, const char *name);

void pkg_set_type(pkg *p, pkg_type_t type);

void pkg_set_root_path(pkg *p, const char *path);

void pkg_set_src_path(pkg *p, const char *path);


#endif