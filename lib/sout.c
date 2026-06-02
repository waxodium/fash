/*
Copyright (C) 2026 waxodium <waxodium@proton.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdarg.h>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

void _putchar(char character) {
#if defined(_WIN32)
    DWORD written;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), &character, 1, &written, NULL);
#else
    write(1, &character, 1);
#endif
}

void sout(const char *format, ...) {
    static char buffer[1024];
    int pos = 0;
    va_list arguments;
    va_start(arguments, format);

    for (const char *cursor = format; *cursor != '\0'; cursor++) {
        if (pos >= 1023) {
            for(int i = 0; i < pos; i++) _putchar(buffer[i]);
            pos = 0;
        }

        if (*cursor == '%' && *(cursor + 1) != '\0') {
            cursor++;
            switch (*cursor) {
                case 's': {
                    char *text = va_arg(arguments, char *);
                    if (text == NULL) text = "(null)";
                    while (*text != '\0') {
                        if (pos >= 1023) {
                            for(int i = 0; i < pos; i++) _putchar(buffer[i]);
                            pos = 0;
                        }
                        buffer[pos++] = *text++;
                    }
                    break;
                }
                case 'd': {
                    int number = va_arg(arguments, int);
                    char digits_buffer[12];
                    int p = 0;
                    if (number == 0) {
                        buffer[pos++] = '0';
                    } else {
                        if (number < 0) {
                            buffer[pos++] = '-';
                            number = -number;
                        }
                        while (number > 0) {
                            digits_buffer[p++] = (number % 10) + '0';
                            number /= 10;
                        }
                        while (p > 0) {
                            if (pos >= 1023) {
                                for(int i = 0; i < pos; i++) _putchar(buffer[i]);
                                pos = 0;
                            }
                            buffer[pos++] = digits_buffer[--p];
                        }
                    }
                    break;
                }
                case 'c': {
                    buffer[pos++] = (char)va_arg(arguments, int);
                    break;
                }
                case '%': {
                    buffer[pos++] = '%';
                    break;
                }
                default: {
                    buffer[pos++] = '%';
                    buffer[pos++] = *cursor;
                    break;
                }
            }
        } else {
            buffer[pos++] = *cursor;
        }
    }

    for (int i = 0; i < pos; i++) {
        _putchar(buffer[i]);
    }

    va_end(arguments);
}
