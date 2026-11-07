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
	cpu->ime = false;
}

// HELPER FUNCTIONS

static uint8_t cpu_inc_r8(CPU* cpu, uint8_t val) {
	val++;
			
	// Z Flag: Set if E is 0
	if (val == 0) {
		cpu->f |= FLAG_Z;
	} else {
		cpu->f &= ~FLAG_Z;
	}

	cpu->f &= ~FLAG_N; //N Flag: reset to 0

	//H Flag: Set if there's carry from bit 3 -> 4
	if ((val & 0x0F) == 0) {
		cpu->f |= FLAG_H;
	} else {
		cpu->f &= ~FLAG_H;
	}

	return val;
}


static uint8_t cpu_dec_r8(CPU* cpu, uint8_t val) {
	val--;
	
	//Z Flag: set if result is 0
	if (val == 0) {
		cpu->f |= FLAG_Z;
	} else {
		cpu->f &= FLAG_Z;
	}

	// H flag: set if lower nibble of original value was 0
	if ((val & 0x0F) == 0) {
		cpu->f |= FLAG_H;
	} else {
		cpu->f &= FLAG_H;
	}

	//N flag: set to 1
	cpu->f |= FLAG_N;

	return val;
}


void cpu_step(CPU* cpu, MMU* mmu) {
	uint16_t current_pc = cpu->pc;

	uint8_t opcode = mmu_read_byte(mmu, cpu->pc);
	// printf("PC: 0x%04X | Opcode: 0x%02X\n", cpu->pc - 1, opcode);
	cpu->pc++;

	switch (opcode) {
		case 0x00: // NOP
			cpu->cycles += 4;
			break;
		case 0x0D: //DEC C
		{
			cpu->c = cpu_dec_r8(cpu, cpu->c);
			cpu->cycles += 4;
			break;
		}
		case 0x0E: //LD C, d8
		{
            uint8_t val = mmu_read_byte(mmu, cpu->pc);
            cpu->c = val;
            cpu->pc++;
			cpu->cycles += 8;
            break;
		}
		case 0x12: // LD (DE), A
		{
			mmu_write_byte(mmu, cpu->de, cpu->a);
			cpu->cycles += 8;
			break;
		}
		case 0x14: //INC D
		{
			cpu->d = cpu_inc_r8(cpu, cpu->d);
			cpu->cycles += 4;
			break;
		}
		case 0x1C: // INC E
		{
			cpu->e = cpu_inc_r8(cpu, cpu->e);
			cpu->cycles += 4;
			break;
		}
		case 0x2A: // LD A, (HL+)
		{
			cpu->a = mmu_read_byte(mmu, cpu->hl);
			cpu->hl++;
			cpu->cycles += 8;
			break;
		}
		case 0x3E: // LD A, d8
		{
			uint8_t val = mmu_read_byte(mmu, cpu->pc);
			cpu->a = val;
			cpu->pc++;
			cpu->cycles += 8;
			break;
		}

		case 0xC3: // JP a16
		{
			uint8_t low = mmu_read_byte(mmu, cpu->pc);
			uint8_t high = mmu_read_byte(mmu, cpu->pc + 1);
			cpu->pc = (high << 8) | low;
			cpu->cycles += 16;
			break;
		}
		case 0x11: // LD DE, d16
		{
            uint8_t low = mmu_read_byte(mmu, cpu ->pc);
            uint8_t high = mmu_read_byte(mmu, cpu->pc + 1);

            cpu->de = (high << 8) | low;
            cpu->pc += 2;
            cpu->cycles += 12;
            break;
		}
		case 0x20: //JR NZ, r8
		{
			int8_t offset = (int8_t)mmu_read_byte(mmu, cpu->pc);
			cpu->pc++; //advance past the offset byte

			//check if zero flag is not set
			if ((cpu->f & FLAG_Z) == 0) {
				cpu->pc += offset;
				cpu->cycles += 12;
			} else {
				cpu->cycles += 8;
			}
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
		case 0x47: // LD B, A
		{
			cpu->b = cpu->a;
			cpu->cycles += 4;
			break;
		}
		case 0x78: // LD A, B
		{
			cpu->b = cpu->a;
			cpu->cycles += 4;
			break;
		}
		case 0xF3: // DI
		{
			cpu->ime = false;
			cpu->cycles += 4;
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
