#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>

void cpu_init(CPU* cpu) {
	cpu->af = 0x01B0;
	cpu->bc = 0x0013;
	cpu->de = 0x00D8;
	cpu->hl = 0x014D;
	cpu->sp = 0xFFFE;
	cpu->pc = 0x0100; // Boot ROM entry point
}

void cpu_step(CPU* cpu, MMU* mmu) {
	uint16_t current_pc = cpu->pc;

	uint8_t opcode = mmu_read_byte(mmu, cpu->pc);
	cpu->pc++;

	switch (opcode) {
		case 0x00: // NOP
			break;

		case 0x3E: // LD A, d8
		{
			uint8_t val = mmu_read_byte(mmu, cpu->pc);
			cpu->a = val;
			cpu->pc++;
			break;
		}

		case 0xC3: // JP a16
		{
			uint8_t low = mmu_read_byte(mmu, cpu->pc);
			uint8_t high = mmu_read_byte(mmu, cpu->pc + 1);
			cpu->pc = (high << 8) | low;
			break;
		}
		case 0x21: // LD HL, d16
		{
			uint8_t low = mmu_read_byte(mmu, cpu ->pc);
			uint8_t high = mmu_read_byte(mmu, cpu->pc + 1);

			cpu->hl = (high << 8) | low;
			cpu->pc += 2;
			cpu->cycles += 12;
			break;
		}
		default:
			fprintf(stderr, "\nCRASH: Unimplemented Opcode 0x%02X at PC: 0x%04X\n\n", opcode, current_pc);
			fprintf(stderr, "Registers:\n");
			fprintf(stderr, "  A: 0x%02X  F: 0x%02X\n", cpu->a, cpu->f);
			fprintf(stderr, "  BC: 0x%04X DE: 0x%04X\n", cpu->bc, cpu->de);
			fprintf(stderr, "  HL: 0x%04X SP: 0x%04X\n", cpu->hl, cpu->sp);
			exit(EXIT_FAILURE);
	}
}
