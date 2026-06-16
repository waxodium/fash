CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude -Ilib -Imodule -Icommand -Iui

SRC    = main.c $(wildcard lib/*.c) $(wildcard module/*.c) $(wildcard command/*.c) $(wildcard ui/*.c)
BUILD = ./build
TARGET = $(BUILD)/turgen

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	$(TARGET)

clean:
	rm -r $(BUILD)

.PHONY: all run clean
