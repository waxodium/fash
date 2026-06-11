#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sout.h"
#include "terminal.h"
#include "navigation.h"

int fclear(char **argv) {
    (void) **argv;
    write(STDOUT_FILENO, "\033c", 2);
    sout("\033[H\033[J");
    return 0;
}

int fexit(char **argv) {
    (void) **argv;
    sout("exit\r\n");
    disableRaw(&Terminal); 
    exit(0);
}


int cd(char **argv) {
    char *targetDir = argv[1];
    char pathBuffer[4096];
    char *homeDir = getenv("HOME");

    if (targetDir == NULL || strcmp(targetDir, "~") == 0) {
        if (!homeDir) {
            sout("\rfash: cd: HOME not set\r\n");
            return 1;
        }
        targetDir = homeDir;
    } else if (strncmp(targetDir, "~/", 2) == 0) {
        if (!homeDir) {
            sout("\rfash: cd: HOME not set\r\n");
            return 1;
        }
        snprintf(pathBuffer, sizeof(pathBuffer), "%s%s", homeDir, targetDir + 1);
        targetDir = pathBuffer;
    }

    

    if (chdir(targetDir) != 0) {
        sout("\rfash: cd: %s: No such file or directory\r\n", argv[1]);
    }

    return 1;
}

