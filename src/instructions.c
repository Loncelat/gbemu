#include "instructions.h"

uint8_t bit_seven, bit_zero, bit_carry;

uint8_t data;
int8_t signedValue;
uint32_t result;

const uint8_t instructionCycles[256] = {
     4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8, 8,  4,  4, 8,  4,
     4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8, 8,  4,  4, 8,  4,
     8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8, 8,  4,  4, 8,  4,
     8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8, 8,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4, 4,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4, 4,  4,  4, 8,  4,
     8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12, 0, 12, 24, 8, 16,
     8, 12, 12,  0, 12, 16,  8, 16,  8, 16, 12, 0, 12,  0, 8, 16,
    12, 12,  8,  0,  0, 16,  8, 16, 16,  4, 16, 0,  0,  0, 8, 16,
    12, 12,  8,  4,  0, 16,  8, 16, 12,  8, 16, 4,  0,  0, 8, 16
};

#define LD_R_N(r) do { \
   (r) = ReadByte(registers.pc); \
   registers.pc++; \
} while (0)

#define LD_RR_NN(rr) do { \
   (rr) = ReadShort(registers.pc); \
   registers.pc += 2; \
} while (0)
    
#define JR_COND(cond) do { \
    if ( (cond) ) { \
        registers.pc += (int8_t) ReadByte(registers.pc) + 1; \
        cycles += 4; \
    } else { \
     registers.pc++; \
    } \
} while (0)
    
#define JP_COND(cond) do { \
   if ( (cond) ) { \
       registers.pc = ReadShort(registers.pc); \
       cycles += 4; \
   } else { \
       registers.pc += 2; \
   } \
} while (0)
    
#define RET_COND(cond) do { \
    if ( (cond) ) { \
        registers.pc = ReadStack(); \
        cycles += 12; \
    } \
} while (0)
    
#define CALL_COND(cond) do { \
    if ( (cond) ) { \
        WriteStack(registers.pc + 2); \
        registers.pc = ReadShort(registers.pc); \
        cycles += 12; \
    } else { \
     registers.pc += 2; \
    } \
} while (0)
    
#define RST(n) do { \
    WriteStack(registers.pc); \
    registers.pc = (n); \
} while (0)

uint8_t inc(uint8_t reg) {
   SET_CLEAR_COND(H_FLAG, (reg & 0x0F) == 0x0F);
   reg++;
   SET_CLEAR_COND(Z_FLAG, !reg);
   CLEAR_FLAGS(N_FLAG);
   return reg;
}
    
uint8_t dec(uint8_t reg) {
    SET_CLEAR_COND(H_FLAG, !(reg & 0x0F));
    reg--;
    SET_CLEAR_COND(Z_FLAG, !reg);
    SET_FLAGS(N_FLAG);
    return reg;
}

void add_n(uint8_t data) {
    uint16_t result = registers.a + data;
    SET_CLEAR_COND(Z_FLAG, !(result & 0xFF));
    SET_CLEAR_COND(H_FLAG, ((registers.a & 0xF) + (data & 0xF)) & 0x10);
    SET_CLEAR_COND(C_FLAG, result & 0xFF00);
    CLEAR_FLAGS(N_FLAG);
    registers.a = (uint8_t) result;
}

void add_nn(uint16_t data) {
    uint32_t result = registers.hl + data;
    SET_CLEAR_COND(H_FLAG, ((registers.hl & 0x0FFF) + (data & 0x0FFF)) & 0x1000);
    SET_CLEAR_COND(C_FLAG, result & 0xFFFF0000);
    CLEAR_FLAGS(N_FLAG);
    registers.hl = (uint16_t) result;
}

void sub(uint8_t data) {
    SET_CLEAR_COND(C_FLAG, data > registers.a);
    SET_CLEAR_COND(H_FLAG, (data & 0x0F) > (registers.a & 0x0F));
    registers.a -= data;
    SET_CLEAR_COND(Z_FLAG, !registers.a);
    SET_FLAGS(N_FLAG);
}

