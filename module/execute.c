#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include "navigation.h"
#include "sout.h"
#include "terminal.h"


typedef int (*functionCMD)(char **argv);

typedef struct {
    const char *name;
    functionCMD func;
} StandardCMD;


static StandardCMD CMDlist[] = {
    {"clear", fclear},
    {"cls",   fclear},
    {"exit",  fexit}, 
    {"cd", cd}
};


int listCMD(void) {
    return sizeof(CMDlist) / sizeof(StandardCMD);
}


void execute(char *buffer) {
    char cmd[4096];
    strncpy(cmd, buffer, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';

    char *argv[1024];
    int b = 0;
    char *token = strtok(cmd, " ");
    
    while (token != NULL && b < 1023) {
        argv[b++] = token;
        token = strtok(NULL, " ");
    }
    argv[b] = NULL;

    if (argv[0] == NULL) return;

    
    // cd parser
    if (directory(argv[0])) {
        char *cd_argv[] = {"cd", argv[0], NULL};
        cd(cd_argv);
        return;
    }

    for (int i = 0; i < listCMD(); i++) {
        if (strcmp(argv[0], CMDlist[i].name) == 0) {
            CMDlist[i].func(argv); 
            return;
        }
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
        if (errno == ENOENT) 
            sout("\rfash: %s: command not found\r\n", argv[0]);
        exit(1);

    } else if (pid > 0) {
        wait(NULL);
    }
    
    enableRaw(&Terminal);
}

