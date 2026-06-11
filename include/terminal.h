#include <termios.h>

void enableRaw(struct termios *orgTerminal);
void disableRaw(struct termios *orgTerminal);
