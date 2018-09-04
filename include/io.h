#pragma once

#include <stdint.h>
#include "memory.h"

#define IO_P1 0x00
#define IO_SB 0x01
#define IO_SC 0x02
#define IO_DIV_LSB 0x03
#define IO_DIV_MSB 0x04
#define IO_TAC 0x07
#define IO_STAT 0x41
#define IO_LY 0x44
#define IO_LYC 0x45
#define IO_DMA 0x46
#define IO_BGP 0x47
#define IO_OBP0 0x48
#define IO_OBP1 0x49

uint8_t ReadIO(uint16_t address);
void WriteIO(uint16_t address, uint8_t data);
void ResetIO(void);