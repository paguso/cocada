#ifndef CFG_H
#define CFG_H

#include "env.h"
#include "pkg.h"
#include "strbuf.h"


#define CONFIG_FILE "cfg.toml"


#define TAG_NAME "{name}"
#define TAG_TYPE "{type}"
#define TAG_VERSION "{version}"
#define TAG_AUTHOR "{author}"
#define TAG_EMAIL "{e-mail}"
#define TAG_COMPILER "{compiler}"
#define TAG_DEBUG_FLAGS "{debug_flags}"
#define TAG_RELEASE_FLAGS "{release_flags}"

#define DEFAULT_NAME "untitled"
#define DEFAULT_AUTHOR ""
#define DEFAULT_EMAIL ""
#define DEFAULT_VERSION "0.1"
#define DEFAULT_COMPILER "gcc"
#define DEFAULT_DEBUG_FLAGS "-Wall -g3 -DDEBUG_LVL=3 -DMEM_DEBUG"
#define DEFAULT_RELEASE_FLAGS "-O3 -DDEBUG_LVL=1"

strbuf *create_config(env *e, pkg *p);



#endif