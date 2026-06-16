#include "turgen.h"

#include "render.h"

#define DEPTH 32
#define ENTRIES 512
#define viewport 20

typedef struct {
    char path[1024];
    char name[256];
    int depth;
    bool end;
    bool last[DEPTH];
} direntry;

static direntry listDir[ENTRIES];
static int count = 0;

void ScanDirectories(const char *base_path, int depth, bool *last) {
    if (depth >= DEPTH || count >= ENTRIES) {
        return;
    }

    DIR *dir = opendir(base_path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    struct stat status;
    char path[1024];
    
    int subdirs = 0;

    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);
        if (stat(path, &status) == 0 && S_ISDIR(status.st_mode)) {
            subdirs++;
        }
    }
    
    rewinddir(dir);

    int currentsubd = 0;

    while ((entry = readdir(dir))) {
        if (count >= ENTRIES) 
            break;
        if (entry->d_name[0] == '.') 
            continue;

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);
        if (stat(path, &status) == 0 && S_ISDIR(status.st_mode)) {
            currentsubd++;
            
            bool lastly = (currentsubd == subdirs);
            
            direntry *de = &listDir[count++];
            
            snprintf(de->path, sizeof(de->path), "%s", path);
            snprintf(de->name, sizeof(de->name), "%s", entry->d_name);
            de->depth = depth;
            de->end = lastly;

            memcpy(de->last, last, sizeof(bool) * depth);
            de->last[depth] = lastly;

            ScanDirectories(path, depth + 1, de->last);
        }
    }
    
    closedir(dir);
}


void DrawUI(char *Frame, int *position, int selected, int start, int border) {
    char buffer[512];
    
    for (int i = start; i < border; i++) {
        
        int level = (listDir[i].depth > 10) ? 10 : listDir[i].depth;
        append_str(Frame, position, "\033[0m");
        
        if (i == selected) {
            append_str(Frame, position, "\033[7m");
        }
        
        for (int d = 0; d < level; d++) {
            if (listDir[i].last[d]) {
                append_str(Frame, position, "    ");
            } else {
                append_str(Frame, position, "│   ");
            }
        }
        
        if (listDir[i].end) {
            snprintf(buffer, sizeof(buffer), "└──── %s\033[0m\033[K\r\n", listDir[i].name);
        } else {
            snprintf(buffer, sizeof(buffer), "├──── %s\033[0m\033[K\r\n", listDir[i].name);
        }
                 
        append_str(Frame, position, buffer);
    }
}


void TabTree(ShellState *state) {
    count = 0;
    memset(listDir, 0, sizeof(listDir));
    bool root[DEPTH] = {false};
    char target[1024];
    char *input = state->buffer;

    bool changedDir = (strncmp(input, "cd ", 3) == 0);
    if (changedDir) {
        input += 3;
    }

    if (input[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) {
            home = ".";
        }
        snprintf(target, sizeof(target), "%s%s", home, input + 1);
    }
    if (strlen(input) == 0) {
        strcpy(target, ".");
    }
    if (strlen(input) > 0 && input[0] != '~') {
        strncpy(target, input, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
    }

    struct stat status;
    bool dirs = (stat(target, &status) == 0 && S_ISDIR(status.st_mode));
    char *slash = strrchr(target, '/');

    if (!dirs && slash) {
        *slash = '\0';
    }
    if (!dirs && !slash) {
        strcpy(target, ".");
    }

    ScanDirectories(target, 0, root);
    if (count == 0) return;

    int selected = 0;
    int start = 0;
    
    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "\033[?25l", 6);

    while (1) {
        if (selected < start) {
            start = selected;
        }
        if (selected >= start + viewport) {
            start = selected - viewport + 1;
        }
        
        int border = start + viewport;
        if (border > count) {
            border = count;
        }
        int lines = border - start;

        char frame[8192] = {0};
        int position = 0;
        
        append_str(frame, &position, "\r");
        DrawUI(frame, &position, selected, start, border);
        
        char up[16];
        snprintf(up, sizeof(up), "\033[%dA", lines);
        append_str(frame, &position, up);
        
        write(STDOUT_FILENO, frame, position);

        char key;
        if (read(STDIN_FILENO, &key, 1) <= 0) {
            continue;
        }

        if (key == 27) {
            struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
            if (poll(&pfd, 1, 15) == 0) {
                break;
            }

            char segment[3];
            read(STDIN_FILENO, &segment[0], 2);

            if (segment[1] == 'A' && selected > 0) {
                selected--;
            }
            if (segment[1] == 'B' && selected < count - 1) {
                selected++;
            }
            continue;
        }

        if (key == 13) {
            if (changedDir) {
                snprintf(state->buffer, 4095, "cd %s", listDir[selected].path);
            }
            if (!changedDir) {
                snprintf(state->buffer, 4095, "%s", listDir[selected].path);
            }
            state->length = strlen(state->buffer);
            state->cursor = state->length;
            break;
        }

        if (key == 'q') {
            break;
        }
    }
    
    write(STDOUT_FILENO, "\r\033[J\033[1A\r\033[K", 13);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    write(STDOUT_FILENO, "\033[?25h", 6);
    render_update(state, 0);
}
