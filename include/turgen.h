#ifndef TURGEN
#define TURGEN

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>

#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <pwd.h>
#include <poll.h>
#include <libgen.h>
#include <dirent.h>
#include <glob.h>
#include <fcntl.h>


#define shellname "turgen"


extern struct termios Terminal;

#endif
