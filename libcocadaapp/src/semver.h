#ifndef SEMVER_H
#define SEMVER_H

#include "result.h"

typedef struct {
    int major;
    int minor;
    int patch;
    char *pre_rel;
    char *build;
} semver;


DECL_RESULT(semver, semver*);

semver_res semver_new_from_str(const char *src);

void semver_free(semver *sver);

void semver_to_str(const semver *src, char *dest);




#endif