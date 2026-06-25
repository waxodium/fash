#include "turgen.h"

#include "render.h"
#include "terminal.h"
#include "sout.h"


/*
    redirect() is a function on handling actual redirection.
    opsing parses and pads to NOT conflict with tokenizer()
    which is the function from quote.c parsing

*/

void redirect(char **argv, int argc, ShellState *state) {
    (void) *state;
    int inputFd = 0; 
    int index = 0;

    while (index < argc) {
        char *cmdArgv[1024];
        int cmdArgc = 0;
        char *inputFile = NULL;
        char *outputFile = NULL;
        int append = 0;
        int piped = 0;

        while (index < argc) {
            if (strcmp(argv[index], "|") == 0) {
                piped = 1;
                index++;
                break;
            } else if (strcmp(argv[index], "<") == 0) {
                inputFile = argv[index + 1];
                index += 2;
            } else if (strcmp(argv[index], ">") == 0) {
                outputFile = argv[index + 1];
                append = 0;
                index += 2;
            } else if (strcmp(argv[index], ">>") == 0) {
                outputFile = argv[index + 1];
                append = 1;
                index += 2;
            } else {
                cmdArgv[cmdArgc++] = argv[index];
                index++;
            }
        }
        cmdArgv[cmdArgc] = NULL;

        if (cmdArgc == 0) continue;

        int pipeFds[2];
        if (piped) {
            if (pipe(pipeFds) < 0) {
                perror("pipe");
                return;
            }
        }

        disableRaw(&Terminal);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            enableRaw(&Terminal);
            return;
        }

        if (pid == 0) {
            struct sigaction sa;
            sa.sa_handler = SIG_DFL;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGINT, &sa, NULL);

            if (inputFd != 0) {
                dup2(inputFd, 0);
                close(inputFd);
            }

            if (piped) {
                dup2(pipeFds[1], 1);
                close(pipeFds[0]);
                close(pipeFds[1]);
            }

            if (inputFile) {
                int fdIn = open(inputFile, O_RDONLY);
                if (fdIn < 0) {
                    perror("open input");
                    exit(1);
                }
                dup2(fdIn, 0);
                close(fdIn);
            }

            if (outputFile) {
                int flags = O_WRONLY | O_CREAT;
                if (append) flags |= O_APPEND;
                else flags |= O_TRUNC;

                int fdOut = open(outputFile, flags, 0644);
                if (fdOut < 0) {
                    perror("open output");
                    exit(1);
                }
                dup2(fdOut, 1);
                close(fdOut);
            }

            execvp(cmdArgv[0], cmdArgv);
            
            if (errno == ENOENT) {
                sout("\r%s: %s: command not found\r\n", shellname, cmdArgv[0]);
            }
            
            exit(1);


        } else {
            if (inputFd != 0) {
                close(inputFd);
            }

            if (piped) {
                close(pipeFds[1]);
                inputFd = pipeFds[0];
            } else {
                inputFd = 0;
            }

            int status;
            waitpid(pid, &status, 0);
            
            enableRaw(&Terminal);
        }
    }
}
