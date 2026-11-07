#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 0x10000 // 64KB


typedef struct {
	uint8_t ram[MEMORY_SIZE];
} MMU;

void mmu_init(MMU* mmu);
bool mmu_load_rom(MMU* mmu, const char* filepath);
uint8_t mmu_read_byte(MMU* mmu, uint16_t address);
void mmu_write_byte(MMU* mmu, uint16_t address, uint8_t value);
