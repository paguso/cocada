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


toolkit linux_gcc_toolkit = {
    .name = "linux-gcc",
    .compiler = "gcc -c",
    .linker = "gcc"
};

toolkit *default_toolkit = &linux_gcc_toolkit;


static bool dir_exists(const char *path)
{   
    struct stat statbuf;
    return ((stat(path, &statbuf) == 0) && S_ISDIR(statbuf.st_mode));
}


static void init_home(env *e, env_res *result)
{
    char *cp;
    if ( (cp = getenv(HOME_ENV_VAR)) ) {
        e->home = cstr_reassign((char *)e->home, cp);
    }
    else {
        e->home = cstr_reassign((char *)e->home, "");
        WARN("Undefined HOME dir. Setting to empty \"\".\n");
    }
    result->ok = true;
}


static void init_cocada_paths(env *e, env_res *result)
{
    char *buf;
    if ( (buf = getenv(COCADA_PATH_ENV_VAR)) ) {
        e->cocada_path = cstr_reassign((char *)e->cocada_path, buf);
    }
    else  { // try default cocada path
        WARN("Undefined "COCADA_PATH_ENV_VAR" environment variable.\n"
             "Setting cocada dir to default value '%s'.\n",e->cocada_path);
        buf = cstr_join(DIR_SEP, 2, e->home, DEFAULT_COCADA_DIR);
        e->cocada_path = cstr_reassign((char *)e->cocada_path, buf);
        FREE(buf);
    }
    //buf = cstr_join(DIR_SEP, 2, e->cocada_path, CPM_DIR);
    //e->cpm_path = cstr_reassign(e->cpm_path, buf);
    //FREE(buf);
    buf = cstr_join(DIR_SEP, 2, e->cocada_path, RESOURCES_DIR);
    e->cpm_resources_path = cstr_reassign((char *)e->cpm_resources_path, buf);
    FREE(buf);
    if (!dir_exists(e->cocada_path)) {
        snprintf(result->res.err.msg, ENV_ERR_BUF_SIZE, "Unable to open COCADA_PATH dir '%s'\n", e->cocada_path);
        ERROR("%s\n",result->res.err.msg);
        result->ok = false;
        return;
    }
    /* 
    if (!dir_exists(e->cpm_path)) {
        snprintf(result->res.err.msg, ENV_ERR_BUF_SIZE, "Unable to open CPM_PATH '%s'\n", e->cpm_path);
        ERROR("%s\n",result->res.err.msg);
        result->ok = false;
        goto FAIL;
    } 
    */
    result->ok = true;
    result->res.ok = e;
}


static void init_cwd(env *e, env_res *result)
{
    errno = 0;
    char *buf = getcwd(NULL, 0);
    if (!buf) {
        snprintf(result->res.err.msg, ENV_ERR_BUF_SIZE, "Unable to determine current working dir.\n");
        perror("getcwd");
        result->ok = false;
        return;
    }
    e->cwd = cstr_reassign((char *)e->cwd, buf);
    result->ok = true;
    result->res.ok = e;
    free(buf);
}


env_res env_init(env *e)
{
    env_res result;
    init_home(e, &result);
    if (!result.ok) return result;
    init_cocada_paths(e, &result);
    if (!result.ok) return result;
    init_cwd(e, &result);
    e->tkt = default_toolkit;
    return result;
}


env *env_new()
{
    env *ret = NEW(env);
    ret->home = NULL;
    ret->cocada_path = NULL;
    //ret->cpm_path = NULL;
    ret->cpm_resources_path = NULL;
    ret->cwd = NULL;
    return ret;
}


void env_free(env *e)
{
    if (e == NULL) return;
    FREE(e->home);
    FREE(e->cwd);
    FREE(e->cocada_path);
    //FREE(e->cpm_path);
    FREE(e->cpm_resources_path);
    FREE(e);
}

