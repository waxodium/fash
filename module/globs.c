// globs.c
#include "turgen.h"

typedef struct {
    char **paths;
    int count;
} Glob;

void freeGlob(Glob *result) {
    if (result->paths) {
        for (int i = 0; i < result->count; i++) {
            free(result -> paths[i]);
        }
        free(result -> paths);
        result->paths = NULL;
        result->count = 0;
    }
}

Glob globbing(const char *pattern) {
    Glob result = {NULL, 0};
    glob_t globbuf;

    if (glob(pattern, GLOB_TILDE | GLOB_MARK, NULL, &globbuf) == 0) {
        result.count = (int) globbuf.gl_pathc;
        result.paths = malloc(sizeof(char *) * result.count);
        
        for (int i = 0; i < result.count; i++) {
            result.paths[i] = strdup(globbuf.gl_pathv[i]);
        }
        globfree(&globbuf);
    }
    return result;
}
