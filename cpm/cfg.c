#include <assert.h>
#include <errno.h>
#include <string.h>

#include "cli.h"
#include "cstrutil.h"
#include "errlog.h"
#include "strbuf.h"
#include "strread.h"
#include "strfilereader.h"
#include "vec.h"

#include "env.h"
#include "cfg.h"



static const char *CFG = 
"[environment]\n"
"compiler = \"{compiler}\"\n"
"debug_flags = \"{debug_flags}\"\n"
"release_flags = \"{release_flags}\"\n"
"\n"
"[package]\n"
"name = \"{name}\"\n"
"type = \"{type}\"\n"
"version = \"{version}\"\n"
"author = \"{author}\"\n"
"e-mail = \"{e-mail}\"\n";


#define BUFSIZE 200
void read_file_to_strbuf(strbuf *dest, const char *path)
{
/*
    strfilereader *sfr = strfilereader_open_path(path);
    ERROR_IF(sfr == NULL, "Failed to read '%s' (%s).\n", path, strerror(errno));
    strread *rdr = strfilereader_as_strread(sfr);
    char c;
    while ((c = strread_getc(rdr)) != EOF) {
        strbuf_append_char(dest, c);        
    }
    strfilereader_close(sfr);
*/
    char *buf = cstr_new(BUFSIZE);
    strfilereader *sfr = strfilereader_open_path(path);
    ERROR_IF(sfr == NULL, "Failed to read '%s' (%s).\n", path, strerror(errno));
    strread *rdr = strfilereader_as_strread(sfr);
    size_t nread;
    while((nread = strread_read_str(rdr, buf, BUFSIZE))) {
        assert(nread <= BUFSIZE);
        buf[nread] = 0;
        strbuf_append(dest, buf, nread);
    }
    FREE(buf);
    strfilereader_close(sfr);
}


strbuf *create_config(env *e, pkg *p)
{
    strbuf *config = strbuf_new();
    char *cfg_template_path = cstr_join(DIR_SEP, 2, e->cpm_resources_path, CONFIG_FILE);
    read_file_to_strbuf(config, cfg_template_path);
    // strbuf_append(config, CFG, strlen(CFG));
    // environment section
    strbuf_replace(config, TAG_COMPILER, DEFAULT_COMPILER);
    strbuf_replace(config, TAG_DEBUG_FLAGS, DEFAULT_DEBUG_FLAGS);
    strbuf_replace(config, TAG_RELEASE_FLAGS, DEFAULT_RELEASE_FLAGS);
    // package section
    strbuf_replace(config, TAG_NAME, p->name);
    strbuf_replace(config, TAG_VERSION, DEFAULT_VERSION);

    FREE(cfg_template_path); 
    return config;    
}
