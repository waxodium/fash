#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sout.h"
#include "terminal.h"
#include "navigation.h"

#define MaxDir 10
#define PathMax 4096

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


char dirHistory[MaxDir][PathMax];
int cdCount = 0;
int historyHead = -1; 

void pushDirHistory(const char *path) {
    if (cdCount > 0 && strcmp(dirHistory[historyHead], path) == 0) {
        return;
    }
    
    historyHead = (historyHead + 1) % MaxDir;
    strncpy(dirHistory[historyHead], path, PathMax);
    
    if (cdCount < MaxDir) {
        cdCount++;
    }
}

int directory(const char *InputPath) {
    char path[4096];
    const char *currentPath = InputPath;

    if (InputPath[0] == '~') {
        char *home = getenv("HOME");
        if (home) {
            snprintf(path, sizeof(path), "%s%s", home, InputPath + 1);
            currentPath = path;
        }
    }

    struct stat statbuf;
    if (stat(currentPath, &statbuf) != 0)
        return 0;
    
    return S_ISDIR(statbuf.st_mode);
}

int cd(char **argv) {
    char *targetDir = argv[1];
    char pathBuffer[4096];
    char currentDirBuffer[4096];
    char *homeDir = getenv("HOME");
    char *oldPwd = getenv("OLDPWD");

    if (getcwd(currentDirBuffer, sizeof(currentDirBuffer)) == NULL) {
        sout("\rfash: cd: cannot determine current directory\r\n");
        return 1;
    }

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
    } else if (strcmp(targetDir, "-") == 0) {
        if (!oldPwd) {
            sout("\rfash: cd: OLDPWD not set\r\n");
            return 1;
        }
        sout("%s\r\n", oldPwd);
        targetDir = oldPwd;
    }

    if (chdir(targetDir) != 0) {
        sout("\rfash: cd: %s: No such file or directory\r\n", argv[1]);
        return 1;
    }

    char newDirBuffer[4096];
    if (getcwd(newDirBuffer, sizeof(newDirBuffer)) != NULL) {
        setenv("OLDPWD", currentDirBuffer, 1);
        setenv("PWD", newDirBuffer, 1);
        
        pushDirHistory(newDirBuffer);
    }

    return 1;
}

