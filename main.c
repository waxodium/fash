#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "sout.h"
#include "render.h"
#include "terminal.h"
#include "navigation.h"

void execute(char *buffer);

// Command History
char history[100][4096];
int historyCount = 0;
int historyView = 0;

struct termios Terminal;


void prompter(char *prompt_buffer, size_t MaxLength) {
    char host[64];
    char cwd[1024];
    
    const char *user = getenv("USER");
    if (!user) 
        user = "user";
    
    if (gethostname(host, sizeof(host)) != 0) {
        strcpy(host, "unknown");
    }

    if (!getcwd(cwd, sizeof(cwd))) {
        strcpy(cwd, "?");
    }

    char *home = getenv("HOME");
    size_t home_length;

    if (home != NULL) {
        home_length = strlen(home);
    } else {
        home_length = 0;
    }

    if (home_length > 0 && strncmp(cwd, home, home_length) == 0) {
        char *pathRemainder = cwd + home_length;
        snprintf(prompt_buffer, MaxLength, "%s@%s:~%s > ", user, host, pathRemainder);
    } else {
        snprintf(prompt_buffer, MaxLength, "%s@%s:%s > ", user, host, cwd);
    }
}

int main() {
    char character;
    ShellState state;
    char prompt[256];
    prompter(prompt, sizeof(prompt));

    enableRaw(&Terminal);

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
        int renderAble = 0;

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
                prompter(prompt, sizeof(prompt));
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
                
                renderAble = 1;    
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

            // Right Arrow
            if (seq[1] == 'C') {
                if (state.cursor < state.length) {
                    state.cursor++;
                    renderAble = 1;
                }
            } 
            // Left Arrow
            else if (seq[1] == 'D') {
                if (state.cursor > 0) {
                    state.cursor--;
                    renderAble = 1;
                }
            }
            // Up Arrow
            else if (seq[1] == 'A') {
                if (historyView > 0) {
                    historyView--;
                    strcpy(state.buffer, history[historyView]);
                    state.length = strlen(state.buffer);
                    state.cursor = state.length;
                    renderAble = 1;
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
                    renderAble = 1;
                }
            }
            break;
        
        // CTRL-D 
        case 4:
            sout("\r\nexit\r\n");
            disableRaw(&Terminal);
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
                
                renderAble = 1;
            }
            break;
        }

        if (renderAble) {
            render_update(&state, old_cursor);
        }
    }

    disableRaw(&Terminal);
    return 0;
}


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

