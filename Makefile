CC = gcc
CFLAGS = -Wall -Wextra -Werror
SRC = main.c lib/*.c
TARGET = fash

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
