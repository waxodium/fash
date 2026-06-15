#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "sout.h"
#include "render.h"
#include "terminal.h"
#include "prompter.h"
#include "tree.h"
#include "input.h"




// Command History
char history[100][4096];
int historyCount = 0;
int historyView = 0;

struct termios Terminal;


int main() {
    char character;
    ShellState state;
    char prompt[256];
    prompter(prompt, sizeof(prompt));

    enableRaw(&Terminal);

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    render_init(&state, prompt);
    sout("%s", prompt); 

    while (1) {
        int bytes = read(STDIN_FILENO, &character, 1);
        if (bytes == -1 && errno == EINTR) {
            sout("\r\n");
            render_init(&state, prompt);
            sout("%s", prompt);
            continue;
        }
        
        if (bytes <= 0) continue;
        
        input(&state, character, prompt);
    }

    disableRaw(&Terminal);
    return 0;
}


