#ifndef RENDER_H
#define RENDER_H

typedef struct {
    char buffer[4096];
    int length;
    int cursor;
    char prompt[256];
    int term_width;
} ShellState;

void render_init(ShellState *state, const char *prompt);
void render_update(ShellState *state, int old_cursor);
int width();

#endif
