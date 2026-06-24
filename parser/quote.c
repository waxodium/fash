#include "turgen.h"
#include <stdlib.h>
#include <stdbool.h>

#define LIMIT 16


/*
 * Dynamic Memory mananagement is not prohibited for strings related functions
 *
 * Read prohibition section from the ../DOCUMENTATION.md file
 */
char **tokenize(char *input) {
    if (!input) 
    {
        return NULL;
    }

    int limit = LIMIT;
    char **args = malloc(limit * sizeof(char *));
    if (!args) 
        return NULL;

    int count = 0;
    char *reader = input;
    char *writer = input;
    
    char quote = '\0';
    bool escaped = false;
    bool active = false;

    while (*reader) {
        if (count >= limit - 1) {
            limit *= 2; 
            
            char **temp = realloc(args, limit * sizeof(char *));
            if (!temp) {
                free(args); 
                return NULL; 
            }
            args = temp;
        }

        char c = *reader;

        if (escaped) {
            
            if (!active) {
                args[count++] = writer;
                active = true;
            }
            
            switch (c) {
                case 'n': *writer++ = '\n'; 
                    break;
                case 't': *writer++ = '\t'; break;
                case 'r': *writer++ = '\r'; break;
                default:  *writer++ = c;    break; 
            }
            escaped = false;
        } 
        
        else if (c == '\\' && quote != '\'') {
            escaped = true;
            
            if (!active) {
                args[count++] = writer;
                active = true;
            }
        } 
        
        else if (quote) {
            if (c == quote) {
                quote = '\0'; 
            } else {
                *writer++ = c;        
            }
        } 
        
        else {
            if (c == ' ' || c == '\t' || c == '\n') {
                if (active) {
                    *writer++ = '\0'; 
                    active = false;
                }
            } 
            
            // add tokenizing for pipe and file redirection
            else if (c == '>' || c == '<' || c == '|' || c == '&')
            {
                if (active)
                {
                    *writer++ = '\0';
                    active = false;
                }

                args[count++] = writer;

                *writer++ = c;

                if ((c == '>' || c == '<') && (*(reader + 1) == c || *(reader + 1) == '&'))
                {
                    reader++;
                    *writer++ = *reader;
                }

                *writer++ = '\0';

                reader++;
                continue;
            }

            else if (c == '"' || c == '\'') {
                quote = c;    
                
                if (!active) {
                    args[count++] = writer;
                    active = true;
                }
            } 
            
            else {
                if (!active) {
                    args[count++] = writer;
                    active = true;
                }
                *writer++ = c;
            }
        }
        reader++;
    }

    if (active) {
        *writer = '\0';
    }

    // // debug 
    // printf("Final buffer:\n");
    // for(char *p = input; p < writer + 5; p++)
    // {
    //     if(*p == '\0')
    //         printf("\\0");
    //     else
    //         putchar(*p);
    // }
    // printf("\n");
    // // debug end 
    
    args[count] = NULL; 

    return args;
}
