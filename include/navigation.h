#include <termios.h>

#ifndef navigation
#define navigation

extern struct termios Terminal;

int fclear(char **argv);
int fexit(char **argv);
int directory(const char *path);
int cd(char **argv);

#endif
