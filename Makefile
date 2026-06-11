CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude -Ilib -Imodule -Icommand

SRC = main.c lib/*.c module/*.c command/*.c
TARGET = fash

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
