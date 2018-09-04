#include "memory.h"

/*
    TODO: fatsoenlijkere werking van io.
    TODO: fatsoenlijke werking OAM DMA.
*/

uint8_t *rom;        // 0000-7FFF ROM | cartridge data
uint8_t vram[0x2000]; // 8000-9FFF Video RAM
uint8_t *sram;        // A000-BFFF External RAM (on cartridge)
uint8_t wram[0x2000]; // C000-DFFF (Working RAM) ( E000-FDFF (Shadow Working RAM) )
uint8_t oam[0x00A0];  // FE00-FE9F Sprite information
uint8_t io[0x0080];   // FF00-FF7F MMIO
uint8_t hram[0x0080]; // FF80-FFFF Zero-page RAM | high RAM

// DMA of zo.
void dma(uint16_t origin) {

    if (origin >= 0xE000) {
        origin -= 0x2000;
    }

    for (uint16_t i = 0; i < 160; ++i) {
        oam[i] = ReadByte(origin + i);
        //gpuCycle(4);
    }
}

uint8_t ReadByte(uint16_t address) {

    switch (address >> 12) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            return ReadRom(address);
        
        case 0x8:
        case 0x9:
            if (gpu.mode != DATA_TO_LCD) {
                return vram[address - 0x8000];
            }

        case 0xA:
        case 0xB:
            return ReadRam(address - 0xA000);

        case 0xC:
        case 0xD:
            return wram[address - 0xC000];
        
        default:
            if (address <= 0xFDFF) {
                return wram[address - 0xE000]; // Leest uit wram.
            }
            else if (address <= 0xFE9F) {
                if (gpu.mode == HBLANK || gpu.mode == VBLANK) {
                    return oam[address - 0xFE00];
                }
            }
            else if (address <= 0xFEFF) {
                return 0xFF;
            }
            else if (address <= 0xFF7F) {
                return ReadIO(address);
            }
            else {
                return hram[address - 0xFF80];
            }
    }

    return 0xFF;
}

uint16_t ReadShort(uint16_t address) {
	return ReadByte(address) | ReadByte(address + 1) << 8;
}

uint16_t ReadStack(void) {
    uint16_t result = ReadShort(registers.sp);
    registers.sp += 2;
    return result;
}

void WriteByte(uint16_t address, uint8_t data) {

    if (address <= 0x7FFF) {
        WriteRom(address, data);
    }    
    else if (address >= 0x8000 && address <= 0x9FFF) {
        if (gpu.mode != DATA_TO_LCD || !LCD_ENABLED) {
            vram[address - 0x8000] = data;
        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        WriteRam(address - 0xA000, data);
    }
    else if (address >= 0xC000 && address <= 0xDFFF) {
        wram[address - 0xC000] = data;
    }
    else if (address >= 0xE000 && address <= 0xFDFF) {
        wram[address - 0xE000] = data; // Hoera voor echo RAM
    }
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        if (gpu.mode == HBLANK || gpu.mode == VBLANK || !LCD_ENABLED) {
            oam[address - 0xFE00] = data;
        }
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) {
        WriteIO(address, data);
    }
    else if (address >= 0xFF80 && address <= 0xFFFF) {
        hram[address - 0xFF80] = data;
    }
}

void WriteShort(uint16_t address, uint16_t data) {
	WriteByte(address, (uint8_t) (data & 0xFF));
	WriteByte(address + 1, (uint8_t) (data >> 8));
}

void WriteStack(uint16_t data) {
    registers.sp -= 2;
    WriteShort(registers.sp, data);
}