#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "cli.h"
#include "cstrutil.h"
#include "errlog.h"
#include "memdbg.h"
#include "new.h"
#include "strbuf.h"
#include "strread.h"
#include "strfilereader.h"
#include "toml.h"
#include "vec.h"

#include "cfg.h"
#include "env.h"
#include "pkg.h"

#define CPM_CMD  "cpm"
#define NEW_CMD  "new"
#define TYPE_OPT 't'
#define BIN_TYPE "bin"
#define LIB_TYPE "lib"
#define CONFIG_CMD  "configure"
#define BUILD_CMD  "build"


void new(const cliparser *newclip)
{
    env *e = env_new();   
    if (!env_init(e).ok) {
        goto FAIL;
    }
    // package name
    char *pkg_name = vec_first_cstr(cliparser_arg_val_from_pos(newclip, 0));
    if (!is_valid_pkg_name(pkg_name)) {
        ERROR("Invalid package name '%s'\n", pkg_name);
        goto FAIL;
    }
    // package type
    const char *pkg_type_str = vec_first_cstr(cliparser_opt_val_from_shortname(newclip, TYPE_OPT));
    pkg_type_t pkg_type;
    switch (pkg_type_str[0]) {
    case 'b': // "bin"
        pkg_type = LIB_PKG;
        break;
    case 'l': // "lib"
        pkg_type = BIN_PKG;
        break;
    default:
        ERROR("Invalid package type %s.\n", pkg_type_str);
        goto FAIL;
        break;
    }
    // package version
    semver *pkg_version = semver_new_from_str(DEFAULT_VERSION).res;

    pkg *p = pkg_new(cstr_clone(pkg_name), pkg_type,  pkg_version, cstr_clone(e->cwd));

    // create root dir
    if (mkdir(pkg_get_root_path(p), DIR_PERM)) {
        ERROR("Unable to create package root dir: %s.\n", strerror(errno));
        goto FAIL;
    }
    // create src dir
    if (mkdir(pkg_get_src_path(p), DIR_PERM)) {
        ERROR("Unable to create package src dir: %s.\n", strerror(errno));
        goto FAIL;
    }
    // create config file
    FILE *cfg_file = fopen(pkg_get_cfg_file_path(p), "w");
    ERROR_ASSERT(cfg_file, "Unable to create config file: %s.\n", strerror(errno) );
    pkg_write_to_cfg(cfg_file, e, p);

FAIL:
    env_free(e);
}



void configure(const cliparser *configclip)
{
    

}



void build(const cliparser *buildclip) 
{

}



cliparser *init_cliparser()
{
    cliparser *clip = cliparser_new(CPM_CMD, "COCADA Package Manager");
    // New subcommand options
    cliparser *newclip = cliparser_new(NEW_CMD, "Create a new package.");
    cliparser_add_pos_arg(newclip, cliarg_new("package", "package name", ARG_STR));
    vec *mod_types = vec_new(sizeof(char *));
    vec_push_cstr(mod_types, cstr_clone(BIN_TYPE));
    vec_push_cstr(mod_types, cstr_clone(LIB_TYPE));
    vec *mod_default = vec_new(sizeof(char *));
    vec_push_cstr(mod_default, cstr_clone(BIN_TYPE));
    cliparser_add_option(newclip, 
        cliopt_new( TYPE_OPT, "type", "Module type", OPT_OPTIONAL, 
            OPT_SINGLE, ARG_CHOICE, 1, 1, mod_types, mod_default ));
    cliparser_add_subcommand(clip, newclip);
    // Config subconmmand
    cliparser *configclip = cliparser_new(CONFIG_CMD, "Configure current package.");
    cliparser_add_subcommand(clip, configclip);
    // Build subcommand
    cliparser *buildclip = cliparser_new(BUILD_CMD, "Build current package.");
    cliopt *bopt, *dopt;
    cliparser_add_option(buildclip,
        dopt = cliopt_new( 'd', "debug", "debug build", OPT_OPTIONAL, OPT_SINGLE, ARG_NONE, 0, 0, NULL, NULL)); 
    cliparser_add_option(buildclip,
        bopt = cliopt_new( 'r', "release", "release build", OPT_OPTIONAL, OPT_SINGLE, ARG_NONE, 0, 0, NULL, NULL)); 
    cliparser_add_option_combo(buildclip, ONE_IF_ANY, 2, bopt, dopt);
    cliparser_add_subcommand(clip, buildclip);
    return clip;
}


int main(int argc, char **argv) 
{
    memdbg_reset();
    cliparser *clip = init_cliparser();    
    cliparse_res result = cliparser_parse(clip, argc, argv, true);
    if ( !result.ok ) {
        ERROR("%s\n", result.res.err.msg);
        goto cleanup;
    }
    const cliparser *act = cliparser_active_subcommand(clip);
    if (act == NULL) {
        cliparser_print_help(clip);
        goto cleanup;
    }
    const cliopt *sc_opt = cliparser_active_sc_option(clip);
    if (sc_opt && cliopt_shortname(sc_opt) == 'h') {
        cliparser_print_help(act);
        goto cleanup;
    }
    else if (!strcmp(cliparser_name(act), NEW_CMD)) {
        new(act);
    }
    else if (!strcmp(cliparser_name(act), CONFIG_CMD)) {
        configure(act);
    }
    else if (!strcmp(cliparser_name(act), BUILD_CMD)) {
        build(act);
    }
    else {
        cliparser_print_help(clip);
    }
cleanup:
    DESTROY_FLAT(clip, cliparser);
    memdbg_print_stats(stdout, true);
}