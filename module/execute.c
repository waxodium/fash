#include "turgen.h"

#include "sout.h"
#include "terminal.h"
#include "globs.h"

#include "execute.h"

// Parser
#include "quote.h"



int total(void) {
    return sizeof(builtins) / sizeof(Command);
}

void execute(char *buffer, ShellState *state) {
    char commandCopy[4096];
    strncpy(commandCopy, buffer, sizeof(commandCopy) - 1);
    commandCopy[sizeof(commandCopy) - 1] = '\0';

    char *argv[1024];
    int argc = 0;
    char *token = strtok(commandCopy, " ");

    while (token && argc < 1023) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    for (int i = 0; i < argc; i++) {
        strip(argv[i]);
    }

    if (argc == 0) return;

    char *command = argv[0];
    bool pathTarget = (strchr(command, '/') != NULL) || 
                  (command[0] == '~') || 
                  (strlen(command) > 0 && command[strlen(command) - 1] == '/') || 
                  (strcmp(command, "..") == 0);

    if (pathTarget) {
        char *cdArgv[] = {"cd", command, NULL};
        for (int i = 0; i < total(); i++) {
            if (strcmp(builtins[i].name, "cd") == 0) {
                builtins[i].func(cdArgv, state);
                return;
            }
        }
    }

    for (int i = 0; i < total(); i++) {
        if (strcmp(command, builtins[i].name) == 0) {
            builtins[i].func(argv, state);
            return;
        }
    }



    char *finalArgv[1024];
    int finalArgc = 0;

    for (int i = 0; i < argc; i++) {
        if (strchr(argv[i], '*') || strchr(argv[i], '?')) {
            GlobResult result = globbing(argv[i]);
            for (int j = 0; j < result.count && finalArgc < 1023; j++) {
                finalArgv[finalArgc++] = result.paths[j];
            }
        } else {
            finalArgv[finalArgc++] = argv[i];
        }
    }
    finalArgv[finalArgc] = NULL;

    disableRaw(&Terminal);

    pid_t pid = fork();
    if (pid < 0) {
        enableRaw(&Terminal);
        return;
    }

    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);

        execvp(finalArgv[0], finalArgv);

        if (errno == ENOENT) {
            sout("\r%s: %s: command not found\r\n", shellname, finalArgv[0]);
        }
        exit(1);
    }

    wait(NULL);
    enableRaw(&Terminal);
}
