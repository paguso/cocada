#ifndef SEMVER_H
#define SEMVER_H

typedef struct {
    int major;
    int minor;
    int patch;
    char *pre_rel;
    char *build;
} semver;


int semver_from_str(semver *dest, const char *src);



#endif