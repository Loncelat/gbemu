#include "cpu.h"

uint8_t t_cycles = 0;
uint8_t enableIMENextClock = 0;

struct cpu cpu = {
    NOT_HALTED, 0, 0
};

void ResetCPU(void) {
    registers.af = 0x01B0;
    registers.bc = 0x0013;
    registers.de = 0x00D8;
    registers.hl = 0x014D;
    registers.sp = 0xFFFE;
    registers.pc = 0x0100;

    cpu.halted = NOT_HALTED;
    cpu.bugged = 0;
    cpu.stopped = 0;

    ResetIO();
    
    timer.div = 0xABCC;
    timer.enabled = 0;
    timer.frequency = 0;

    gpu.ly_equals_lyc = 0x1;
    gpu.mode = VBLANK;
    gpu.cycles = 25;

    mbc.romBank = 1;
    mbc.ramBank = 0;

    keys.A = keys.B = keys.Select = keys.Start = 1;
    keys.Up = keys.Down = keys.Left = keys.Right = 1;

    // Zet IE naar 0x00.
    WriteByte(0xFFFF, 0x00);

}

void PrintRegisters(void) {
    printf("AF: %04X\n", registers.af);
    printf("BC: %04X\n", registers.bc);
    printf("DE: %04X\n", registers.de);
    printf("HL: %04X\n", registers.hl);
    printf("SP: %04X\n", registers.sp);
    printf("PC: %04X\n", registers.pc);
    printf("LCDC: %02X\n", *gpu.control);
    printf("STAT: %02X\n", ReadByte(0xFF41));
    printf("DIV: %02X\n", timer.div);
    printf("LY: %02X\n", ReadIO(0x44));
    printf("CYCLES: %03i\n", gpu.cycles);
    printf("MODE: %02X\n", gpu.mode);
    // printf("TIMA: %02X\n", *timer.tima);
    // printf("TMA: %02X\n", *timer.tma);
    // printf("TAC: %02X\n", ReadByte(0xFF07));
    // printf("ROM: %02X\n\n", mbc.romBank);
}

void CPUCycle(void) {

    register uint8_t opcode;
    t_cycles = 0;
    
    if (CheckAndHandleInterrupts()) {
        t_cycles += (cpu.halted ? 24 : 20);
        return;
    }

    if (enableIMENextClock) { IME = 1; enableIMENextClock = 0; }

    if (cpu.halted) {
        t_cycles += 4;
        return;
    }

    if (cpu.bugged) {
        opcode = ReadByte(registers.pc);
        cpu.bugged = 0;
    } else {
        opcode = ReadByte(registers.pc);
        registers.pc++;
    }

    t_cycles += ExecuteInstruction(opcode);

}