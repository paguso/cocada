#ifndef PKG_H
#define PKG_H

#include "new.h"
#include "semver.h"
#include "toml.h"
#include "vec.h"


#define SRC_DIR "src"
#define BUILD_DIR "build"
#define ERRBUF_SIZE 200
#define MAIN_BIN_C  "main.c"
#define MAIN_LIB_C  "lib.c"
#define MAIN_LIB_H  "lib.h"



#define TYPE_LBL(i) ((i==BIN_PKG)?"bin":(i==LIB_PKG)?"lib":"")

#define TYPE_FROM_LBL(LBL) ( (strncmp(LBL, "lib", 3)==0) ? LIB_PKG : BIN_PKG )

#define DEFAULT_NAME "untitled"
#define DEFAULT_TYPE BIN_PKG
#define DEFAULT_VERSION "0.1.0"
#define DEFAULT_COMPILER_CMD "gcc"
#define DEFAULT_DEBUG_ARGS "\"-W\", \"all\", \"-g\", \"3\", \"-D\", \"DEBUG_LVL=3\" ,\"-D\", \"MEM_DEBUG\""
#define DEFAULT_RELEASE_ARGS "\"-O\", \"3\", \"-D\", \"DEBUG_LVL=1\""
#define DEFAULT_AUTHOR ""
#define DEFAULT_EMAIL ""


typedef enum {
    BIN_PKG,
    LIB_PKG
} pkg_type_t;

extern const char* const pkg_type_str[];// = {"bin", "lib"};

typedef struct pkg pkg;

typedef struct tgt tgt;


bool is_valid_pkg_name(const char *str);

pkg *pkg_new(char *name, pkg_type_t type, semver *version, char *root_dir);

void pkg_free(pkg *p);

const char *pkg_get_name(const pkg *p);

void pkg_set_name(pkg *p, char *name);

pkg_type_t pkg_get_type(const pkg *p);

void pkg_set_type(pkg *p, pkg_type_t type);

const semver *pkg_get_version(const pkg *p);

void pkg_set_version(pkg *p, semver *ver);

const char *pkg_get_author(const pkg *p);

void pkg_set_author(pkg *p, char *author);

const char *pkg_get_e_mail(const pkg *p);

void pkg_set_e_mail(pkg *p, char *e_mail);

const char *pkg_get_root_path(const pkg *p);

void pkg_set_root_path(pkg *p, char *path);

const char *pkg_get_src_path(const pkg *p);

const char *pkg_get_cfg_file_path(const pkg *p);

void pkg_add_target(pkg *p, tgt *t);

const vec *pkg_get_targets(pkg *p);




typedef enum {
    TGT_BIN,
    TGT_LIB,
} tgt_type_t;

extern const char* const tgt_type_str[];// = {"bin", "lib", "test", "doc"};


typedef enum {
    TGT_LIB_STATIC,
    TGT_LIB_DYNAMIC
} tgt_lib_type_t;

extern const char* const tgt_lib_type_str[];// = {"debug", "release"};


typedef enum {
    TGT_DEBUG,
    TGT_RELEASE
} tgt_mode_t;

extern const char* const tgt_mode_str[];// = {"debug", "release"};


bool is_valid_tgt_name(const char *str);


void tgt_finalise(void *ptr, const finaliser *fnr);

const char *tgt_get_name(tgt *t);

void tgt_set_name(tgt *t, char *name);

tgt_type_t tgt_get_type(tgt *t);

void tgt_set_type(tgt *t, tgt_type_t typ);

tgt_mode_t tgt_get_mode(tgt *t);

void tgt_set_mode(tgt *t, tgt_mode_t mode);

const char *tgt_get_output_path(tgt *t);

void tgt_set_output_path(tgt *t, char *path); 


#endif