#include "turgen.h"
#include <fcntl.h>
#include <unistd.h>
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
    if (buffer == NULL || strlen(buffer) == 0) return;

    char copy[4096];
    strncpy(copy, buffer, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char **argv = tokenize(copy);
    
    // debug to see token 
    for(int i = 0; argv[i] != NULL; i++)
    {
        printf("[%s]\n", argv[i]);
    }
    // debug end
    
    if (!argv) return;

    int argc = 0;
    while (argv[argc] != NULL) {
        argc++;
    }

    if (argc == 0) {
        free(argv);
        return;
    }

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
                free(argv);
                return;
            }
        }
    }

    for (int i = 0; i < total(); i++) {
        if (strcmp(command, builtins[i].name) == 0) {
            builtins[i].func(argv, state);
            free(argv);
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
            if (finalArgc < 1023) {
                finalArgv[finalArgc++] = argv[i];
            }
        }
    }
    finalArgv[finalArgc] = NULL;

    disableRaw(&Terminal);

    pid_t pid = fork();
    if (pid < 0) {
        free(argv); 
        enableRaw(&Terminal);
        return;
    }

    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        for (int i = 0; finalArgv[i] != NULL; i++)
        {
            if (strcmp(finalArgv[i], ">") == 0)
            {
                int fd = open(finalArgv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                {
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);

                finalArgv[i] = NULL;
                break;
            }

            else if (strcmp(finalArgv[i], ">>") == 0)
            {
                int fd = open(finalArgv[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);

                if (fd < 0)
                {
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);

                finalArgv[i] = NULL;
                break;
            }

            else if (strcmp(finalArgv[i], "<") == 0)
            {
                int fd = open(finalArgv[i + 1], O_RDONLY);

                if (fd < 0)
                {
                    exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);

                finalArgv[i] = NULL;
                break;
            }
        }

        execvp(finalArgv[0], finalArgv);

        if (errno == ENOENT) {
            sout("\r%s: %s: command not found\r\n", shellname, finalArgv[0]);
        }
        exit(1);
    }

    wait(NULL);
    enableRaw(&Terminal);

    free(argv);
}
