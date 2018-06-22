#include "mbc5.h"

void MBC5_ROM_WRITE(uint16_t address, uint8_t data) {
    if (address <= 0x2FFF) {
        mbc.romBank = (mbc.romBank & 0x100) | data;
        mbc.romBank &= (mbc.rombanks - 1);
    } 
    else if (address <= 0x3FFF) {
        mbc.romBank = (mbc.romBank & 0xFF) | ((data & 0x1) << 8);
        mbc.romBank &= (mbc.rombanks - 1);
    }
    else if (address <= 0x5FFF) {
        mbc.ramBank = (data & 0xF);
        mbc.ramBank &= mbc.rambanks;
    }
}

uint8_t MBC5_RAM_READ(uint16_t address) {
    return sram[(mbc.ramBank * 0x2000) + (address & 0x1FFF)];
}
void MBC5_RAM_WRITE(uint16_t address, uint8_t data) {
    sram[(mbc.ramBank * 0x2000) + (address & 0x1FFF)] = data;
}