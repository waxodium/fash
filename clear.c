#include <stdio.h>

void clearScreen() {
    puts("\033[H\033[2J\033[3J");
    fflush(stdin);
}
