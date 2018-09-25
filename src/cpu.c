#include "cpu.h"

uint8_t t_cycles = 0;
uint8_t enableIMENextClock = 0;

struct cpu cpu = {
    NOT_HALTED, 0, 0
};

void ResetCPU(void) {

    ResetIO();

    if (mbc.useBootRom) {
        registers.af = 0;
        registers.bc = 0;
        registers.de = 0;
        registers.hl = 0;
        registers.sp = 0;
        registers.pc = 0;

        timer.div       = 0;
        timer.enabled   = 0;
        timer.frequency = 0;

        gpu.scanline    = 0;
        gpu.cycles      = 0;
        gpu.mode        = HBLANK;
        gpu.coincidence = 1;

    } else {
        registers.af = 0x01B0;
        registers.bc = 0x0013;
        registers.de = 0x00D8;
        registers.hl = 0x014D;
        registers.sp = 0xFFFE;
        registers.pc = 0x0100;

        cpu.halted  = NOT_HALTED;
        cpu.bugged  = 0;
        cpu.stopped = 0;
        
        timer.div       = 0xABCC;
        timer.enabled   = 0;
        timer.frequency = 0;

        gpu.scanline    = 153;
        gpu.cycles      = 436;
        gpu.mode        = VBLANK;
        gpu.coincidence = 1;
    }

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
    printf("DIV: %04X\n", timer.div);
    printf("LY: %02X\n", ReadIO(0x44));
    printf("SCN: %02X\n", gpu.scanline);
    printf("CYCLES: %03i\n", gpu.cycles);
    printf("MODE: %02X\n", gpu.mode);
    printf("TIMA: %02X\n", *timer.tima);
    printf("TMA: %02X\n", *timer.tma);
    printf("TAC: %02X\n", ReadByte(0xFF07));
    printf("ROM: %02X\n\n", mbc.romBank);
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

    #ifdef DEBUG
    if (registers.pc == 0x100) {
        PrintRegisters();
        getchar();
    }
 
    #endif

    if (cpu.bugged) {
        opcode = ReadByte(registers.pc);
        cpu.bugged = 0;
    } else {
        opcode = ReadByte(registers.pc);
        registers.pc += 1;
    }

    t_cycles += ExecuteInstruction(opcode);

}