#pragma once

#include <stdint.h>
#include "memory.h"

enum IORegisters {
    IO_P1 = 0x00,
    IO_SB = 0x01,
    IO_SC = 0x02,

    // Timer
    IO_DIV = 0x04,
    IO_TIMA = 0x05,
    IO_TMA = 0x06,
    IO_TAC = 0x07,

    IO_IF = 0x0F,

    // Audio
    IO_NR10 = 0x10,
    IO_NR11 = 0x11,
    IO_NR12 = 0x12,
    IO_NR13 = 0x13,
    IO_NR14 = 0x14,
    IO_NR21 = 0x16,
    IO_NR22 = 0x17,
    IO_NR23 = 0x18,
    IO_NR24 = 0x19,
    IO_NR30 = 0x1A,
    IO_NR31 = 0x1B,
    IO_NR32 = 0x1C,
    IO_NR33 = 0x1D,
    IO_NR34 = 0x1E,
    IO_NR41 = 0x20,
    IO_NR42 = 0x21,
    IO_NR43 = 0x22,
    IO_NR44 = 0x23,
    IO_NR50 = 0x24,
    IO_NR51 = 0x25,
    IO_NR52 = 0x26,

    // Video
    IO_LCDC = 0x40,
    IO_STAT = 0x41,
    IO_SCY = 0x42,
    IO_SCX = 0x43,
    IO_LY = 0x44,
    IO_LYC = 0x45,
    IO_DMA = 0x46,
    IO_BGP = 0x47,
    IO_OBP0 = 0x48,
    IO_OBP1 = 0x49,
    IO_WY = 0x4A,
    IO_WX = 0x4B,
};

uint8_t ReadIO(uint16_t address);
void WriteIO(uint16_t address, uint8_t data);
void ResetIO(void);