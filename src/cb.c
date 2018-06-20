#include "cb.h"

uint8_t hlp;

#define SET(r, b) (r) |= (1 << (b))
#define RES(r, b) (r) &= ~(1 << (b))

uint8_t rlc_r(uint8_t reg) {
    uint8_t bit_seven = reg & (1 << 7);
    reg <<= 1;
    if (bit_seven) { SET_FLAGS(C_FLAG); reg |= (1 << 0); }
    else { CLEAR_FLAGS(C_FLAG); }
    SET_CLEAR_COND(Z_FLAG, !reg);
    CLEAR_FLAGS(N_FLAG | H_FLAG);
    return reg;
}

uint8_t rrc_r(uint8_t reg) {
    uint8_t bit_zero = reg & (1 << 0);
    reg >>= 1;
    if (bit_zero) { SET_FLAGS(C_FLAG); reg |= (1 << 7); }
    else { CLEAR_FLAGS(C_FLAG); }
    SET_CLEAR_COND(Z_FLAG, !reg);
    CLEAR_FLAGS(N_FLAG | H_FLAG);
    return reg;
}

uint8_t rl_r(uint8_t reg) {
    uint8_t bit_seven = reg & (1 << 7); 
    uint8_t bit_carry = GET_FLAGS(C_FLAG); 
    reg <<= 1; 
    SET_CLEAR_COND(C_FLAG, bit_seven); 
    if (bit_carry) { reg |= (1 << 0); } 
    SET_CLEAR_COND(Z_FLAG, !reg); 
    CLEAR_FLAGS(N_FLAG | H_FLAG); 
    return reg;
}

uint8_t rr_r(uint8_t reg) {
    uint8_t bit_zero = reg & (1 << 0); 
    uint8_t bit_carry = GET_FLAGS(C_FLAG); 
    reg >>= 1; 
    SET_CLEAR_COND(C_FLAG, bit_zero); 
    if (bit_carry) { reg |= (1 << 7); } 
    SET_CLEAR_COND(Z_FLAG, !reg); 
    CLEAR_FLAGS(N_FLAG | H_FLAG); 
    return reg;
}

uint8_t sla_r(uint8_t reg) {
    SET_CLEAR_COND(C_FLAG, reg & (1 << 7)); 
    reg <<= 1; 
    SET_CLEAR_COND(Z_FLAG, !reg); 
    CLEAR_FLAGS(N_FLAG | H_FLAG); 
    return reg;
}

uint8_t sra_r(uint8_t reg) {
        SET_CLEAR_COND(C_FLAG, reg & (1 << 0)); 
    uint8_t bit_seven = reg & (1 << 7); 
    reg >>= 1; 
    reg |= bit_seven; 
    SET_CLEAR_COND(Z_FLAG, !reg); 
    CLEAR_FLAGS(N_FLAG | H_FLAG); 
    return reg;
}

uint8_t swap_r(uint8_t reg) {
    uint8_t higher_nibble = reg & 0xF0; 
    reg <<= 4; 
    reg |= higher_nibble >> 4; 
    SET_CLEAR_COND(Z_FLAG, !reg); 
    CLEAR_FLAGS(N_FLAG | H_FLAG | C_FLAG); 
    return reg;
}

uint8_t srl_r(uint8_t reg) {
    SET_CLEAR_COND(C_FLAG, reg & (1 << 0)); 
    reg >>= 1; 
    SET_CLEAR_COND(Z_FLAG, !reg); 
    CLEAR_FLAGS(N_FLAG | H_FLAG); 
    return reg;
}

void bit(uint8_t reg, uint8_t bit) {
    SET_CLEAR_COND(Z_FLAG, !( reg & (1 << bit) )); 
    SET_FLAGS(H_FLAG); 
    CLEAR_FLAGS(N_FLAG); 
}

