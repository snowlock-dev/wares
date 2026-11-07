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

static void cpu_or_r8(CPU* cpu, uint8_t val) {
	cpu->a |= val;

	cpu->f = 0;
	if (cpu->a == 0) { cpu->f |= FLAG_Z; }
}

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
		case 0x01: //LD BC, d16
        {
            uint8_t low = mmu_read_byte(mmu, cpu ->pc);
            uint8_t high = mmu_read_byte(mmu, cpu->pc + 1);

            cpu->bc = (high << 8) | low;
            cpu->pc += 2;
            cpu->cycles += 12;
            break;
        }
		case 0x03: // INC BC
        {
            uint16_t bc = ((uint16_t)cpu->b << 8) | cpu->c;
            bc++;
            cpu->b = (bc>>8) & 0xFF;
            cpu->c = bc & 0xFF;
            cpu->cycles += 8;
            break;
        }
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
		case 0x18: // JR r8
		{
			int8_t offset = (int8_t)mmu_read_byte(mmu, cpu->pc++);
			cpu->pc += offset;
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
		case 0x23: // INC HL
		{
			uint16_t hl = ((uint16_t)cpu->h << 8) | cpu->l;
			hl++;
			cpu->h = (hl>>8) & 0xFF;
			cpu->l = hl & 0xFF;
			cpu->cycles += 8;
			break;
		}
		case 0x28: // JR Z, r8
		{
			int8_t offset = (int8_t)mmu_read_byte(mmu, cpu->pc++);
			if (cpu->f & FLAG_Z) {
				cpu->pc += offset;
				cpu->cycles += 12;
			} else {
				cpu->cycles += 8;
			}
			break;
		}
		case 0x31: // LD SP, d16
		{
			uint8_t low = mmu_read_byte(mmu, cpu->pc++); 
			uint8_t high = mmu_read_byte(mmu, cpu->pc++);

			cpu->sp = (high << 8) | low;
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
			cpu->a = cpu->b;
			cpu->cycles += 4;
			break;
		}
		case 0x7C: // LD A, H
		{
			cpu->a = cpu->h;
			cpu->cycles += 4;
			break;
		}
		case 0x7D: // LD A, L
		{
			cpu->a = cpu->l;
			cpu->cycles += 4;
			break;
		}
		case 0xB1: // OR C
		{
			cpu_or_r8(cpu, cpu->c);
			cpu->cycles += 4;
			break;
		}
        case 0xC5: // PUSH BC
        {
            cpu->sp -= 2;
            mmu_write_byte(mmu, cpu->sp + 1, cpu->b);
            mmu_write_byte(mmu, cpu->sp, cpu->c & 0xF0); // force lower 4 bits to 0
            cpu->cycles += 16;
            break;
        }
		case 0xC9: // RET
		{
			uint8_t low = mmu_read_byte(mmu, cpu->sp);
			uint8_t high = mmu_read_byte(mmu, cpu->sp + 1);
			cpu->sp += 2;

			cpu->pc = (high << 8) | low;
			cpu->cycles += 16;
			break;
		}
		case 0xCD: // CALL a16
		{
			uint8_t low = mmu_read_byte(mmu, cpu->pc++);
			uint8_t high = mmu_read_byte(mmu, cpu->pc++);
			uint16_t target = (high << 8) | low;

			cpu->sp -= 2; //push current pc into stack
			mmu_write_byte(mmu, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);
			mmu_write_byte(mmu, cpu->sp, cpu->pc & 0xFF);

			cpu->pc = target; //jump to target
			cpu->cycles += 24;
			break;
		}
		case 0xE0: //LDH (a8), A
		{
			uint8_t offset = mmu_read_byte(mmu, cpu->pc++);
			uint16_t address = 0xFF00 | offset;

			mmu_write_byte(mmu, address, cpu->a);
			cpu->cycles += 12;
			break;
		}
		case 0xE1: // POP HL
		{
			cpu->l = mmu_read_byte(mmu, cpu->sp);
			cpu->h = mmu_read_byte(mmu, cpu->sp + 1);
			cpu->sp += 2;
			cpu->cycles += 12;
			break;
		}
		case 0xE5: // PUSH HL
		{
			cpu->sp -= 2;
			mmu_write_byte(mmu, cpu->sp + 1, cpu->h);
			mmu_write_byte(mmu, cpu->sp, cpu->l);
			cpu->cycles += 16;
			break;
		}
		case 0xEA: //LD (a16), A
		{
			uint8_t low = mmu_read_byte(mmu, cpu->pc++);
			uint8_t high = mmu_read_byte(mmu, cpu->pc++);
			uint16_t address = (high << 8) | low;

			mmu_write_byte(mmu, address, cpu->a);
			cpu->cycles += 16;
			break;
		}
        case 0xF0: //LDH A, (a8)
        {
            uint8_t offset = mmu_read_byte(mmu, cpu->pc++);
            uint16_t address = 0xFF00 | offset;

            cpu->a = mmu_read_byte(mmu, address);
            cpu->cycles += 12;
            break;
        }        
		case 0xF1: // POP AF
        {
            cpu->f = mmu_read_byte(mmu, cpu->sp);
            cpu->a = mmu_read_byte(mmu, cpu->sp + 1);
            cpu->sp += 2;
            cpu->cycles += 12;
            break;
        }
		case 0xF3: // DI
		{
			cpu->ime = false;
			cpu->cycles += 4;
			break;
		}
		case 0xF5: // PUSH AF
		{
			cpu->sp -= 2;
			mmu_write_byte(mmu, cpu->sp + 1, cpu->a);
			mmu_write_byte(mmu, cpu->sp, cpu->f & 0xF0); // force lower 4 bits to 0
			cpu->cycles += 16;
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
