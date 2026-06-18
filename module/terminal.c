// terminal.c
#include "turgen.h"

void enableRaw(struct termios *orgTerminal) {
    struct termios rawTerminal;
    tcgetattr(STDIN_FILENO, orgTerminal);
    rawTerminal = *orgTerminal;
    rawTerminal.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    rawTerminal.c_oflag &= ~(OPOST);
    rawTerminal.c_cflag |= (CS8);
    rawTerminal.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    rawTerminal.c_cc[VMIN] = 0;
    rawTerminal.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawTerminal);
}

void disableRaw(struct termios *orgTerminal) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orgTerminal);
}