uint8_t ExecuteExInstruction(void) {
    uint8_t opcode = ReadByte(registers.pc);
    registers.pc += 1;

    uint8_t cycles = 8;

    if ((opcode & 0xF) == 0x06 || (opcode & 0xF) == 0x0E) {
         /* 
            BIT b, (HL) duurt slechts 12 cycles.
            Vanaf 0x46 t/m 0x7E.
         */
        if (opcode > 0x40 && opcode < 0x80) { 
            cycles += 4;
        } else {
            cycles += 8;
        }
    }

    switch (opcode) {
        case 0x00: registers.b = rlc_r(registers.b); break;
        case 0x01: registers.c = rlc_r(registers.c); break;
        case 0x02: registers.d = rlc_r(registers.d); break;
        case 0x03: registers.e = rlc_r(registers.e); break;
        case 0x04: registers.h = rlc_r(registers.h); break;
        case 0x05: registers.l = rlc_r(registers.l); break;
        case 0x06: 
            hlp = ReadByte(registers.hl);
            hlp = rlc_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x07: registers.a = rlc_r(registers.a); break;
        case 0x08: registers.b = rrc_r(registers.b); break; 
        case 0x09: registers.c = rrc_r(registers.c); break; 
        case 0x0A: registers.d = rrc_r(registers.d); break; 
        case 0x0B: registers.e = rrc_r(registers.e); break; 
        case 0x0C: registers.h = rrc_r(registers.h); break; 
        case 0x0D: registers.l = rrc_r(registers.l); break; 
        case 0x0E: 
            hlp = ReadByte(registers.hl);
            hlp = rrc_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x0F: registers.a = rrc_r(registers.a); break; 
        case 0x10: registers.b = rl_r(registers.b); break;
        case 0x11: registers.c = rl_r(registers.c); break;
        case 0x12: registers.d = rl_r(registers.d); break;
        case 0x13: registers.e = rl_r(registers.e); break;
        case 0x14: registers.h = rl_r(registers.h); break;
        case 0x15: registers.l = rl_r(registers.l); break;
        case 0x16:
            hlp = ReadByte(registers.hl);
            hlp = rl_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x17: registers.a = rl_r(registers.a); break;
        case 0x18: registers.b = rr_r(registers.b); break;
        case 0x19: registers.c = rr_r(registers.c); break;
        case 0x1A: registers.d = rr_r(registers.d); break;
        case 0x1B: registers.e = rr_r(registers.e); break;
        case 0x1C: registers.h = rr_r(registers.h); break;
        case 0x1D: registers.l = rr_r(registers.l); break;
        case 0x1E:
            hlp = ReadByte(registers.hl);
            hlp = rr_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x1F: registers.a = rr_r(registers.a); break;
        case 0x20: registers.b = sla_r(registers.b); break;
        case 0x21: registers.c = sla_r(registers.c); break;
        case 0x22: registers.d = sla_r(registers.d); break;
        case 0x23: registers.e = sla_r(registers.e); break;
        case 0x24: registers.h = sla_r(registers.h); break;
        case 0x25: registers.l = sla_r(registers.l); break;
        case 0x26:
            hlp = ReadByte(registers.hl);
            hlp = sla_r(hlp);
            WriteByte(registers.hl, hlp);
            break;        
        case 0x27: registers.a = sla_r(registers.a); break;
        case 0x28: registers.b = sra_r(registers.b); break;
        case 0x29: registers.c = sra_r(registers.c); break;
        case 0x2A: registers.d = sra_r(registers.d); break;
        case 0x2B: registers.e = sra_r(registers.e); break;
        case 0x2C: registers.h = sra_r(registers.h); break;
        case 0x2D: registers.l = sra_r(registers.l); break;
        case 0x2E:
            hlp = ReadByte(registers.hl);
            hlp = sra_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x2F: registers.a = sra_r(registers.a); break;
        case 0x30: registers.b = swap_r(registers.b); break;
        case 0x31: registers.c = swap_r(registers.c); break;
        case 0x32: registers.d = swap_r(registers.d); break;
        case 0x33: registers.e = swap_r(registers.e); break;
        case 0x34: registers.h = swap_r(registers.h); break;
        case 0x35: registers.l = swap_r(registers.l); break;
        case 0x36:
            hlp = ReadByte(registers.hl);
            hlp = swap_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x37: registers.a = swap_r(registers.a); break;
        case 0x38: registers.b = srl_r(registers.b); break;
        case 0x39: registers.c = srl_r(registers.c); break;
        case 0x3A: registers.d = srl_r(registers.d); break;
        case 0x3B: registers.e = srl_r(registers.e); break;
        case 0x3C: registers.h = srl_r(registers.h); break;
        case 0x3D: registers.l = srl_r(registers.l); break;
        case 0x3E:
            hlp = ReadByte(registers.hl);
            hlp = srl_r(hlp);
            WriteByte(registers.hl, hlp);
            break;
        case 0x3F: registers.a = srl_r(registers.a); break;
        case 0x40: bit(registers.b, 0); break;
        case 0x41: bit(registers.c, 0); break;
        case 0x42: bit(registers.d, 0); break;
        case 0x43: bit(registers.e, 0); break;
        case 0x44: bit(registers.h, 0); break;
        case 0x45: bit(registers.l, 0); break;
        case 0x46:
            hlp = ReadByte(registers.hl);
            bit(hlp, 0);
            break;
        case 0x47: bit(registers.a, 0); break;
        case 0x48: bit(registers.b, 1); break;
        case 0x49: bit(registers.c, 1); break;
        case 0x4A: bit(registers.d, 1); break;
        case 0x4B: bit(registers.e, 1); break;
        case 0x4C: bit(registers.h, 1); break;
        case 0x4D: bit(registers.l, 1); break;
        case 0x4E:
            hlp = ReadByte(registers.hl);
            bit(hlp, 1);
            break;
        case 0x4F: bit(registers.a, 1); break;
        case 0x50: bit(registers.b, 2); break;
        case 0x51: bit(registers.c, 2); break;
        case 0x52: bit(registers.d, 2); break;
        case 0x53: bit(registers.e, 2); break;
        case 0x54: bit(registers.h, 2); break;
        case 0x55: bit(registers.l, 2); break;
        case 0x56:
            hlp = ReadByte(registers.hl);
            bit(hlp, 2);
            break;
        case 0x57: bit(registers.a, 2); break;
        case 0x58: bit(registers.b, 3); break;
        case 0x59: bit(registers.c, 3); break;
        case 0x5A: bit(registers.d, 3); break;
        case 0x5B: bit(registers.e, 3); break;
        case 0x5C: bit(registers.h, 3); break;
        case 0x5D: bit(registers.l, 3); break;
        case 0x5E:
            hlp = ReadByte(registers.hl);
            bit(hlp, 3);
            break;
        case 0x5F: bit(registers.a, 3); break;
        case 0x60: bit(registers.b, 4); break;
        case 0x61: bit(registers.c, 4); break;
        case 0x62: bit(registers.d, 4); break;
        case 0x63: bit(registers.e, 4); break;
        case 0x64: bit(registers.h, 4); break;
        case 0x65: bit(registers.l, 4); break;
        case 0x66:
            hlp = ReadByte(registers.hl);
            bit(hlp, 4);
            break;
        case 0x67: bit(registers.a, 4); break;
        case 0x68: bit(registers.b, 5); break;
        case 0x69: bit(registers.c, 5); break;
        case 0x6A: bit(registers.d, 5); break;
        case 0x6B: bit(registers.e, 5); break;
        case 0x6C: bit(registers.h, 5); break;
        case 0x6D: bit(registers.l, 5); break;
        case 0x6E:
            hlp = ReadByte(registers.hl);
            bit(hlp, 5);
            break;
        case 0x6F: bit(registers.a, 5); break;
        case 0x70: bit(registers.b, 6); break;
        case 0x71: bit(registers.c, 6); break;
        case 0x72: bit(registers.d, 6); break;
        case 0x73: bit(registers.e, 6); break;
        case 0x74: bit(registers.h, 6); break;
        case 0x75: bit(registers.l, 6); break;
        case 0x76:
            hlp = ReadByte(registers.hl);
            bit(hlp, 6);
            break;
        case 0x77: bit(registers.a, 6); break;
        case 0x78: bit(registers.b, 7); break;
        case 0x79: bit(registers.c, 7); break;
        case 0x7A: bit(registers.d, 7); break;
        case 0x7B: bit(registers.e, 7); break;
        case 0x7C: bit(registers.h, 7); break;
        case 0x7D: bit(registers.l, 7); break;
        case 0x7E:
            hlp = ReadByte(registers.hl);
            bit(hlp, 7);
            break;
        case 0x7F: bit(registers.a, 7); break;
        case 0x80: RES(registers.b, 0); break;
        case 0x81: RES(registers.c, 0); break;
        case 0x82: RES(registers.d, 0); break;
        case 0x83: RES(registers.e, 0); break;
        case 0x84: RES(registers.h, 0); break;
        case 0x85: RES(registers.l, 0); break;
        case 0x86:
            hlp = ReadByte(registers.hl);
            RES(hlp, 0);
            WriteByte(registers.hl, hlp);
            break;
        case 0x87: RES(registers.a, 0); break;
        case 0x88: RES(registers.b, 1); break;
        case 0x89: RES(registers.c, 1); break;
        case 0x8A: RES(registers.d, 1); break;
        case 0x8B: RES(registers.e, 1); break;
        case 0x8C: RES(registers.h, 1); break;
        case 0x8D: RES(registers.l, 1); break;
        case 0x8E:
            hlp = ReadByte(registers.hl);
            RES(hlp, 1);
            WriteByte(registers.hl, hlp);
            break;
        case 0x8F: RES(registers.a, 1); break;
        case 0x90: RES(registers.b, 2); break;
        case 0x91: RES(registers.c, 2); break;
        case 0x92: RES(registers.d, 2); break;
        case 0x93: RES(registers.e, 2); break;
        case 0x94: RES(registers.h, 2); break;
        case 0x95: RES(registers.l, 2); break;
        case 0x96:
            hlp = ReadByte(registers.hl);
            RES(hlp, 2);
            WriteByte(registers.hl, hlp);
            break;
        case 0x97: RES(registers.a, 2); break;
        case 0x98: RES(registers.b, 3); break;
        case 0x99: RES(registers.c, 3); break;
        case 0x9A: RES(registers.d, 3); break;
        case 0x9B: RES(registers.e, 3); break;
        case 0x9C: RES(registers.h, 3); break;
        case 0x9D: RES(registers.l, 3); break;
        case 0x9E:
            hlp = ReadByte(registers.hl);
            RES(hlp, 3);
            WriteByte(registers.hl, hlp);
            break;
        case 0x9F: RES(registers.a, 3); break;
        case 0xA0: RES(registers.b, 4); break;
        case 0xA1: RES(registers.c, 4); break;
        case 0xA2: RES(registers.d, 4); break;
        case 0xA3: RES(registers.e, 4); break;
        case 0xA4: RES(registers.h, 4); break;
        case 0xA5: RES(registers.l, 4); break;
        case 0xA6:
            hlp = ReadByte(registers.hl);
            RES(hlp, 4);
            WriteByte(registers.hl, hlp);
            break;
        case 0xA7: RES(registers.a, 4); break;
        case 0xA8: RES(registers.b, 5); break;
        case 0xA9: RES(registers.c, 5); break;
        case 0xAA: RES(registers.d, 5); break;
        case 0xAB: RES(registers.e, 5); break;
        case 0xAC: RES(registers.h, 5); break;
        case 0xAD: RES(registers.l, 5); break;
        case 0xAE:
            hlp = ReadByte(registers.hl);
            RES(hlp, 5);
            WriteByte(registers.hl, hlp);
            break;
        case 0xAF: RES(registers.a, 5); break;
        case 0xB0: RES(registers.b, 6); break;
        case 0xB1: RES(registers.c, 6); break;
        case 0xB2: RES(registers.d, 6); break;
        case 0xB3: RES(registers.e, 6); break;
        case 0xB4: RES(registers.h, 6); break;
        case 0xB5: RES(registers.l, 6); break;
        case 0xB6:
            hlp = ReadByte(registers.hl);
            RES(hlp, 6);
            WriteByte(registers.hl, hlp);
            break;
        case 0xB7: RES(registers.a, 6); break;
        case 0xB8: RES(registers.b, 7); break;
        case 0xB9: RES(registers.c, 7); break;
        case 0xBA: RES(registers.d, 7); break;
        case 0xBB: RES(registers.e, 7); break;
        case 0xBC: RES(registers.h, 7); break;
        case 0xBD: RES(registers.l, 7); break;
        case 0xBE:
            hlp = ReadByte(registers.hl);
            RES(hlp, 7);
            WriteByte(registers.hl, hlp);
            break;
        case 0xBF: RES(registers.a, 7); break;
        case 0xC0: SET(registers.b, 0); break;
        case 0xC1: SET(registers.c, 0); break;
        case 0xC2: SET(registers.d, 0); break;
        case 0xC3: SET(registers.e, 0); break;
        case 0xC4: SET(registers.h, 0); break;
        case 0xC5: SET(registers.l, 0); break;
        case 0xC6:
            hlp = ReadByte(registers.hl);
            SET(hlp, 0);
            WriteByte(registers.hl, hlp);
            break;
        case 0xC7: SET(registers.a, 0); break;
        case 0xC8: SET(registers.b, 1); break;
        case 0xC9: SET(registers.c, 1); break;
        case 0xCA: SET(registers.d, 1); break;
        case 0xCB: SET(registers.e, 1); break;
        case 0xCC: SET(registers.h, 1); break;
        case 0xCD: SET(registers.l, 1); break;
        case 0xCE: 
            hlp = ReadByte(registers.hl);
            SET(hlp, 1);
            WriteByte(registers.hl, hlp);
            break;
        case 0xCF: SET(registers.a, 1); break;
        case 0xD0: SET(registers.b, 2); break;
        case 0xD1: SET(registers.c, 2); break;
        case 0xD2: SET(registers.d, 2); break;
        case 0xD3: SET(registers.e, 2); break;
        case 0xD4: SET(registers.h, 2); break;
        case 0xD5: SET(registers.l, 2); break;
        case 0xD6:
            hlp = ReadByte(registers.hl);
            SET(hlp, 2);
            WriteByte(registers.hl, hlp);
            break;
        case 0xD7: SET(registers.a, 2); break;
        case 0xD8: SET(registers.b, 3); break;
        case 0xD9: SET(registers.c, 3); break;
        case 0xDA: SET(registers.d, 3); break;
        case 0xDB: SET(registers.e, 3); break;
        case 0xDC: SET(registers.h, 3); break;
        case 0xDD: SET(registers.l, 3); break;
        case 0xDE:
            hlp = ReadByte(registers.hl);
            SET(hlp, 3);
            WriteByte(registers.hl, hlp);
            break;
        case 0xDF: SET(registers.a, 3); break;
        case 0xE0: SET(registers.b, 4); break;
        case 0xE1: SET(registers.c, 4); break;
        case 0xE2: SET(registers.d, 4); break;
        case 0xE3: SET(registers.e, 4); break;
        case 0xE4: SET(registers.h, 4); break;
        case 0xE5: SET(registers.l, 4); break;
        case 0xE6:
            hlp = ReadByte(registers.hl);
            SET(hlp, 4);
            WriteByte(registers.hl, hlp);
            break;
        case 0xE7: SET(registers.a, 4); break;
        case 0xE8: SET(registers.b, 5); break;
        case 0xE9: SET(registers.c, 5); break;
        case 0xEA: SET(registers.d, 5); break;
        case 0xEB: SET(registers.e, 5); break;
        case 0xEC: SET(registers.h, 5); break;
        case 0xED: SET(registers.l, 5); break;
        case 0xEE:
            hlp = ReadByte(registers.hl);
            SET(hlp, 5);
            WriteByte(registers.hl, hlp);
            break;
        case 0xEF: SET(registers.a, 5); break;
        case 0xF0: SET(registers.b, 6); break;
        case 0xF1: SET(registers.c, 6); break;
        case 0xF2: SET(registers.d, 6); break;
        case 0xF3: SET(registers.e, 6); break;
        case 0xF4: SET(registers.h, 6); break;
        case 0xF5: SET(registers.l, 6); break;
        case 0xF6:
            hlp = ReadByte(registers.hl);
            SET(hlp, 6);
            WriteByte(registers.hl, hlp);
            break;
        case 0xF7: SET(registers.a, 6); break;
        case 0xF8: SET(registers.b, 7); break;
        case 0xF9: SET(registers.c, 7); break;
        case 0xFA: SET(registers.d, 7); break;
        case 0xFB: SET(registers.e, 7); break;
        case 0xFC: SET(registers.h, 7); break;
        case 0xFD: SET(registers.l, 7); break;
        case 0xFE:
            hlp = ReadByte(registers.hl);
            SET(hlp, 7);
            WriteByte(registers.hl, hlp);
            break;
        case 0xFF: SET(registers.a, 7); break;
    }

    return cycles;
}