void adc(uint8_t data) {
    uint8_t carry_set = GET_FLAGS(C_FLAG) ? 1 : 0;	
	uint16_t result = registers.a + data + carry_set;
	
    SET_CLEAR_COND(C_FLAG, result & 0xFF00);
    SET_CLEAR_COND(H_FLAG, ((data & 0xF) + (registers.a & 0xF) + carry_set) > 0xF);

    registers.a = result & 0xFF;

    SET_CLEAR_COND(Z_FLAG, !(registers.a));	
	CLEAR_FLAGS(N_FLAG);
}

void sbc(uint8_t data) {
    uint8_t carry_set = GET_FLAGS(C_FLAG) ? 1 : 0;	
    int16_t result = registers.a - data - carry_set;
    
    SET_CLEAR_COND(H_FLAG, (int16_t) (registers.a & 0xF) - (data & 0xF) - carry_set < 0);
    SET_CLEAR_COND(C_FLAG, result < 0);

    registers.a = (uint8_t) result;
    
    SET_CLEAR_COND(Z_FLAG, !(registers.a));	
    SET_FLAGS(N_FLAG);
}

void and(uint8_t data) {
    registers.a &= data;
    CLEAR_ALL();
    SET_FLAGS(H_FLAG);
    if (registers.a == 0x00) { SET_FLAGS(Z_FLAG); }
}

void or(uint8_t data) {
    registers.a |= data;
    CLEAR_ALL();
    if (registers.a == 0x00) { SET_FLAGS(Z_FLAG); }
}

void xor(uint8_t data) {
    registers.a ^= data;
    CLEAR_ALL();
    if (registers.a == 0x00) { SET_FLAGS(Z_FLAG); }
}

void cp(uint8_t data) {
    SET_CLEAR_COND(Z_FLAG, registers.a == data);
    SET_CLEAR_COND(C_FLAG, data > registers.a);
    SET_CLEAR_COND(H_FLAG, (data & 0x0F) > (registers.a & 0x0F));
    SET_FLAGS(N_FLAG);
}

void undefined(uint8_t opcode) { 
    printf("Encountered illegal opcode %X at address %X\n", opcode, registers.pc - 1);
    PrintRegisters(); 
}

