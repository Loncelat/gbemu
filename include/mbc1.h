#pragma once

#include <stdint.h>
#include "mbc.h"

#define MBC1_ROM_MODE (0)
#define MBC1_RAM_MODE (1)

uint8_t MBC1_ROM_READ(uint16_t address);
void MBC1_ROM_WRITE(uint16_t address, uint8_t data);

uint8_t MBC1_RAM_READ(uint16_t address);
void MBC1_RAM_WRITE(uint16_t address, uint8_t data);