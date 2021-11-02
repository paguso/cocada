#ifndef ENV_H
#define ENV_H

#include "result.h"
#include "strbuf.h"

#define DIR_SEP "/"  
#define DIR_PERM 0755

#define USER_ENV_VAR "USER"
#define HOME_ENV_VAR "HOME"
#define COCADA_PATH_ENV_VAR "COCADA_PATH"
#define DEFAULT_COCADA_DIR  ".cocada"
#define CPM_DIR "cpm"
#define RESOURCES_DIR "resources"


typedef struct toolkit {
    char name[16];
    char compiler[16];
    char linker[16];
} toolkit;

extern toolkit linux_gcc_toolkit;

extern toolkit *default_toolkit;


typedef struct 
{
    const char *home;
    const char *cocada_path;
    //const char *cpm_path;
    const char *cpm_resources_path;
    const char *cwd;
    const toolkit *tkt;
} env;


/**
 * @brief Returns a new unintialised environment. The returned environment has to
 * be initialised via the env_init before use.
 * @see env_init
 */
env *env_new();

#define ENV_ERR_BUF_SIZE 128
typedef struct {
    char msg[ENV_ERR_BUF_SIZE];
} env_error;

void env_free(env *e);

DECL_RESULT_ERR(env, env*, env_error);

env_res env_init(env *e);

#endif