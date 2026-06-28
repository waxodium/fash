#include "turgen.h"
#include "terminal.h"
#include "render.h"

void seperate(char **argv, int argc, ShellState *state) {
    (void) *state;
    int cursor = 0;

    while (cursor < argc) {
        char *args[1024];
        int count = 0;

        while (cursor < argc && strcmp(argv[cursor], ";") != 0) {
            args[count++] = argv[cursor++];
        }
        
        if (cursor < argc && strcmp(argv[cursor], ";") == 0) {
            cursor++;
        }

        args[count] = NULL;
        if (count == 0) continue;

        // >> disable rawMode first
        disableRaw(&Terminal);

        pid_t child = fork();
        
        if (child == 0) {
            
            execvp(args[0], args);
            exit(1);

        } else 
        {
            wait(NULL);
            enableRaw(&Terminal);
        }
    
    }

}
