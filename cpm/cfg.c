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


#define BUFSIZE 255
void read_file_to_strbuf(strbuf *dest, const char *path)
{
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
