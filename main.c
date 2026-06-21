/*

///##########################################################################///

turgen shell
Copyright (C) 2026 waxodium <waxodium@proton.me>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

///##########################################################################///

*/

#include "turgen.h"

#include "sout.h"
#include "render.h"
#include "terminal.h"
#include "prompter.h"
#include "tree.h"
#include "input.h"



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


