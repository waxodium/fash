#include "turgen.h"
#include "sout.h"

#include "echo.h"


int echo(char **argv, ShellState *state)
{
    int i = 1;
    (void) state;

    while (argv[i] != NULL)
    {
        sout("%s", argv[i]);

        if (argv[i + 1] != NULL) {
            sout(" ");
        }
        i++;
    }
    
    // EOF
    sout("\r\n");
    return 0;
}
