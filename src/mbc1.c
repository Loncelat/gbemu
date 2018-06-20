#include "mbc1.h"

void MBC1_ROM_WRITE(uint16_t address, uint8_t data) {
    
    // 0x2000 - 0x3FFF
    if (address <= 0x3FFF) {

        uint8_t bank = (data & 0x1F);
        if (!bank) {
            bank += 1;
        }
        mbc.romBank = (mbc.romBank & 0x60) | bank;
        mbc.romBank &= (mbc.rombanks - 0x1);
        return;
    }
        
    // 0x4000 - 0x5FFF
    if (address <= 0x5FFF) {
        mbc.ramBank = data & 0x3;
        return;
    }

    uint8_t newMode = (data & 0x1);

    // Controleer of de nieuwe mode anders is.
    if (mbc.mode ^ newMode) {

        if (newMode == MBC1_ROM_MODE) {
            mbc.romBank = (mbc.romBank & 0x1F) | (mbc.ramBank << 5);
        } else {
            mbc.romBank = (mbc.romBank & 0x1F);
        }
        mbc.romBank &= (mbc.rombanks - 0x1);
    }

    mbc.mode = newMode;
}

uint8_t MBC1_RAM_READ(uint16_t address) {
    if (address > mbc.ramsize) {
        return 0xFF;
    }

    if (mbc.mode && mbc.ramsize >= 0x2000) {
        address = ( (mbc.ramBank & (mbc.rambanks - 1)) * 0x2000) + (address & 0x1FFF);
    }

    return sram[address];
}

void MBC1_RAM_WRITE(uint16_t address, uint8_t data) {
    if (address > mbc.ramsize) {
        return;
    }
    
    if (mbc.mode && mbc.ramsize >= 0x2000) {
        address = ((mbc.ramBank & (mbc.rambanks - 1)) * 0x2000) + (address & 0x1FFF);
    }

    sram[address] = data;
}