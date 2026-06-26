#include "turgen.h"


int operators(char c) {
    return (c == '|' || c == '<' || c == '>' || c == '&' || c == ';' || c == '(' || c == ')');
}


/*
    [!NOTE]
    opsing() function is just a tokenizer for operators syntax. 
    NOT actual management for
    - redirection
    - processes
    - logical syntaxes

    Actual logic are placed in ../module/

    redirection, process, logical syntaxes should be in module,
    while operator.c in the parser/ will parses and read
    through returning the output for others to use later.
*/

char* opsing(const char *input) {
    char *output = 
        malloc(strlen(input) * 4 + 1);
    

    if (!output)  {
        return NULL;
    }

    

    int j = 0;
    int singlequote = 0;
    int doublequote = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\'' && !doublequote) {
            singlequote = !singlequote;
            output[j++] = input[i];
            continue;
        } else if (input[i] == '"' && !singlequote) {
            doublequote = !doublequote;
            output[j++] = input[i];
            continue;
        }

        if (!singlequote && !doublequote) {
            /** FD Duplication **/
            if ((input[i] == '1' || input[i] == '2') && input[i+1] == '>' && input[i+2] == '&' && isdigit(input[i+3])) {
                output[j++] = ' ';
                output[j++] = input[i];       // '2'
                output[j++] = input[++i];   // '>'
                output[j++] = input[++i];   // '&'
                output[j++] = input[++i];   // '1'
                output[j++] = ' ';


                // output[j++] = input[i++]; 
                continue;
            }

            // FD Redirection with numbers 1 and 2
            if ((input[i] == '1' || input[i] == '2') && input[i+1] == '>') {
                output[j++] = ' ';
                output[j++] = input[i];     // '2'
                output[j++] = input[++i];   // '>'
                output[j++] = ' ';
                continue;
            }

            // standard operator 
            if ((input[i] == '&' && input[i+1] == '&') || 
                (input[i] == '>' && input[i+1] == '>') ||
                (input[i] == '>' && input[i+1] == '&')) {
                
                output[j++] = ' ';
                output[j++] = input[i];
                output[j++] = input[++i];
                output[j++] = ' ';
                continue;
            }

            // Singular Operator
            if (operators(input[i])) {
                output[j++] = ' ';
                output[j++] = input[i];
                output[j++] = ' ';
                continue;
            }
        } 
        
        output[j++] = input[i];
    }
    

    output[j] = '\0';
    

    return output;
}
