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

struct termios cookedTerminal;

int main() {   
    char buffer[1024];
    int position = 0;
    char character;
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
                position = 0;
                sout("%s", prompt);
                continue;
            }
            break;
        }

        if (bytes == 0) continue;
       
        // Enter key (ASCII 13)
        if (character == 13) {
            if (character == 13 || character == 10) {
                buffer[position] = '\0';
                
                sout("\r\n");
                
                if (position > 0) {
                    execute(buffer);
                }
                
                sout("%s", prompt);
                
                position = 0;
            }
        } 
        
        // Backspace (ASCII 127)
        else if (character == 127) {
            if (position > 0) {
                position--;
                sout("\b \b");
            }
        } 
            
        else if (position < 1023) {
            buffer[position++] = character;
            sout("%c", character);
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
    disableRaw(&cookedTerminal);

    
    // Fash standard commands
    if (strcmp(buffer, "exit") == 0) {
        sout("exit\r\n");
        disableRaw(&cookedTerminal);
        exit(0);
    }

    pid_t pid = fork();
    if (pid == 0) {
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        

        execvp(argv[0], argv);
        if (errno == ENOENT) {
            sout("\r\nfash: %s: command not found\r\n", buffer);
        }
        exit(1);


    } else if (pid > 0) {
        wait(NULL);
    }


    enableRaw(&cookedTerminal);

}
