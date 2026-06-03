#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "lib/sout.h"
#include "lib/render.h"
#include "lib/terminal.h"

void execute(char *buffer);

// Command History
char history[100][4096];
int historyCount = 0;
int historyView = 0;

struct termios cookedTerminal;


int main() {
    char character;
    ShellState state;
    char prompt[] = "fash ~> ";

    enableRaw(&cookedTerminal);

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    render_init(&state, prompt);
    sout("%s", prompt); 

    while (1) {
        int bytes = read(STDIN_FILENO, &character, 1);
        if (bytes == -1 && errno == EINTR) {
            sout("\r\n");
            render_init(&state, prompt);
            sout("%s", prompt);
            continue;
        }
        if (bytes <= 0) continue;

        int old_cursor = state.cursor;

        switch (character) {
        
        // For Enter key
        case 13:
            state.buffer[state.length] = '\0';
            sout("\r\n");
            
            if (state.length > 0) {
                if (historyCount == 100) {
                    for (int i = 0; i < 99; i++) {
                        strcpy(history[i], history[i + 1]);
                    }
                    historyCount = 99;
                }
                
                if (historyCount == 0 || strcmp(history[historyCount - 1], state.buffer) != 0) {
                    strcpy(history[historyCount], state.buffer);
                    historyCount++;
                }

                execute(state.buffer);
            }
            
            historyView = historyCount;
            
            render_init(&state, prompt);
            sout("%s", prompt);
            continue; 

        // BackSpace
        case 127:
            if (state.cursor > 0) {
                for (int i = state.cursor - 1; i < state.length - 1; i++) {
                    state.buffer[i] = state.buffer[i + 1];
                }
                state.cursor--;
                state.length--;
                state.buffer[state.length] = '\0';
            }
            break; 

        case 27: // Arrows
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) <= 0 || read(STDIN_FILENO, &seq[1], 1) <= 0) {
                break;
            }

            if (seq[0] != '[') {
                break;
            }

            // Command History
            if (seq[1] == 'C') {
                if (state.cursor < state.length) state.cursor++;
            } else if (seq[1] == 'D') {
                if (state.cursor > 0) state.cursor--;
            }
            
            // Up Arrow
            else if (seq[1] == 'A') {
                if (historyView > 0) {
                    historyView--;
                    strcpy(state.buffer, history[historyView]);
                    state.length = strlen(state.buffer);
                    state.cursor = state.length;
                }

            } 


            // Down Arrow
            else if (seq[1] == 'B') {
                if (historyView < historyCount) {
                    historyView++;
                    if (historyView == historyCount) {
                        state.buffer[0] = '\0';
                        state.length = 0;
                    } else {
                        strcpy(state.buffer, history[historyView]);
                        state.length = strlen(state.buffer);
                    }
                    state.cursor = state.length;
                }
            }

            break;

        
        // CTRL-D 
        case 4:
            sout("\r\nexit\r\n");
            disableRaw(&cookedTerminal);
            exit(0);
            break;

        default:
            if (state.length < 4095 && character >= 32 && character <= 126) {
                if (state.cursor < state.length) {
                    for (int i = state.length; i > state.cursor; i--) {
                        state.buffer[i] = state.buffer[i - 1];
                    }
                }
                state.buffer[state.cursor++] = character;
                state.length++;
                state.buffer[state.length] = '\0';
            }
            break;
        }

        render_update(&state, old_cursor);
    }

    disableRaw(&cookedTerminal);
    return 0;
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

    

    if (strcmp(argv[0], "cls") == 0 || strcmp(argv[0], "clear") == 0) {
        sout("\033[H\033[J");
        return; 
    }
    

    if (strcmp(argv[0], "exit") == 0) {
        sout("exit\r\n");
        disableRaw(&cookedTerminal); 
        exit(0);
    }
    

    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] == NULL) chdir(getenv("HOME"));
        else if (chdir(argv[1]) != 0) sout("\rfash: cd: %s: No such file or directory\r\n", argv[1]);
        return;
    }

    


    disableRaw(&cookedTerminal);

    
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
    
    enableRaw(&cookedTerminal);
}

