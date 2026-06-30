#include "turgen.h"

#define LIMIT 32

static int operator(char c) {
    return c == '&' ||
           c == '|' ||
           c == ';' ||
           c == '<' ||
           c == '>';
}


char **tokenize(char *input) {
    if (!input)
        return NULL;

    int cap = LIMIT;
    int count = 0;

    char **args = malloc(cap * sizeof(char *));
    if (!args)
        return NULL;


    char *reader = input;
    char *writer = input;

    bool active = false;
    char quote = '\0';
    bool escaped = false;


    while (*reader)
    {

        if (count >= cap - 1)
        {
            cap *= 2;

            char **tmp = realloc(args, cap * sizeof(char *));
            if (!tmp)
            {
                free(args);
                return NULL;
            }

            args = tmp;
        }


        char c = *reader;


        if (escaped)
        {
            if (!active)
            {
                args[count++] = writer;
                active = true;
            }

            *writer++ = c;

            escaped = false;
            reader++;
            continue;
        }


        if (c == '\\' && quote != '\'')
        {
            escaped = true;

            if (!active)
            {
                args[count++] = writer;
                active = true;
            }

            reader++;
            continue;
        }


        if (quote)
        {
            if (c == quote)
            {
                quote = '\0';
            }
            else
            {
                *writer++ = c;
            }

            reader++;
            continue;
        }




        if (c == '\'' || c == '"')
        {
            quote = c;

            if (!active)
            {
                args[count++] = writer;
                active = true;
            }

            reader++;
            continue;
        }


        if (operator(c))
        {
            if (active)
            {
                *writer++ = '\0';
                active = false;
            }


            args[count++] = writer;

            *writer++ = *reader++;

            if ((*reader == c) &&
                (c == '&' || c == '|' || c == '<' || c == '>'))
            {
                *writer++ = *reader++;
            }

            *writer++ = '\0';

            continue;
        }


        if (isspace((unsigned char)c))
        {
            if (active)
            {
                *writer++ = '\0';
                active = false;
            }

            reader++;
            continue;
        }


        if (!active)
        {
            args[count++] = writer;
            active = true;
        }

        *writer++ = c;
        reader++;
    }


    if (active)
    {
        *writer++ = '\0';
    }


    args[count] = NULL;

    return args;
}

