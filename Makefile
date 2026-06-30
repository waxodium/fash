CC = gcc
CFLAGS = -Wall -Wextra -Werror -Ilib -Imodule -Icommand -Iui -Iinclude -Iinclude/module -Iinclude/command -Iinclude/ui -Iinclude/parser

SRC    = main.c $(wildcard lib/*.c) $(wildcard module/*.c) $(wildcard command/*.c) $(wildcard ui/*.c) $(wildcard parser/*.c)
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
