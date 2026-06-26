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
    int stream = 0; 
    int cursor = 0;

    while (cursor < argc) {
        char *args[1024];
        int count = 0;
        int piped = 0;

        struct {
            int source;
            int merge;
            int finish;
            int target;
            char *file;
            int append;
            int input;
        } redirs[32];
        int total = 0;

        while (cursor < argc) {
            char *word = argv[cursor];

            if (strcmp(word, "|") == 0) {
                piped = 1;
                cursor++;
                break;
            } 

            char *marker = strpbrk(word, "<>");
            if (marker == NULL) {
                args[count++] = word;
                cursor++;
                continue;
            }

            int local = 1;
            if (*marker == '<') {
                local = 0;
            }
            
            if (marker > word && isdigit((unsigned char)word[0])) {
                local = atoi(word);
            }

            if (*(marker + 1) == '&') {
                redirs[total].source = local;
                
                if (*(marker + 2) == '-') {
                    redirs[total].merge = 0;
                    redirs[total].finish = 1;
                    cursor += 1;
                }
                else if (*(marker + 2) != '\0') {
                    redirs[total].merge = 1;
                    redirs[total].finish = 0;
                    redirs[total].target = atoi(marker + 2);
                    cursor += 1;
                } 
                else {
                    redirs[total].merge = 1;
                    redirs[total].finish = 0;
                    if (cursor + 1 < argc) {
                        redirs[total].target = atoi(argv[cursor + 1]);
                        cursor += 2;
                    } else {
                        cursor += 1;
                    }
                }
                total++;
            } 
            else {
                redirs[total].source = local;
                redirs[total].merge = 0;
                redirs[total].finish = 0;
                
                redirs[total].input = 0;
                if (*marker == '<') {
                    redirs[total].input = 1;
                }

                redirs[total].append = 0;
                if (*(marker + 1) == '>') {
                    redirs[total].append = 1;
                }

                redirs[total].file = argv[cursor + 1];
                cursor += 2;
                total++;
            }
        }
        args[count] = NULL;

        if (count == 0) continue;

        int links[2];
        if (piped) {
            if (pipe(links) < 0) { perror("pipe"); return; }
        }

        disableRaw(&Terminal);

        pid_t child = fork();
        if (child < 0) { perror("fork"); enableRaw(&Terminal); return; }

        if (child != 0) {
            if (stream != 0) close(stream);

            if (piped) {
                close(links[1]);
                stream = links[0];
            } else {
                stream = 0;
            }

            int status;
            waitpid(child, &status, 0);
            enableRaw(&Terminal);
            continue;
        }

        struct sigaction action;
        action.sa_handler = SIG_DFL;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGINT, &action, NULL);

        if (stream != 0) {
            dup2(stream, 0);
            close(stream);
        }

        if (piped) {
            dup2(links[1], 1);
            close(links[0]);
            close(links[1]);
        }

        for (int step = 0; step < total; step++) {
            if (redirs[step].finish) {
                if (close(redirs[step].source) < 0) {
                    perror("close fd");
                    exit(1);
                }
                continue;
            } 
            
            if (redirs[step].merge) {
                if (dup2(redirs[step].target, redirs[step].source) < 0) {
                    perror("dup2 merge");
                    exit(1);
                }
                continue;
            } 

            int modes = O_WRONLY | O_CREAT | O_TRUNC;
            if (redirs[step].input) {
                modes = O_RDONLY;
            } else if (redirs[step].append) {
                modes = O_WRONLY | O_CREAT | O_APPEND;
            }

            int desk = open(redirs[step].file, modes, 0644);
            if (desk < 0) {
                perror("open failed");
                exit(1);
            }
            if (dup2(desk, redirs[step].source) < 0) {
                perror("dup2 file");
                close(desk);
                exit(1);
            }
            close(desk);
        }

        execvp(args[0], args);
        if (errno == ENOENT) {
            sout("\r%s: %s: command not found\r\n", shellname, args[0]);
        }
        exit(1);
    }
}
