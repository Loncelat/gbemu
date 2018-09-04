#pragma once

#include <stdint.h>
#include "memory.h"

uint8_t ReadIO(uint16_t address);
void WriteIO(uint16_t address, uint8_t data);
void ResetIO(void);