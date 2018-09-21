#include "mbc2.h"

void MBC2_ROM_WRITE(uint16_t address, uint8_t data) {
    if (address <= 0x3FFF && (address & 0x0100)) {
        mbc.romBank = data & 0x0F;
    }
}

uint8_t MBC2_RAM_READ(uint16_t address) {
    if (address <= 0x1FF) {
        return sram[address];
    } else {
        return 0xFF;
    }
}

void MBC2_RAM_WRITE(uint16_t address, uint8_t data) {
    if (address <= 0x1FF) {
        sram[address] = data & 0x0F;
    }
}