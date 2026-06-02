#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>

#include <string.h>
#include <errno.h>
#include <signal.h>

#include "lib/sout.h"

void enableRaw(struct termios *orgTerminal);
void disableRaw(struct termios *orgTerminal);
void execute(char *buffer);
void arrows(char *inputBuffer, int *cursor, int *inputLength, char *prompt, int *total, int *historyIndex, char historyList[10][1024]);

struct termios cookedTerminal;

int main() {
    char buffer[1024];
    char character;
    char history[10][1024];
    
    int count = 0;
    int length = 0;
    int cursor = 0;
    int historyPosition = -1;
    
    char prompt[] = "fash ~> ";

    enableRaw(&cookedTerminal);

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sout("%s", prompt);

    while (1) {
        int bytes = read(STDIN_FILENO, &character, 1);
        if (bytes == -1) {
            if (errno == EINTR) {
                sout("\r\n");
                length = 0;
                cursor = 0;
                sout("%s", prompt);
                continue;
            }
            break;
        }

        if (bytes == 0) continue;

        switch (character) {
        case 13:
            buffer[length] = '\0';
            sout("\r\n");
            if (length > 0) {
                strcpy(history[count % 10], buffer);
                count++;
                execute(buffer);
            }
            sout("%s", prompt);
            length = 0;
            cursor = 0;
            historyPosition = -1;
            break;

        case 127:
            if (cursor > 0) {
                for (int i = cursor - 1; i < length - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                cursor--;
                length--;
                buffer[length] = '\0';
        
                sout("\r\033[K%s%s", prompt, buffer);
                for (int i = 0; i < (length - cursor); i++) {
                    sout("\033[D");
                }
            }
            break;

        case 27:
            arrows(buffer, &cursor, &length, prompt, &count, &historyPosition, history);
            break;
        
        case 4:
            sout("\r\nexit\r\n");
            disableRaw(&cookedTerminal);
            exit(0);
            break;

        default:
            if (length < 1023) {
                if (cursor == length) {
                    buffer[cursor] = character;
                    cursor++;
                    length++;
                    buffer[length] = '\0';
                    
                    sout("%c", character); 
                } else {
                    for (int i = length; i > cursor; i--) {
                        buffer[i] = buffer[i - 1];
                    }
                    
                    buffer[cursor] = character;
                    cursor++;
                    length++;
                    buffer[length] = '\0';
            
                    sout("\r\033[K%s%s", prompt, buffer);
                    for (int i = 0; i < (length - cursor); i++)
                        sout("\033[D");
                    
                }
            }
        }
    }

    disableRaw(&cookedTerminal);
    return 0;
}

void enableRaw(struct termios *orgTerminal) {
    struct termios rawTerminal;
    tcgetattr(STDIN_FILENO, orgTerminal);
    rawTerminal = *orgTerminal;
    rawTerminal.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    rawTerminal.c_oflag &= ~(OPOST);
    rawTerminal.c_cflag |= (CS8);
    rawTerminal.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    rawTerminal.c_cc[VMIN] = 0;
    rawTerminal.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawTerminal);
}

void disableRaw(struct termios *orgTerminal) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orgTerminal);
}

void execute(char *buffer) {
    char *argv[1024];
    int b = 0;
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        argv[b] = token;
        b++;
        token = strtok(NULL, " ");
    }
    argv[b] = NULL;

    if (argv[0] == NULL) return;

    if (strcmp(argv[0], "cls") == 0) {
        sout("\033[H\033[J");
        return; 
    }

    if (strcmp(argv[0], "exit") == 0) {
        sout("exit\r\n");
        disableRaw(&cookedTerminal); 
        exit(0);
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
        if (errno == ENOENT) {
            sout("\rfash: %s: command not found\r\n", argv[0]);
        }
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    }
    
    enableRaw(&cookedTerminal);
}


void arrows(char *inputBuffer, int *cursor, int *inputLength, char *prompt, int *total, int *historyIndex, char historyList[10][1024]) {
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) <= 0 || read(STDIN_FILENO, &seq[1], 1) <= 0) return;
    if (seq[0] != '[') return;

    if (seq[1] == 'A') {
        if (*historyIndex < *total - 1 && *historyIndex < 9) {
            (*historyIndex)++;
        } else return;
    } else if (seq[1] == 'B') {
        if (*historyIndex >= 0) {
            (*historyIndex)--;
        } else return;
    } else if (seq[1] == 'C') {
        if (*cursor < *inputLength) {
            (*cursor)++;
            sout("\033[C");
        }
        return;
    } else if (seq[1] == 'D') {
        if (*cursor > 0) {
            (*cursor)--;
            sout("\033[D");
        }
        return;
    } else return;

    sout("\r\033[K%s", prompt);

    if (*historyIndex >= 0) {
        int target = (*total - 1 - *historyIndex) % 10;
        if (target < 0) target += 10;
        strcpy(inputBuffer, historyList[target]);
        *inputLength = strlen(inputBuffer);
        *cursor = *inputLength;
        sout("%s", inputBuffer);
    } else {
        *inputLength = 0;
        *cursor = 0;
        inputBuffer[0] = '\0';
    }
}
