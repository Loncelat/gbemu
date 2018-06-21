#include "mbc1.h"

uint8_t MBC1_ROM_READ(uint16_t address) {

    uint32_t real_address;

    if (address <= 0x3FFF) {

        if (mbc.mode == MBC1_ROM_MODE) {
            real_address = address;
        } else {
            real_address = (((mbc.ramBank << 5) & (mbc.rombanks - 1)) * 0x4000) + address;
        }

    } else {

        // 8 Mbit & 16 Mbit.
        if (mbc.rombanks >= 64) {
            real_address = (((mbc.romBank & 0x1F) | (mbc.ramBank << 5)) & (mbc.rombanks - 1)) * 0x4000 + (address & 0x3FFF);
        } else {
            real_address = (mbc.romBank * 0x4000) + (address & 0x3FFF);
        }
    }

    return rom[real_address];
}

void MBC1_ROM_WRITE(uint16_t address, uint8_t data) {
    
    if (address <= 0x3FFF) {

        uint8_t bank = (data & 0x1F);
        if (!bank) {
            bank += 1;
        }
        mbc.romBank = (mbc.romBank & 0x60) | bank;
        mbc.romBank &= (mbc.rombanks - 0x1);
    }
    else if (address <= 0x5FFF) {
        mbc.ramBank = data & 0x3;
    }
    else {
        mbc.mode = data & 0x1;
    }
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