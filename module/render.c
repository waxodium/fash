#include "fash.h"

#include "render.h"

int width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0) {
        return 80;
    }
    return w.ws_col;
}

void append_str(char *frame, int *pos, const char *str) {
    while (*str && *pos < 4095) {
        frame[(*pos)++] = *str++;
    }
}

void append_int(char *frame, int *pos, int num) {
    char buf[12];
    int i = 0;
    if (num == 0) {
        frame[(*pos)++] = '0';
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0 && *pos < 4095) {
        frame[(*pos)++] = buf[--i];
    }
}

void render_init(ShellState *state, const char *prompt) {
    strncpy(state->prompt, prompt, 255);
    state->prompt[255] = '\0';
    state->buffer[0] = '\0';
    state->length = 0;
    state->cursor = 0;
    state->term_width = width();
}

void render_update(ShellState *state, int old_cursor) {
    state->term_width = width();
    char frame[4096];
    int pos = 0;

    append_str(frame, &pos, "\033[?25l");

    int prompt_len = strlen(state->prompt);
    int old_pos = prompt_len + old_cursor;
    
    int old_row = old_pos / state->term_width;
    if (old_pos > 0 && old_pos % state->term_width == 0) {
        old_row--;
    }

    append_str(frame, &pos, "\r");
    for (int i = 0; i < old_row; i++) {
        append_str(frame, &pos, "\033[A");
    }
    

    if (state->length > 0) {
        append_str(frame, &pos, "\033[32;3m");
    } else {
        append_str(frame, &pos, "\033[0m");
    }
    
    append_str(frame, &pos, state->prompt);
    append_str(frame, &pos, "\033[0m");  
    
    if (state->length > 0) {
        append_str(frame, &pos, state->buffer);
    }

    append_str(frame, &pos, "\033[J");

    

    int current_pos = prompt_len + state->cursor;
    int total_len = prompt_len + state->length;
    
    int total_row = total_len / state->term_width;
    if (total_len > 0 && total_len % state->term_width == 0) {
        total_row--;
    }
    
    int cursor_row = current_pos / state->term_width;
    if (current_pos > 0 && current_pos % state->term_width == 0) {
        cursor_row--;
    }
    
    int cursor_col = current_pos % state->term_width;
    if (current_pos > 0 && current_pos % state->term_width == 0) {
        cursor_col = state->term_width;
    }

    for (int i = 0; i < (total_row - cursor_row); i++) {
        append_str(frame, &pos, "\033[A");
    }
    
    append_str(frame, &pos, "\r");
    if (cursor_col > 0) {
        append_str(frame, &pos, "\033[");
        append_int(frame, &pos, cursor_col);
        append_str(frame, &pos, "C");
    }

    append_str(frame, &pos, "\033[?25h");
    
    write(STDOUT_FILENO, frame, pos);
}
