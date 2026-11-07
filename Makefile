CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lSDL3
BUILD_DIR = build


all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) include/wares.h src/main.c -o $(BUILD_DIR)/wares $(LDFLAGS)

clean: 
	rm -rf $(BUILD_DIR)

.PHONY: clean
