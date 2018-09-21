#pragma once

#include <stdio.h>
#include <stdint.h>

#include "memory.h"
#include "mbc.h"

void MBC2_ROM_WRITE(uint16_t address, uint8_t data);

uint8_t MBC2_RAM_READ(uint16_t address);
void MBC2_RAM_WRITE(uint16_t address, uint8_t data);