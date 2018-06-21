#include "mbc3.h"

//uint8_t RTC_Register;

void MBC3_ROM_WRITE(uint16_t address, uint8_t data) {
    if (address <= 0x3FFF) {
        mbc.romBank = data & 0x7F;
        if (mbc.romBank == 0x00) {
            mbc.romBank = 0x01;
        }
        mbc.romBank &= (mbc.rombanks - 1);
    }
    else if (address <= 0x5FFF) {

        if (data <= 0x03) {
            if (mbc.rambanks > 0) {
                mbc.ramBank = data & (mbc.rambanks - 1);
            }
            mbc.mode = MBC3_RAM_MODE;
        } 
        else if (data >= 0x8 && data <= 0xC) {
            //RTC_Register = data;
            mbc.mode = MBC3_RTC_MODE;
        }
    }
}

uint8_t MBC3_RAM_READ(uint16_t address) {
    if (mbc.mode == MBC3_RAM_MODE && mbc.rambanks > 0) {
        return sram[((mbc.ramBank & (mbc.rambanks - 1)) * 0x2000) + (address & 0x1FFF)];
    } else if (mbc.mode == MBC3_RTC_MODE) {
        // TODO: RTC support.
        return 0xFF;
    }
    return 0xFF;
}
void MBC3_RAM_WRITE(uint16_t address, uint8_t data) {
    if (mbc.mode == MBC3_RAM_MODE && mbc.rambanks > 0) {
        sram[((mbc.ramBank & (mbc.rambanks - 1)) * 0x2000) + (address & 0x1FFF)] = data;
    } else if (mbc.mode == MBC3_RTC_MODE) {
        // TODO: RTC support.
        return;
    }
}