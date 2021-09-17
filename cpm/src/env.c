#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cstrutil.h"
#include "errlog.h"
#include "errno.h"
#include "new.h"
#include "strbuf.h"
#include "env.h"

static void init_home(env *e)
{
    char *cp;
    if ( (cp = getenv(HOME_ENV_VAR)) ) {
        e->home = cstr_reassign(e->cocada_path, cp);
    }
    else {
        e->home = cstr_reassign(e->cocada_path, "");
        WARN("Undefined HOME dir. Setting to empty \"\".\n");
    }
}


static void init_cocada_paths(env *e)
{
    char *buf;
    if ( (buf = getenv(COCADA_PATH_ENV_VAR)) ) {
        e->cocada_path = cstr_reassign(e->cocada_path, buf);
    }
    else  {
        buf = cstr_join(DIR_SEP, 2, e->home, DEFAULT_COCADA_DIR);
        e->cocada_path = cstr_reassign(e->cocada_path, buf);
        FREE(buf);
        WARN("Undefined "COCADA_PATH_ENV_VAR" environment variable.\n"
             "Setting cocada dir to default value '%s'.\n",e->cocada_path);
    }
    buf = cstr_join(DIR_SEP, 2, e->cocada_path, CPM_DIR);
    e->cpm_path = cstr_reassign(e->cpm_path, buf);
    FREE(buf);
    buf = cstr_join(DIR_SEP, 2, e->cocada_path, RESOURCES_DIR);
    e->cpm_resources_path = cstr_reassign(e->cpm_resources_path, buf);
    FREE(buf);
}


static void init_cwd(env *e)
{
    errno = 0;
    char *buf = getcwd(NULL, 0);
    if (buf == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    e->cwd = cstr_reassign(e->cwd, buf);
    free(buf);
}


void env_init(env *e)
{
    init_home(e);
    init_cocada_paths(e);    
    init_cwd(e);
}


env *env_new()
{
    env *ret = NEW(env);
    ret->home = NULL;
    ret->cocada_path = NULL;
    ret->cpm_path = NULL;
    ret->cpm_resources_path = NULL;
    ret->cwd = NULL;
    env_init(ret);
    return ret;
}


void env_free(env *e)
{
    if (e == NULL) return;
    FREE(e->home);
    FREE(e->cwd);
    FREE(e->cocada_path);
    FREE(e->cpm_path);
    FREE(e->cpm_resources_path);
    FREE(e);
}

