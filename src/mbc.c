#include "mbc.h"

struct mbc mbc = { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 };


uint8_t ReadRom(uint16_t address) {
    switch(mbc.type) {
        case MBC1:
            return MBC1_ROM_READ(address);
            
        default:
            if (address <= 0x3FFF) {
                return rom[address];
            } else {
                return rom[(mbc.romBank * 0x4000) + (address & 0x3FFF)]; 
            }
    }
}

void WriteRom(uint16_t address, uint8_t data) {

    if (address <= 0x1FFF) {

        if (mbc.type == MBC2 && !(address & 0x0100)) {
            // Bit 9 must be 0 to toggle RAM_EN.
            mbc.RAM_EN = !mbc.RAM_EN;
        } else {
            mbc.RAM_EN = (data & 0x0F) == 0x0A;
        }
        return;
    }

    switch (mbc.type) {
        case ROM:
            break;

        case MBC1:
            MBC1_ROM_WRITE(address, data);
            break;

        case MBC2:
            MBC2_ROM_WRITE(address, data);
            break;

        case MBC3:
            MBC3_ROM_WRITE(address, data);
            break;

        case MBC5:
            MBC5_ROM_WRITE(address, data);
            break;

        default:
            break;
    }
    return;
}

uint8_t ReadRam(uint16_t address) {

    // TODO: Account for no ram + timer.
    if (!mbc.RAM_EN || mbc.ramsize == 0) {
        return 0xFF;
    }

    switch (mbc.type) {
        case MBC1:
            return MBC1_RAM_READ(address);

        case MBC2:
            return MBC2_RAM_READ(address);

        case MBC3:
            return MBC3_RAM_READ(address);

        case MBC5:
            return MBC5_RAM_READ(address);

        default:
            return 0xFF;
    }
}

void WriteRam(uint16_t address, uint8_t data) {

    // TODO: Account for no ram + timer.
    if (!mbc.RAM_EN || mbc.ramsize == 0) {
        return;
    }

    switch (mbc.type) {
        case MBC1:
            MBC1_RAM_WRITE(address, data);
            break;

        case MBC2:
            MBC2_RAM_WRITE(address, data);
            break;

        case MBC3:
            MBC3_RAM_WRITE(address, data);
            break;

        case MBC5:
            MBC5_RAM_WRITE(address, data);
            break;

        default:
            return;
    }
}