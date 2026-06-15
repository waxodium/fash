#include "fash.h"

#include "navigation.h"
#include "sout.h"
#include "terminal.h"

typedef int (*Handler)(char **, ShellState *);

typedef struct {
    const char *name;
    Handler func;
} Command;

static Command builtins[] = {
    {"clear", fclear},
    {"cls",   fclear},
    {"exit",  fexit}, 
    {"cd",    cd}
};

int total(void) {
    return sizeof(builtins) / sizeof(Command);
}

void execute(char *buffer, ShellState *state) {
    char copy[4096];
    strncpy(copy, buffer, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *argv[1024];
    int count = 0;
    char *token = strtok(copy, " ");
    
    while (token && count < 1023) {
        argv[count] = token;
        count++;
        token = strtok(NULL, " ");
    }
    argv[count] = NULL;

    if (!argv[0]) return;

    for (int i = 0; i < total(); i++) {
        if (strcmp(argv[0], builtins[i].name) == 0) {
            builtins[i].func(argv, state); 
            return;
        }
    }

    bool path = (argv[0][0] == '/' || argv[0][0] == '~' || strchr(argv[0], '/') || strcmp(argv[0], ".") == 0 || strcmp(argv[0], "..") == 0);
    bool glob = (strchr(argv[0], '*') || strchr(argv[0], '?'));
    bool dir = directory(argv[0]);

    if (path && dir) {
        char *args[] = { "cd", argv[0], NULL};
        cd(args, state);
        return;
    }

    if (glob) {
        char *args[] = { "cd", argv[0], NULL };
        cd(args, state);
        return;
    }

    disableRaw(&Terminal);

    pid_t pid = fork();
    
    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);

        execvp(argv[0], argv);
        
        if (errno == ENOENT) {
            sout("\rfash: %s: command not found\r\n", argv[0]);
        }
        exit(1);
    }

    if (pid > 0) {
        wait(NULL);
    }
    
    enableRaw(&Terminal);
}
