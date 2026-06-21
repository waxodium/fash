#include "turgen.h"

void strip(char *string) {
    if (string == NULL || strlen(string) < 2) return;

    size_t length = strlen(string);
    char first = string[0];
    char last = string[length - 1];

    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) 
    {
        memmove(string, string + 1, length - 2);
        string[length - 2] = '\0';
    }
}
