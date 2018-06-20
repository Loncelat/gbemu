#include "mbc3.h"

void MBC3_ROM_WRITE(uint16_t address, uint8_t data) {
    if (address <= 0x3FFF) {
        mbc.romBank = data & 0x7F;
        if (mbc.romBank == 0x00) {
            mbc.romBank = 0x01;
        }
        mbc.romBank &= mbc.rombanks - 1;
        return;
    }

    if (address <= 0x5FFF) {
        if (data <= 0x03) {
            if (mbc.rambanks > 0) {
                mbc.ramBank = data & (mbc.rambanks - 1);
            }
        } else {
            return;
        }
    }
}

uint8_t MBC3_RAM_READ(uint16_t address) {
    return 0xFF;
}
void MBC3_RAM_WRITE(uint16_t address, uint8_t data) {
    
}