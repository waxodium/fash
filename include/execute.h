#ifndef EXECUTE
#define EXECUTE

#include "navigation.h"
#include "echo.h"

typedef int (*Handler)(char **, ShellState *);

typedef struct {
    const char *name;
    Handler func;
} Command;

static Command builtins[] = {
    {"clear", tclear},
    {"cls",   tclear},
    {"exit",  texit}, 
    {"cd",    cd},
    {"echo", echo}
};

#endif
