#include "io.h"

uint8_t ReadIO(uint16_t address) {
    switch(address - 0xFF00) {
        case IO_P1:;
            uint8_t JoyPad_Register = io[0x00];
            if (JoyPad_Register & (1 << 4)) { // RLUD
                JoyPad_Register &= (keys.Start << 3) | (keys.Select << 2) | (keys.B << 1) | (keys.A << 0);
            }
            if (JoyPad_Register & (1 << 5)) {
                JoyPad_Register &= (keys.Down << 3) | (keys.Up << 2) | (keys.Left << 1) | (keys.Right << 0);
            }
            return JoyPad_Register | 0xC0;
            
        case IO_DIV_LSB: return timer.div & 0x00FF;
        case IO_DIV_MSB: return (timer.div & 0xFF00) >> 8;
        case IO_TAC: return 0xF8 | (timer.enabled << 2) | timer.frequency;
        case IO_STAT: return ((*gpu.stat | 0x80) & 0xF8) | (gpu.ly_equals_lyc << 2) | (gpu.mode);
        default: return io[address - 0xFF00];
    }
}

void WriteIO(uint16_t address, uint8_t data) {
    switch(address - 0xFF00) {
        case IO_P1: // JOYPAD
            io[0x00] = (data & (3 << 4)) | (io[0x00] & ~(3 << 4));
            break;
        case IO_DIV_LSB:
        case IO_DIV_MSB: timer.div = 0x0000; *timer.tima = 0x00; break;
        case IO_TAC: WriteTimerControl(data); io[0x7] = 0xF8 | (data & 0x3); break;
        case 0x0F: io[0x0F] = data | 0xE0; break;

        // Bit 7 altijd 1. Bits 0-2 behouden.
        case IO_STAT: *gpu.stat = ((data | 0x80) & 0xF8); break;
        case IO_LY: break; // LY is read-only.

        case IO_LYC: 
            *gpu.lyc = data; 
            Compare_LY_LYC();
            break;

        case IO_DMA: 
            *gpu.dma = data;
            dma(data << 8);
            break;

        case IO_BGP: 
            *gpu.bgp = data;
            gpu.bgPalette[0] = (data & 0x03) >> 0;
            gpu.bgPalette[1] = (data & 0x0C) >> 2;
            gpu.bgPalette[2] = (data & 0x30) >> 4;
            gpu.bgPalette[3] = (data & 0xC0) >> 6;
            break;

        case IO_OBP0: 
            *gpu.obp0 = data | 0x03;
            gpu.obpPalette[0][0] = (data & 0x0C) >> 2;
            gpu.obpPalette[0][1] = (data & 0x30) >> 4;
            gpu.obpPalette[0][2] = (data & 0xC0) >> 6;
            break;
        
        case IO_OBP1: 
            *gpu.obp1 = data | 0x03;
            gpu.obpPalette[1][0] = (data & 0x0C) >> 2;
            gpu.obpPalette[1][1] = (data & 0x30) >> 4;
            gpu.obpPalette[1][2] = (data & 0xC0) >> 6;
            break;

        default: io[address - 0xFF00] = data;
    }
}

void ResetIO(void) {
    return;
}