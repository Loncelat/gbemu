#pragma once

#include <stdint.h>
#include "mbc.h"

#define MBC3_RAM_MODE (0)
#define MBC3_RTC_MODE (1)

void MBC3_ROM_WRITE(uint16_t address, uint8_t data);

uint8_t MBC3_RAM_READ(uint16_t address);
void MBC3_RAM_WRITE(uint16_t address, uint8_t data);