#include <termios.h>

#ifndef navigation
#define navigation


#include "render.h"

int tclear(char **argv, ShellState *state);
int texit(char **argv, ShellState *state);
int directory(const char *path);
int cd(char **argv, ShellState *state);

#endif
