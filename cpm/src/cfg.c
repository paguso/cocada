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
#include "pkg.h"
#include "toml.h"


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
        strbuf_nappend(dest, buf, nread);
    }
    FREE(buf);
    strfilereader_close(sfr);
}


void pkg_write_to_cfg(FILE *stream, env *e, const pkg *p)
{
    fprintf(stream, "[package]\n");
    fprintf(stream, "%s = \"%s\"\n", TAG_NAME, pkg_get_name(p));
    fprintf(stream, "%s = \"%s\"\n", TAG_TYPE, pkg_type_str[pkg_get_type(p)]);
    char buf[100];
    semver_to_str(pkg_get_version(p), buf);
    fprintf(stream, "%s = \"%s\"\n", TAG_VERSION, buf);
    if (pkg_get_author(p)) {
        fprintf(stream, "%s = \"%s\"\n", TAG_AUTHOR, pkg_get_author(p));
    }
    if (pkg_get_e_mail(p)) {
        fprintf(stream, "%s = \"%s\"\n", TAG_EMAIL, pkg_get_e_mail(p));
    }
    
}


#define READ_TOML_REQ_DATUM(SERVER, DATUM, TAG, TYPE) \
    toml_datum_t DATUM = toml_##TYPE##_in(SERVER, TAG);\
    if (! DATUM.ok ) {\
        strbuf_append(errbuf, "Error parsing config file "CONFIG_FILE".");\
        strbuf_append(errbuf, "Missing ["TAG"] tag");\
        goto cleanup;\
    }


#define READ_TOML_REQ_ARRAY(SERVER, ARR, TAG) \
    toml_array_t *ARR = toml_array_in(SERVER, TAG);\
    if (! ARR ) {\
        strbuf_append(errbuf, "Error parsing config file "CONFIG_FILE".");\
        strbuf_append(errbuf, "Missing ["TAG"] array tag");\
        goto cleanup;\
    }


pkg *pkg_read_from_cfg(FILE *stream, env *e, strbuf *errbuf)
{
    toml_table_t *tbl = NULL;
    char tomlerrbuf[256];
    tbl = toml_parse_file(stream, tomlerrbuf, 256);
    if (!tbl) {
        strbuf_append(errbuf, "Cannot parse config file "CONFIG_FILE".");
        strbuf_append(errbuf, tomlerrbuf);
        goto cleanup;
    }

    toml_table_t *compiler = toml_table_in(tbl, "compiler");
    if (!compiler) {
        strbuf_append(errbuf, "Error parsing config file "CONFIG_FILE".");
        strbuf_append(errbuf, "Missing [compiler] tag");
        goto cleanup;
    }

    READ_TOML_REQ_DATUM(compiler, debug_compiler, TAG_DEBUG_COMPILER, string);
    READ_TOML_REQ_ARRAY(compiler, debug_args, TAG_DEBUG_ARGS);
    READ_TOML_REQ_DATUM(compiler, release_compiler, TAG_RELEASE_COMPILER, string);
    READ_TOML_REQ_ARRAY(compiler, release_args, TAG_RELEASE_ARGS);

    toml_table_t *package = toml_table_in(tbl, "package");
    if (!package) {
        strbuf_append(errbuf, "Error parsing config file "CONFIG_FILE".\n");
        strbuf_append(errbuf, "Missing [package] tag");
        goto cleanup;
    }

    READ_TOML_REQ_DATUM(package, name, TAG_NAME, string);
    if ( ! is_valid_pkg_name(name.u.s) ) {
        strbuf_append(errbuf, "Error parsing config file "CONFIG_FILE".\n");
        strbuf_append(errbuf, "Invalid package name '");
        strbuf_append(errbuf, name.u.s);
        strbuf_append(errbuf, "'.");
        goto cleanup;
    }

    READ_TOML_REQ_DATUM(package, type, TAG_TYPE, string);

    READ_TOML_REQ_DATUM(package, version, TAG_VERSION, string);
    semver_res svres = semver_new_from_str(version.u.s);
    if ( ! svres.ok ) {
        strbuf_append(errbuf, "Error parsing config file "CONFIG_FILE".\n");
        strbuf_append(errbuf, "Invalid version ");
        strbuf_append(errbuf, version.u.s);
        goto cleanup;
    }

    pkg *ret = pkg_new(cstr_clone(name.u.s), TYPE_FROM_LBL(type.u.s), svres.res, cstr_clone(e->cwd));

    toml_datum_t author = toml_string_in(package, TAG_AUTHOR);
    if (author.ok) {
        pkg_set_author(ret, cstr_clone(author.u.s));
    }

    toml_datum_t email = toml_string_in(package, TAG_EMAIL);
    if (email.ok) {
        pkg_set_e_mail(ret, cstr_clone(email.u.s));
    }

cleanup:
    FREE(name.u.s);
    FREE(type.u.s);
    FREE(version.u.s);
    FREE(author.u.s);
    FREE(email.u.s);
    FREE(debug_compiler.u.s);
    FREE(release_compiler.u.s);
    return ret;
}

