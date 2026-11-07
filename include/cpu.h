#pragma once

#include <stdint.h>
#include "mmu.h"

#define FLAG_Z (1 << 7)
#define FLAG_N (1 << 6)
#define FLAG_H (1 << 5)
#define FLAG_C (1 << 4)

typedef struct {
	union {
		struct { uint8_t f, a; };
		uint16_t af;
	};
	union {
		struct { uint8_t c, b; };
		uint16_t bc;
	};
	union {
		struct { uint8_t e, d; };
		uint16_t de;
	};
	union {
		struct { uint8_t l, h; };
		uint16_t hl;
	};

	uint16_t sp;
	uint16_t pc;

	int cycles;
} CPU;

void cpu_init(CPU* cpu);
void cpu_step(CPU* cpu, MMU* mmu);
