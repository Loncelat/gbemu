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

static inline uint8_t ReadIO(uint16_t address);
static inline void WriteIO(uint16_t address, uint8_t data);

// DMA of zo.
void dma(uint16_t origin) {
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

inline uint8_t ReadIO(uint16_t address) {
    switch(address - 0xFF00) {
        case 0x00:;
            uint8_t JoyPad_Register = io[0x00];
            if (JoyPad_Register & (1 << 4)) { // RLUD
                JoyPad_Register &= 0xFF & ((keys.Start << 3) | (keys.Select << 2) | (keys.B << 1) | (keys.A << 0));
            }
            if (JoyPad_Register & (1 << 5)) {
                JoyPad_Register &= 0xFF & ((keys.Down << 3) | (keys.Up << 2) | (keys.Left << 1) | (keys.Right << 0));
            }
            return JoyPad_Register;
            
        case 0x03: return timer.div & 0x00FF;
        case 0x04: return (timer.div & 0xFF00) >> 8;
        case 0x07: return 0xF8 | (timer.enabled << 2) | timer.frequency;
        case 0x41: return ((*gpu.stat | 0x80) & 0xF8) | (gpu.ly_equals_lyc << 2) | (gpu.mode);
        default: return io[address - 0xFF00];
    }
}
inline void WriteIO(uint16_t address, uint8_t data) {
    switch(address - 0xFF00) {
        case 0x00: // JOYPAD
            io[0x00] = (data & (3 << 4)) | (io[0x00] & ~(3 << 4));
            break;
        case 0x03:
        case 0x04: timer.div = 0x0000; *timer.tima = 0x00; break;
        case 0x07: WriteTimerControl(data); io[0x7] = 0xF8 | (data & 0x3); break;
        case 0x0F: io[0x0F] = data | 0xE0; break;

        // Bit 7 altijd 1. Bits 0-2 behouden.
        case 0x41: *gpu.stat = ((data | 0x80) & 0xF8); break;
        case 0x44: break; // LY is read-only.

        case 0x45: 
            *gpu.lyc = data; 
            Compare_LY_LYC();
            break;

        case 0x46: 
            *gpu.dma = data;
            dma(data << 8);
            break;
        case 0x47: 
            *gpu.bgp = data;
            gpu.bgPalette[0] = (data & 0x03) >> 0;
            gpu.bgPalette[1] = (data & 0x0C) >> 2;
            gpu.bgPalette[2] = (data & 0x30) >> 4;
            gpu.bgPalette[3] = (data & 0xC0) >> 6;
            break;

        case 0x48: 
            *gpu.obp0 = data | 0x03;
            gpu.obpPalette[0][0] = (data & 0x0C) >> 2;
            gpu.obpPalette[0][1] = (data & 0x30) >> 4;
            gpu.obpPalette[0][2] = (data & 0xC0) >> 6;
            break;
        
        case 0x49: 
            *gpu.obp1 = data | 0x03;
            gpu.obpPalette[1][0] = (data & 0x0C) >> 2;
            gpu.obpPalette[1][1] = (data & 0x30) >> 4;
            gpu.obpPalette[1][2] = (data & 0xC0) >> 6;
            break;

        default: io[address - 0xFF00] = data;
    }
}