#include "turgen.h"
#include "render.h"
#include "sout.h"

/*
 * This builtin echo prints every argument separated by an space and ends the output with a new line
 * it also deletes the ""
*/

int
echo(char **argv, ShellState *state)
{
    int i;
    int len;
    int inquote;
    char *arg;
    (void) state;
    
    i = 1; /* because the arg 0 is "echo" and we dont want to print echo */
    inquote = 0;

    while (argv[i] != NULL)
    {
        arg = argv[i];
        len = strlen(arg);

        /* if the full arg is open and closed by quotes */
       if (len >= 2 && arg[0] == '"' && arg[len -1] == '"') {
            write(STDOUT_FILENO, arg + 1, len - 2);
       }
    
       /* if the arg starts with quotes */
       else if (len > 0 && arg[0] == '"') {
            inquote = 1;
            write(STDOUT_FILENO, arg + 1, len - 1);
        }

       /* if the arg ends with quotes */
       else if (inquote && len > 0 && arg[len - 1] == '"') {
           write(STDOUT_FILENO, arg, len -1);
           inquote = 0;
       }

       /* regular ones */
       else {
            sout("%s", arg);
       }

       /* adding spaces */
       if (argv[i + 1] != NULL) {
           sout(" ");
       }

       i++;
    }
    /* End of Line */
    sout("\r\n");
    return 0;
}

