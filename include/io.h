#pragma once

#include <stdint.h>
#include "memory.h"

enum IORegisters {
    IO_P1 = 0x00,
    IO_SB = 0x01,
    IO_SC = 0x02,

    // Timer
    IO_DIV_LSB = 0x03,
    IO_DIV_MSB = 0x04,
    IO_TIMA = 0x05,
    IO_TMA = 0x06,
    IO_TAC = 0x07,

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