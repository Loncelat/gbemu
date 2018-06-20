#pragma once

#define Z_FLAG (1 << 7)
#define N_FLAG (1 << 6)
#define H_FLAG (1 << 5)
#define C_FLAG (1 << 4)

#define SET_FLAGS(flags) registers.f |= (flags)
#define SET_CLEAR_COND(flags, cond) do { \
	if ((cond)) {\
		SET_FLAGS((flags)); }\
	else {\
		CLEAR_FLAGS((flags));}\
} while (0)
#define CLEAR_FLAGS(flags) registers.f &= ~(flags) & 0xF0
#define GET_FLAGS(flags) (registers.f & (flags) & 0xF0)
#define CLEAR_ALL() registers.f = 0x00;

#include <stdint.h>

// Hoera voor little endian
struct registers {
	struct {
		union {
			struct {
				uint8_t f;
				uint8_t a;
			};
			uint16_t af;
		};
	};
	
	struct {
		union {
			struct {
				uint8_t c;
				uint8_t b;
			};
			uint16_t bc;
		};
	};
	
		struct {
			union {
				struct {
					uint8_t e;
					uint8_t d;
				};
				uint16_t de;
			};
	};
	
		struct {
			union {
				struct {
					uint8_t l;
					uint8_t h;
				};
				uint16_t hl;
			};
	};
	
	uint16_t pc; // Program counter
	uint16_t sp; // Stack pointer
} extern registers;