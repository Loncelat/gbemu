#pragma once

#include <stdint.h>
#include "mbc.h"

void MBC5_ROM_WRITE(uint16_t address, uint8_t data);

uint8_t MBC5_RAM_READ(uint16_t address);
void MBC5_RAM_WRITE(uint16_t address, uint8_t data);