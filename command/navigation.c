#include "turgen.h"

#include "sout.h"
#include "tree.h"
#include "render.h"
#include "terminal.h"
#include "navigation.h"

#define MaxDir 10
#define PathMax 4096

int fclear(char **argv, ShellState *state) {
    (void) **argv;
    (void) *state;
    write(STDOUT_FILENO, "\033c", 2);
    sout("\033[H\033[J");
    return 0;
}

int fexit(char **argv, ShellState *state) {
    (void) **argv;
    (void) *state;
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

int cd(char **argv, ShellState *state) {
    char *target = argv[1];
    char path[4096];
    char current[4096];
    char newdir[4096];
    char *home = getenv("HOME");
    char *old = getenv("OLDPWD");

    if (getcwd(current, sizeof(current)) == NULL) {
        sout("\r%s: cd: cannot determine current directory\r\n", shellname);
        return 1;
    }

    if (!target || strcmp(target, "~") == 0) {
        if (!home) {
            sout("\r%s: cd: HOME not set\r\n", shellname);
            return 1;
        }
        target = home;
    }

    if (target && strncmp(target, "~/", 2) == 0) {
        if (!home) {
            sout("\r%s: cd: HOME not set\r\n", shellname);
            return 1;
        }
        snprintf(path, sizeof(path), "%s%s", home, target + 1);
        target = path;
    }

    if (target && strcmp(target, "-") == 0) {
        if (!old) {
            sout("\r%s: cd: OLDPWD not set\r\n", shellname);
            return 1;
        }
        sout("%s\r\n", old);
        target = old;
    }

    glob_t match;
    bool hasglob = (target && (strchr(target, '*') || strchr(target, '?')));
    bool globbed = (hasglob && glob(target, 0, NULL, &match) == 0);

    if (globbed && match.gl_pathc > 1) {
        globfree(&match);
        
        TabTree(state); 
        return 1;
    }

    if (globbed && match.gl_pathc == 1) {
        snprintf(path, sizeof(path), "%s", match.gl_pathv[0]);
        target = path;
        globfree(&match);
    }

    if (chdir(target) != 0) {
        sout("\r%s: cd: %s: No such file or directory\r\n", shellname, argv[1]);
        return 1;
    }

    if (getcwd(newdir, sizeof(newdir)) != NULL) {
        setenv("OLDPWD", current, 1);
        setenv("PWD", newdir, 1);
        pushDirHistory(newdir);
    }

    return 1;
}
