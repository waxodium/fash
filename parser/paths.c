#include "turgen.h"

#include "paths.h"


void joinPath(char *destination, size_t size, const char *base, const char *name) {
    size_t baseLength = strlen(base);
    
    if (baseLength > 0 && base[baseLength - 1] == '/') {
        snprintf(destination, size, "%s%s", base, name);

    } else {
        snprintf(destination, size, "%s/%s", base, name);

    }

}
