#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

