#include "turgen.h"

#include "sout.h"
#include "terminal.h"
#include "globs.h"

#include "execute.h"

#include "quote.h"
#include "operator.h"
#include "redirect.h"
#include "paths.h"

int total(void) {
    return sizeof(builtins) / sizeof(Command);
}



static bool routing(char **argv, int argc, ShellState *state) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") == 0 || strpbrk(argv[i], "<>") != NULL) {
            redirect(argv, argc, state);
            return true;
        }
    }
    return false;
}




static bool internal(char *command, char **argv, ShellState *state) {
    struct stat info;

    if (strcmp(command, "..") == 0 || strchr(command, '/') != NULL) {
        if (stat(command, &info) == 0 && S_ISDIR(info.st_mode)) {
            char path[1024];
            
            if (command[0] != '/' && command[0] != '.' && command[0] != '~') {
                joinPath(path, sizeof(path), ".", command);
            } else {
                strncpy(path, command, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
            }

            char *args[] = {
                "cd", path, NULL 
            };

            for (int i = 0; i < total(); i++) {
                if (strcmp(builtins[i].name, "cd") == 0) {
                    builtins[i].func(args, state);
                    return true;
                }
            }
        }
    }

    for (int i = 0; i < total(); i++) {
        if (strcmp(command, builtins[i].name) == 0) {
            builtins[i].func(argv, state);
            return true;
        }
    }

    return false;
}


static int expanding(char **argv, int argc, char **final) {
    int count = 0;
    for (int i = 0; i < argc; i++) {
        if (strchr(argv[i], '*') || strchr(argv[i], '?')) {
            GlobResult match = globbing(argv[i]);
            for (int j = 0; j < match.count && count < 1023; j++) {
                final[count++] = match.paths[j];
            }
        } else {
            if (count < 1023) {
                final[count++] = argv[i];
            }
        }
    }
    final[count] = NULL;
    return count;
}





static void external(char **final) {
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

        execvp(final[0], final);

        if (errno == ENOENT) {
            sout("\r%s: %s: command not found\r\n", shellname, final[0]);
        }
        exit(1);
    }

    wait(NULL);
    enableRaw(&Terminal);
}












void execute(char *buffer, ShellState *state) {
    if (buffer == NULL || strlen(buffer) == 0) return;

    char *padded = opsing(buffer);
    if (!padded) return;

    char **argv = tokenize(padded);
    if (!argv) {
        free(padded);
        return;
    }

    int argc = 0;
    while (argv[argc] != NULL) argc++;

    if (argc == 0) {
        free(argv);
        return;
    }

    if (routing(argv, argc, state)) {
        free(argv);
        return;
    }

    if (internal(argv[0], argv, state)) {
        free(argv);
        return;
    }

    char *final[1024];
    expanding(argv, argc, final);
    external(final);

    free(argv);
}

