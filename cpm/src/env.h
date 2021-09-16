#ifndef ENV_H
#define ENV_H

#define DIR_SEP "/"  
#define DIR_PERM 0755

#define USER_ENV_VAR "USER"
#define HOME_ENV_VAR "HOME"
#define COCADA_PATH_ENV_VAR "COCADA_PATH"
#define DEFAULT_COCADA_DIR  ".cocada"
#define CPM_DIR "cpm"
#define RESOURCES_DIR "resources"


#include "strbuf.h"


typedef struct 
{
    const char *home;
    const char *cocada_path;
    const char *cpm_path;
    const char *cpm_resources_path;
    const char *cwd;
} env;


env *env_new();

void env_init(env *e);

void env_free(env *e);

#endif