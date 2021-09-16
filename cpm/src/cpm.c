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


void new(const cliparser *newclip)
{
    const vec *vtype = cliparser_opt_val_from_shortname(newclip, TYPE_OPT);
    ERROR_ASSERT(vtype && vec_len(vtype), "Undefined package type.\n");
    const char *stype = vec_first_cstr(vtype);
    pkg_type_t type = BIN_PKG;
    switch (stype[0]) {
    case 'l':
        type = LIB_PKG;
        break;
    default:
        type = BIN_PKG;
        break;
    }
    const vec *vname = cliparser_arg_val_from_pos(newclip, 0);
    ERROR_ASSERT(vname && vec_len(vname), "Undefined package name.\n");
    const char *name = vec_first_cstr(vname);
    
    env *e = env_new();   
    pkg *p = pkg_new(cstr_clone(name), type, cstr_clone(e->cwd));
    strbuf *cfg_cont = create_config(e, p);

    // create root dir
    ERROR_IF( mkdir(p->root_path, DIR_PERM), 
            "Unable to create package root dir: %s.\n", strerror(errno) );
    // create src dir
    ERROR_IF( mkdir( p->src_path, DIR_PERM), 
            "Unable to create package src dir: %s.\n", strerror(errno) );
    // create config file
    char *cfg_filename = cstr_join(DIR_SEP, 2, p->root_path, CONFIG_FILE);
    FILE *cfg_file = fopen(cfg_filename, "w");
    ERROR_ASSERT(cfg_file, "Unable to create config file: %s.\n", strerror(errno) );
    fprintf(cfg_file, "%s", strbuf_as_str(cfg_cont));

    env_free(e);
    pkg_free(p);
    strbuf_free(cfg_cont);
    FREE(cfg_filename);
    fclose(cfg_file); 
}


cliparser *init_cliparser()
{
    cliparser *clip = cliparser_new(CPM_CMD, "COCADA Package Manager");
    // New subcommand options
    cliparser *newclip = cliparser_new(NEW_CMD, "Creates a new package.");
    cliparser_add_pos_arg(newclip, cliarg_new("package", "package name", ARG_STR));
    vec *mod_types = vec_new(sizeof(char *));
    vec_push_cstr(mod_types, cstr_clone(BIN_TYPE));
    vec_push_cstr(mod_types, cstr_clone(LIB_TYPE));
    vec *mod_default = vec_new(sizeof(char *));
    vec_push_cstr(mod_default, cstr_clone(BIN_TYPE));
    cliparser_add_option(newclip, 
        cliopt_new( TYPE_OPT, "type", "Module type", OPT_OPTIONAL, OPT_SINGLE, ARG_CHOICE, 1, 1, 
                    mod_types, mod_default ));
    cliparser_add_subcommand(clip, newclip);
    return clip;
}


int main(int argc, char **argv) 
{
    memdbg_reset();
    cliparser *clip = init_cliparser();    
    cliparse_exit_status res = cliparser_parse(clip, argc, argv);
    if ( res != PARSE_SUCC ) {
        ERROR("%s\n", cliparser_parse_status_msg(clip));
    }
    const cliparser *act = cliparser_active_subcommand(clip);
    if (act == NULL) {
        cliparser_print_help(clip);
        goto cleanup;
    }
    else if (!strcmp(cliparser_name(act), NEW_CMD)) {
        new(act);
    }
    else {
        cliparser_print_help(clip);
    }
cleanup:
    DESTROY_FLAT(clip, cliparser);
    memdbg_print_stats(stdout, true);
}