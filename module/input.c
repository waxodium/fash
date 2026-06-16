#include "turgen.h"

#include "input.h"
#include "sout.h"
#include "render.h"
#include "terminal.h"
#include "prompter.h"
#include "tree.h"

void execute(char *buffer);

static char history[100][4096];
static int historyCount = 0;
static int historyView = 0;

extern struct termios Terminal;

void input(ShellState *state, char character, char *prompt) {
    int old_cursor = state->cursor;
    int renderAble = 0;

    switch (character) {
        
        // CTRL-A: To beginning line
        case 1:
            state->cursor = 0;
            renderAble = 1;
            break;

        // CTRL-C
        case 3:
            sout("^C\r\n");
            state->buffer[0] = '\0';
            state->length = 0;
            state->cursor = 0;
            
            render_init(state, prompt);
            sout("%s", prompt);
            return;

        // CTRL-E: To end of line
        case 5:
            state->cursor = state->length;
            renderAble = 1;
            break;

        // Enter key
        case 13:
            state->buffer[state->length] = '\0';
            sout("\r\n");
            
            if (state->length > 0) {
                if (historyCount == 100) {
                    for (int i = 0; i < 99; i++) {
                        strcpy(history[i], history[i + 1]);
                    }
                    historyCount = 99;
                }
                
                if (historyCount == 0 || strcmp(history[historyCount - 1], state->buffer) != 0) {
                    strcpy(history[historyCount], state->buffer);
                    historyCount++;
                }

                execute(state->buffer);
                prompter(prompt, 256);
            }
            
            historyView = historyCount;
            render_init(state, prompt);
            sout("%s", prompt);
            return;

        // BackSpace
        case 127:
            if (state->cursor > 0) {
                for (int i = state->cursor - 1; i < state->length - 1; i++) {
                    state->buffer[i] = state->buffer[i + 1];
                }
                state->cursor--;
                state->length--;
                state->buffer[state->length] = '\0';
                renderAble = 1;    
            }
            break; 

        // (Arrows, HOME, END)
        case 27: {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) <= 0 || read(STDIN_FILENO, &seq[1], 1) <= 0) {
                break;
            }

            if (seq[0] == '[') {
                // Up Arrow
                if (seq[1] == 'A') {
                    if (historyView > 0) {
                        historyView--;
                        strcpy(state->buffer, history[historyView]);
                        state->length = strlen(state->buffer);
                        state->cursor = state->length;
                        renderAble = 1;
                    }
                } 
                // Down Arrow
                else if (seq[1] == 'B') {
                    if (historyView < historyCount) {
                        historyView++;
                        if (historyView == historyCount) {
                            state->buffer[0] = '\0';
                            state->length = 0;
                        } else {
                            strcpy(state->buffer, history[historyView]);
                            state->length = strlen(state->buffer);
                        }
                        state->cursor = state->length;
                        renderAble = 1;
                    }
                }
                // Right Arrow
                else if (seq[1] == 'C') {
                    if (state->cursor < state->length) {
                        state->cursor++;
                        renderAble = 1;
                    }
                } 
                
                // Left Arrow
                else if (seq[1] == 'D') {
                    if (state->cursor > 0) {
                        state->cursor--;
                        renderAble = 1;
                    }
                }
                
                // HOME Keys
                else if (seq[1] == 'H' || seq[1] == '1') {
                    if (seq[1] == '1') { char trash; read(STDIN_FILENO, &trash, 1); } 
                    state->cursor = 0;
                    renderAble = 1;
                }
                // End Keys
                else if (seq[1] == 'F' || seq[1] == '4') {
                    if (seq[1] == '4') { char trash; read(STDIN_FILENO, &trash, 1); } 
                    state->cursor = state->length;
                    renderAble = 1;
                }
            }
            
            else if (seq[0] == 'O') {
                if (seq[1] == 'H') {         
                    state->cursor = 0;
                    renderAble = 1;
                } else if (seq[1] == 'F') {  
                    state->cursor = state->length;
                    renderAble = 1;
                }
            }
            break;
        }
        
        // CTRL-D 
        case 4:
            sout("\r\nexit\r\n");
            disableRaw(&Terminal);
            exit(0);
            break;
        
        // TAB key
        case 9:
            if (strchr(state->buffer, '*')) {
                TabTree(state);
                return;
            }
            break;

        default:
            if (state->length < 4095 && character >= 32 && character <= 126) {
                if (state->cursor < state->length) {
                    for (int i = state->length; i > state->cursor; i--) {
                        state->buffer[i] = state->buffer[i - 1];
                    }
                }
                state->buffer[state->cursor++] = character;
                state->length++;
                state->buffer[state->length] = '\0';
                
                renderAble = 1;
            }
            break;
    }

    if (renderAble) {
        render_update(state, old_cursor);
    }
}