uint8_t ExecuteInstruction(uint8_t opcode) {

    uint8_t cycles = 0;
    
    switch (opcode) {
        case 0x00: break;
        case 0x01: LD_RR_NN(registers.bc); break;
        case 0x02: WriteByte(registers.bc, registers.a); break;
        case 0x03: registers.bc++; break;
        case 0x04: registers.b = inc(registers.b);  break;
        case 0x05: registers.b = dec(registers.b); break;
        case 0x06: LD_R_N(registers.b); break;
        case 0x07:; 
            bit_seven = registers.a & (1 << 7); // Extraheer bit 7.
            registers.a <<= 1; // Shift A een naar links.
            
            CLEAR_ALL();

            // Zet de carry en bit 0 van A naar de oude 7de bit van A
            if (bit_seven) { SET_FLAGS(C_FLAG); registers.a |= (1 << 0); }
            else { CLEAR_FLAGS(C_FLAG); }
            
            break;
        case 0x08: 
            WriteShort(ReadShort(registers.pc), registers.sp); 
            registers.pc += 2;
            break;
        case 0x09: add_nn(registers.bc); break;
        case 0x0A: registers.a = ReadByte(registers.bc); break;
        case 0x0B: registers.bc--; break;
        case 0x0C: registers.c = inc(registers.c); break;
        case 0x0D: registers.c = dec(registers.c); break;
        case 0x0E: LD_R_N(registers.c); break;
        case 0x0F:;
            bit_zero = registers.a & (1 << 0); // Extraheer bit 0.
            registers.a >>= 1; // Shift A een naar rechts.
            
            CLEAR_ALL();

            // Zet de carry en bit 0 van A naar de oude 7de bit van A
            if (bit_zero) { SET_FLAGS(C_FLAG); registers.a |= (1 << 7); }
            
            break;
        case 0x10: cpu.halted = HALTED; registers.pc += 1; break;
        case 0x11: LD_RR_NN(registers.de); break;
        case 0x12: WriteByte(registers.de, registers.a); break;
        case 0x13: registers.de++; break;
        case 0x14: registers.d = inc(registers.d); break;
        case 0x15: registers.d = dec(registers.d); break;
        case 0x16: LD_R_N(registers.d); break;
        case 0x17:; 
            bit_seven = registers.a & (1 << 7); // Extraheer bit 7.
            bit_carry = registers.f & (1 << 4); // Extraheer de carry. (bit 5 van F)
            registers.a <<= 1; // Shift A een naar links.

            CLEAR_ALL();
            
            // Zet de carry en bit 0 van A naar de oude 7de bit van A.
            SET_CLEAR_COND(C_FLAG, bit_seven);

            // Sla de oude carry op in de 0de bit van A.
            if(bit_carry) { registers.a |= (1 << 0); }
            break;
        case 0x18: registers.pc += (int8_t) ReadByte(registers.pc) + 1; break;
        case 0x19: add_nn(registers.de); break;
        case 0x1A: registers.a = ReadByte(registers.de); break;
        case 0x1B: registers.de--; break;
        case 0x1C: registers.e = inc(registers.e); break;
        case 0x1D: registers.e = dec(registers.e); break;
        case 0x1E: LD_R_N(registers.e); break;
        case 0x1F:; 
            bit_zero = registers.a & (1 << 0); // Extraheer bit 0.
            bit_carry = registers.f & 0x10; // Extraheer de carry. (bit 5 van F)
            registers.a >>= 1; // Shift A een naar rechts.

            CLEAR_ALL();
            // Zet de carry naar de oude 0de bit van A.
            SET_CLEAR_COND(C_FLAG, bit_zero);

            // Sla de oude carry op in de 7de bit van A.
            if (bit_carry) { registers.a |= (1 << 7); }
            break;
        case 0x20: JR_COND(!GET_FLAGS(Z_FLAG)); break;
        case 0x21: LD_RR_NN(registers.hl); break;
        case 0x22: WriteByte(registers.hl, registers.a); registers.hl++; break;
        case 0x23: registers.hl++; break;
        case 0x24: registers.h = inc(registers.h); break;
        case 0x25: registers.h = dec(registers.h); break;
        case 0x26: LD_R_N(registers.h); break;
        case 0x27: 
                if (!GET_FLAGS(N_FLAG)) { // Als de vorige berekening optellen was.
                if (GET_FLAGS(C_FLAG) || (registers.a > 0x99)) { registers.a += 0x60; SET_FLAGS(C_FLAG); }
                if (GET_FLAGS(H_FLAG) || (registers.a & 0x0F) > 0x09) { registers.a += 0x06; }
            } else { // Als de vorige berekening aftrekken was.
                if (GET_FLAGS(C_FLAG)) { registers.a -= 0x60; }
                if (GET_FLAGS(H_FLAG)) { registers.a -= 0x06; }
            }

            // H wordt gereset.
            CLEAR_FLAGS(H_FLAG);

            SET_CLEAR_COND(Z_FLAG, !registers.a);
            break;
        case 0x28: JR_COND(GET_FLAGS(Z_FLAG)); break;
        case 0x29: add_nn(registers.hl); break;
        case 0x2A: registers.a = ReadByte(registers.hl); registers.hl++; break;
        case 0x2B: registers.hl--; break;
        case 0x2C: registers.l = inc(registers.l); break;
        case 0x2D: registers.l = dec(registers.l); break;
        case 0x2E: LD_R_N(registers.l); break;
        case 0x2F: registers.a = ~registers.a; SET_FLAGS(N_FLAG | H_FLAG); break;
        case 0x30: JR_COND(!GET_FLAGS(C_FLAG)); break;
        case 0x31: LD_RR_NN(registers.sp); break;
        case 0x32: WriteByte(registers.hl, registers.a); registers.hl--; break;
        case 0x33: registers.sp++; break;
        case 0x34: WriteByte( registers.hl, inc(ReadByte(registers.hl)) ); break;
        case 0x35: WriteByte( registers.hl, dec(ReadByte(registers.hl)) );break;
        case 0x36: WriteByte(registers.hl, ReadByte(registers.pc)); registers.pc++; break;
        case 0x37: SET_FLAGS(C_FLAG); CLEAR_FLAGS(N_FLAG | H_FLAG); break;
        case 0x38: JR_COND(GET_FLAGS(C_FLAG)); break;
        case 0x39: add_nn(registers.sp); break;
        case 0x3A: registers.a = ReadByte(registers.hl); registers.hl--; break;
        case 0x3B: registers.sp--; break;
        case 0x3C: registers.a = inc(registers.a);break;
        case 0x3D: registers.a = dec(registers.a);break;
        case 0x3E: LD_R_N(registers.a); break;
        case 0x3F: 
            registers.f ^= (1 << 4);
            CLEAR_FLAGS(N_FLAG | H_FLAG);
            break;
        case 0x40: break;
        case 0x41: registers.b = registers.c; break;
        case 0x42: registers.b = registers.d; break;
        case 0x43: registers.b = registers.e; break;
        case 0x44: registers.b = registers.h; break;
        case 0x45: registers.b = registers.l; break;
        case 0x46: registers.b = ReadByte(registers.hl); break;
        case 0x47: registers.b = registers.a; break;
        case 0x48: registers.c = registers.b; break;
        case 0x49: break;
        case 0x4A: registers.c = registers.d; break;
        case 0x4B: registers.c = registers.e; break;
        case 0x4C: registers.c = registers.h; break;
        case 0x4D: registers.c = registers.l; break;
        case 0x4E: registers.c = ReadByte(registers.hl); break;
        case 0x4F: registers.c = registers.a; break;
        case 0x50: registers.d = registers.b; break;
        case 0x51: registers.d = registers.c; break;
        case 0x52: break;
        case 0x53: registers.d = registers.e; break;
        case 0x54: registers.d = registers.h; break;
        case 0x55: registers.d = registers.l; break;
        case 0x56: registers.d = ReadByte(registers.hl); break;
        case 0x57: registers.d = registers.a; break;
        case 0x58: registers.e = registers.b; break;
        case 0x59: registers.e = registers.c; break;
        case 0x5A: registers.e = registers.d; break;
        case 0x5B: break;
        case 0x5C: registers.e = registers.h; break;
        case 0x5D: registers.e = registers.l; break;
        case 0x5E: registers.e = ReadByte(registers.hl); break;
        case 0x5F: registers.e = registers.a; break;
        case 0x60: registers.h = registers.b; break;
        case 0x61: registers.h = registers.c; break;
        case 0x62: registers.h = registers.d; break;
        case 0x63: registers.h = registers.e; break;
        case 0x64: break;
        case 0x65: registers.h = registers.l; break;
        case 0x66: registers.h = ReadByte(registers.hl); break;
        case 0x67: registers.h = registers.a; break;
        case 0x68: registers.l = registers.b; break;
        case 0x69: registers.l = registers.c; break;
        case 0x6A: registers.l = registers.d; break;
        case 0x6B: registers.l = registers.e; break;
        case 0x6C: registers.l = registers.h; break;
        case 0x6D: break;
        case 0x6E: registers.l = ReadByte(registers.hl); break;
        case 0x6F: registers.l = registers.a; break;
        case 0x70: WriteByte(registers.hl, registers.b); break;
        case 0x71: WriteByte(registers.hl, registers.c); break;
        case 0x72: WriteByte(registers.hl, registers.d); break;
        case 0x73: WriteByte(registers.hl, registers.e); break;
        case 0x74: WriteByte(registers.hl, registers.h); break;
        case 0x75: WriteByte(registers.hl, registers.l); break;
        case 0x76:
            cpu.halted = HALTED;
            if ( (IME == 0) && (INTERRUPT_FLAGS & INTERRUPT_ENABLE & 0x1F )) {
                cpu.bugged = 1;
            }
            break;
        case 0x77: WriteByte(registers.hl, registers.a); break;
        case 0x78: registers.a = registers.b; break;
        case 0x79: registers.a = registers.c; break;
        case 0x7A: registers.a = registers.d; break;
        case 0x7B: registers.a = registers.e; break;
        case 0x7C: registers.a = registers.h; break;
        case 0x7D: registers.a = registers.l; break;
        case 0x7E: registers.a = ReadByte(registers.hl); break;
        case 0x7F: break;
        case 0x80: add_n(registers.b); break;
        case 0x81: add_n(registers.c); break;
        case 0x82: add_n(registers.d); break;
        case 0x83: add_n(registers.e); break;
        case 0x84: add_n(registers.h); break;
        case 0x85: add_n(registers.l); break;
        case 0x86: add_n(ReadByte(registers.hl)); break;
        case 0x87: add_n(registers.a); break;
        case 0x88: adc(registers.b); break;
        case 0x89: adc(registers.c); break;
        case 0x8A: adc(registers.d); break;
        case 0x8B: adc(registers.e); break;
        case 0x8C: adc(registers.h); break;
        case 0x8D: adc(registers.l); break;
        case 0x8E: adc(ReadByte(registers.hl)); break;
        case 0x8F: adc(registers.a); break;
        case 0x90: sub(registers.b); break;
        case 0x91: sub(registers.c); break;
        case 0x92: sub(registers.d); break;
        case 0x93: sub(registers.e); break;
        case 0x94: sub(registers.h); break;
        case 0x95: sub(registers.l); break;
        case 0x96: sub(ReadByte(registers.hl)); break;
        case 0x97: sub(registers.a); break;
        case 0x98: sbc(registers.b); break;
        case 0x99: sbc(registers.c); break;
        case 0x9A: sbc(registers.d); break;
        case 0x9B: sbc(registers.e); break;
        case 0x9C: sbc(registers.h); break;
        case 0x9D: sbc(registers.l); break;
        case 0x9E: sbc(ReadByte(registers.hl)); break;
        case 0x9F: sbc(registers.a); break;
        case 0xA0: and(registers.b); break;
        case 0xA1: and(registers.c); break;
        case 0xA2: and(registers.d); break;
        case 0xA3: and(registers.e); break;
        case 0xA4: and(registers.h); break;
        case 0xA5: and(registers.l); break;
        case 0xA6: and(ReadByte(registers.hl)); break;
        case 0xA7: and(registers.a); break;
        case 0xA8: xor(registers.b); break;
        case 0xA9: xor(registers.c); break;
        case 0xAA: xor(registers.d); break;
        case 0xAB: xor(registers.e); break;
        case 0xAC: xor(registers.h); break;
        case 0xAD: xor(registers.l); break;
        case 0xAE: xor(ReadByte(registers.hl)); break;
        case 0xAF: xor(registers.a); break;
        case 0xB0: or(registers.b); break;
        case 0xB1: or(registers.c); break;
        case 0xB2: or(registers.d); break;
        case 0xB3: or(registers.e); break;
        case 0xB4: or(registers.h); break;
        case 0xB5: or(registers.l); break;
        case 0xB6: or(ReadByte(registers.hl)); break;
        case 0xB7: or(registers.a); break;
        case 0xB8: cp(registers.b); break;
        case 0xB9: cp(registers.c); break;
        case 0xBA: cp(registers.d); break;
        case 0xBB: cp(registers.e); break;
        case 0xBC: cp(registers.h); break;
        case 0xBD: cp(registers.l); break;
        case 0xBE: cp(ReadByte(registers.hl)); break;
        case 0xBF: cp(registers.a); break;
        case 0xC0: RET_COND(!GET_FLAGS(Z_FLAG)); break;
        case 0xC1: registers.bc = ReadStack(); break;
        case 0xC2: JP_COND(!GET_FLAGS(Z_FLAG)); break;
        case 0xC3: registers.pc = ReadShort(registers.pc); break;
        case 0xC4: CALL_COND(!GET_FLAGS(Z_FLAG)); break;
        case 0xC5: WriteStack(registers.bc); break;
        case 0xC6: add_n(ReadByte(registers.pc)); registers.pc++; break;
        case 0xC7: RST(0x00); break;
        case 0xC8: RET_COND(GET_FLAGS(Z_FLAG)); break;
        case 0xC9: registers.pc = ReadStack(); break;
        case 0xCA: JP_COND(GET_FLAGS(Z_FLAG)); break;
        case 0xCB: cycles += ExecuteExInstruction(); break;
        case 0xCC: CALL_COND(GET_FLAGS(Z_FLAG)); break;
        case 0xCD: 
            WriteStack(registers.pc + 2);
            registers.pc = ReadShort(registers.pc);
            break;
        case 0xCE: adc(ReadByte(registers.pc)); registers.pc++; break;
        case 0xCF: RST(0x08); break;
        case 0xD0: RET_COND(!GET_FLAGS(C_FLAG)); break;
        case 0xD1: registers.de = ReadStack(); break;
        case 0xD2: JP_COND(!GET_FLAGS(C_FLAG)); break;
        case 0xD3: undefined(0xD3); break;
        case 0xD4: CALL_COND(!GET_FLAGS(C_FLAG)); break;
        case 0xD5: WriteStack(registers.de); break;
        case 0xD6: sub(ReadByte(registers.pc)); registers.pc++; break;
        case 0xD7: RST(0x10); break;
        case 0xD8: RET_COND(GET_FLAGS(C_FLAG)); break;
        case 0xD9: 
            registers.pc = ReadStack();
            IME = 1;
            break;
        case 0xDA: JP_COND(GET_FLAGS(C_FLAG)); break;
        case 0xDB: undefined(0xDB); break;
        case 0xDC: CALL_COND(GET_FLAGS(C_FLAG)); break;
        case 0xDD: undefined(0xDD); break;
        case 0xDE: sbc(ReadByte(registers.pc)); registers.pc++; break;
        case 0xDF: RST(0x18); break;
        case 0xE0: WriteByte(0xFF00 + ReadByte(registers.pc), registers.a); registers.pc++; break;
        case 0xE1: registers.hl = ReadStack(); break;
        case 0xE2: WriteByte(0xFF00 + registers.c, registers.a); break;
        case 0xE3: undefined(0xE3); break;
        case 0xE4: undefined(0xE4); break;
        case 0xE5: WriteStack(registers.hl); break;
        case 0xE6: and(ReadByte(registers.pc)); registers.pc++; break;
        case 0xE7: RST(0x20); break;
        case 0xE8:; 
            data = ReadByte(registers.pc);
            registers.pc++;
            
            signedValue = (int8_t) data;
            result = registers.sp + signedValue;
            
            SET_CLEAR_COND(H_FLAG, ((registers.sp & 0xF) + (data & 0xF)) & 0x10);
            SET_CLEAR_COND(C_FLAG, (registers.sp ^ signedValue ^ result) & 0x100);
            
            registers.sp = (uint16_t) result;
            CLEAR_FLAGS(Z_FLAG | N_FLAG);
            break;
        case 0xE9: registers.pc = registers.hl; break;
        case 0xEA: WriteByte(ReadShort(registers.pc), registers.a); registers.pc += 2; break;
        case 0xEB: undefined(0xEB); break;
        case 0xEC: undefined(0xEC); break;
        case 0xED: undefined(0xED); break;
        case 0xEE: xor(ReadByte(registers.pc)); registers.pc++; break;
        case 0xEF: RST(0x28); break;
        case 0xF0: registers.a = ReadByte(0xFF00 + ReadByte(registers.pc)); registers.pc++; break;
        case 0xF1: registers.af = ReadStack() & 0xFFF0; break;
        case 0xF2: registers.a = ReadByte(0xFF00 + registers.c);break;
        case 0xF3: IME = 0; break;
        case 0xF4: undefined(0xF4); break;
        case 0xF5: WriteStack(registers.af); break;
        case 0xF6: or(ReadByte(registers.pc)); registers.pc++; break;
        case 0xF7: RST(0x30); break;
        case 0xF8:; 
            data = ReadByte(registers.pc);
            registers.pc++;
            
            signedValue = (int8_t) data;
            result = registers.sp + signedValue;
            
            SET_CLEAR_COND(H_FLAG, ((registers.sp & 0xF) + (data & 0xF)) & 0x10);
            SET_CLEAR_COND(C_FLAG, (registers.sp ^ signedValue ^ result) & 0x100);
            
            registers.hl = (uint16_t) result;
            CLEAR_FLAGS(Z_FLAG | N_FLAG);
        break;
        case 0xF9: registers.sp = registers.hl; break;
        case 0xFA: registers.a = ReadByte(ReadShort(registers.pc)); registers.pc += 2; break;
        case 0xFB: enableIMENextClock = 1; break;
        case 0xFC: undefined(0xFC); break;
        case 0xFD: undefined(0xFD); break;
        case 0xFE: cp(ReadByte(registers.pc)); registers.pc++; break;
        case 0xFF: RST(0x38); break;
    }
    
    cycles += instructionCycles[opcode];
    return cycles;    
}