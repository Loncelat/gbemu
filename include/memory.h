#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL2/SDL.h"
#include "mbc.h"
#include "registers.h"
#include "timer.h"
#include "input.h"
#include "gpu.h"
#include "io.h"

extern uint8_t *rom; // 0000-7FFF ROM | cartridge data
extern uint8_t vram[0x2000]; // 8000-9FFF Video RAM
extern uint8_t *sram; // A000-BFFF External RAM
extern uint8_t wram[0x2000]; // C000-DFFF (Working RAM)
extern uint8_t oam[0x00A0]; // FE00-FE9F Sprite information
extern uint8_t io[0x0080];   // FF00-FF7F MMIO
extern uint8_t hram[0x0080]; // FF80-FFFF Zero-page RAM | high RAM

void PrintRom(void);
void dma(uint16_t origin);

uint8_t ReadByte(uint16_t address);
uint16_t ReadShort(uint16_t address);
uint16_t ReadStack(void);

void WriteByte(uint16_t address, uint8_t data);
void WriteShort(uint16_t address, uint16_t data);
void WriteStack(uint16_t data);