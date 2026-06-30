#include "turgen.h"

#include "sout.h"
#include "terminal.h"
#include "globs.h"
#include "execute.h"

#include "operator.h"
#include "redirect.h"
#include "paths.h"
#include "seperator.h"

static void external(char **final);

int total(void) {
    return sizeof(builtins) / sizeof(Command);
}

static bool routing(char **argv, int argc, ShellState *state) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "|") == 0 ||strpbrk(argv[i], "<>") != NULL) {
            redirect(argv, argc, state);
            return true;
        }
    }

    return false;
}

static bool semicolon(char **argv, int argc, ShellState *state) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], ";") == 0) {
            seperate(argv, argc, state);
            return true;
        }
    }

    return false;
}

static bool internal(char *command, char **argv, ShellState *state) {
    struct stat info;

    if (strcmp(command, "..") == 0 ||strchr(command, '/') != NULL) {
        
        if (stat(command, &info) == 0 && S_ISDIR(info.st_mode)) {
            char path[1024];

            if (command[0] != '/' && command[0] != '.' && command[0] != '~')
            {
                joinPath(path, sizeof(path), ".", command);
            } else {
                strncpy(path, command, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
            }

            char *args[] = {
                "cd",
                path,
                NULL
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
            final[count++] = argv[i];
        }
    }

    final[count] = NULL;

    return count;
}

static int run_command(char **argv, int argc, ShellState *state) {
    if (argc == 0) return 0;

    if (internal(argv[0], argv, state))
        return 0;

    char *final[1024];
    expanding(argv, argc, final);
    external(final);

    return 0;
}


static bool logical(char **argv, int argc, ShellState *state) {
    for (int i = 0; i < argc; i++) {
        
        if (strcmp(argv[i], "&&") == 0 || strcmp(argv[i], "||") == 0)
        {
            char *left[1024];
            char *right[1024];
            int l = 0;
            int r = 0;

            for (int j = 0; j < i; j++)
                left[l++] = argv[j];

            left[l] = NULL;


            for (int j = i + 1; j < argc; j++)
                right[r++] = argv[j];

            right[r] = NULL;



            /*
             * left side
             */
            bool success = false;
            if (internal(left[0], left, state)) {
                success = true;
            } else {
                char *final[1024];
                expanding(left, l, final);
                external(final);
                success = true;
            }




            if (strcmp(argv[i], "&&") == 0)
            {
                if (success)
                    run_command(right, r, state);
            }
            
            else if (strcmp(argv[i], "||") == 0)
            {
                if (!success)
                    run_command(right, r, state);
            }


            return true;
        }
    }

    return false;
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
    if (!buffer || *buffer == '\0')
        return;

    char **argv = tokenize(buffer);

    if (!argv) return;
    int argc = 0;

    while (argv[argc]) {
        argc++;
    }

    if (argc == 0) {
        free(argv);
        return;
    }

    if (routing(argv, argc, state)) {
        free(argv);
        return;
    }

    if (semicolon(argv, argc, state)) {
        free(argv);
        return;
    }

    if (logical(argv, argc, state)) {
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
