CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lSDL3
BUILD_DIR = build


all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) src/main.c src/mmu.c src/cpu.c -o $(BUILD_DIR)/wares $(LDFLAGS)

clean: 
	rm -rf $(BUILD_DIR)

.PHONY: clean
