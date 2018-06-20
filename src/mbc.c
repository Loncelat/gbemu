#include "mbc.h"

struct mbc mbc = { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 };


uint8_t ReadRom(uint16_t address) {

    uint32_t realDest;

    realDest = (uint32_t) (mbc.romBank * 0x4000) + (address & 0x3FFF);
    
    // if (mbc.mode) {
    //     realDest = (uint32_t) ((mbc.romBank & 0x1F) * 0x4000) + (address & 0x3FFF);
    // } else {
    // }

    return rom[realDest];

}

void WriteRom(uint16_t address, uint8_t data) {

    if (address <= 0x1FFF) {
        mbc.RAM_EN = (data & 0x0F) == 0x0A;
        return;
    }

    switch (mbc.type) {
        case ROM:
            break;

        case MBC1:
            MBC1_ROM_WRITE(address, data);
            break;

        case MBC3:
            MBC3_ROM_WRITE(address, data);
            break;

        default:
            break;
    }
    return;
}

uint8_t ReadRam(uint16_t address) {

    if (!mbc.RAM_EN || mbc.ramsize == 0) {
        return 0xFF;
    }

    switch (mbc.type) {
        case MBC1:
            return MBC1_RAM_READ(address);

        case MBC3:
            return MBC3_RAM_READ(address);

        default:
            return 0xFF;
    }
}

void WriteRam(uint16_t address, uint8_t data) {

    if (!mbc.RAM_EN || mbc.ramsize == 0) {
        return;
    }

    switch (mbc.type) {
        case MBC1:
            MBC1_RAM_WRITE(address, data);

        case MBC3:
            MBC3_RAM_WRITE(address, data);

        default:
            return;
    }
}