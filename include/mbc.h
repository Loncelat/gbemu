#pragma once

#include <stdio.h>
#include <stdint.h>
#include "memory.h"

#include "mbc1.h"
#include "mbc2.h"
#include "mbc3.h"
#include "mbc5.h"

extern struct mbc {
    uint8_t RAM_EN : 1;

    uint8_t ramBank : 5;

    uint8_t mode : 1;
    uint8_t battery : 1;
    uint8_t timer : 1;

    uint8_t bootromEnabled : 1;
    uint8_t useBootRom : 1;

    uint16_t romBank;

    uint8_t type;

    uint16_t rombanks;
    uint8_t rambanks;
    uint32_t ramsize;

    FILE *save;
} mbc;

enum {
    ROM =  0,
    MBC1 = 1,
    MBC2 = 2,
    MBC3 = 3,
    MBC5 = 5,
    HUC1 = 6,
};

uint8_t ReadRom(uint16_t address);
void WriteRom(uint16_t address, uint8_t data);

uint8_t ReadRam(uint16_t address);
void WriteRam(uint16_t address, uint8_t data);