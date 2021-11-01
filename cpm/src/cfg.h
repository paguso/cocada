#ifndef CFG_H
#define CFG_H

#include <stdio.h>

#include "env.h"
#include "pkg.h"
#include "strbuf.h"
#include "toml.h"


#define CONFIG_FILE "cfg.toml"

#define TAG_DEBUG_COMPILER "debug_compiler"
#define TAG_DEBUG_ARGS "debug_args"
#define TAG_RELEASE_COMPILER "release_compiler"
#define TAG_RELEASE_ARGS "release_args"

#define TAG_NAME "name"
#define TAG_TYPE "type"
#define TAG_VERSION "version"
#define TAG_AUTHOR "author"
#define TAG_EMAIL "e-mail"

#define BRACKET(TAG) "{"TAG"}"


pkg *pkg_read_from_cfg(FILE *stream, env *e, strbuf *errbuf);

void pkg_write_to_cfg(FILE *stream, env *e, const pkg *p);


#endif