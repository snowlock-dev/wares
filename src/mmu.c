#include "../include/mmu.h"
#include <stdio.h>
#include <string.h>

void mmu_init(MMU* mmu) {
	memset(mmu->ram, 0, MEMORY_SIZE);
}

bool mmu_load_rom(MMU* mmu, const char* filepath) {
	FILE* rom = fopen(filepath, "rb");
	if (!rom) {
		fprintf(stderr, "Failed to open ROM file: %s\n", filepath);
		return false;
	}

	fseek(rom, 0, SEEK_END);
	long size = ftell(rom);
	rewind(rom);

	if (size > 0x8000) {
		fprintf(stderr, "ROM size exceeds 32KB, not supported yet");
		return false;
	}

	size_t bytes_read = fread(mmu->ram, 1, size, rom); //read ROM into lower half of memory 0x0000 - 0x7FFF
	fclose(rom);

	printf("Successfully loaded %zu bytes into memory.\n", bytes_read);
	return true;
}

uint8_t mmu_read_byte(MMU* mmu, uint16_t address) {
	return mmu->ram[address];
}

void mmu_write_byte(MMU* mmu, uint16_t address, uint8_t value) {
	if (address < 0x8000) {
		return;
	}

	if (address == 0xFF01) {
		putchar(value); //print characters sent by test rom
		fflush(stdout);
	}
	mmu->ram[address] = value;
}
