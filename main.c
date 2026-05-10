#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "clear.c"


void execute(char *buffer);

int main() {
    fputs("Welcome to fcsh! The friendly c-shell\n", stdout);
    int i = true;
    char input[1024];

    while(i) {
        printf("fcsh> ");
        char *get = fgets(input, sizeof(input), stdin);
        input[sizeof(input)] = *get;
        
        if (input[0] == '\n') {
            continue;
        };
        
        char *check = strstr(input, "clear");
        if (check) {
            clearScreen();
        } else if (!check) {
            input[strcspn(input, "\n")] = 0;
            execute(input);
        }

        if (get) {
            continue;
        }

    }

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

    if (argv[0] == NULL) {
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        return;
    } else if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            exit(1);
        }
    } else {
        wait(NULL);
    }
}
