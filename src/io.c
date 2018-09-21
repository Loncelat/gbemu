#include "io.h"

uint8_t ReadIO(uint16_t address) {
    switch(address) {
        case IO_P1:;
            uint8_t JoyPad_Register = io[IO_P1];
            if (JoyPad_Register & (1 << 4)) {
                JoyPad_Register &= (keys.Start << 3) | (keys.Select << 2) | (keys.B << 1) | (keys.A << 0) | 0xF0;
            }
            if (JoyPad_Register & (1 << 5)) {
                JoyPad_Register &= (keys.Down << 3) | (keys.Up << 2) | (keys.Left << 1) | (keys.Right << 0) | 0xF0;
            }
            return JoyPad_Register | 0xC0;
            
        case IO_DIV: return (timer.div & 0xFF00) >> 8;
        case IO_TAC: return 0xF8 | (timer.enabled << 2) | timer.frequency;
        case IO_STAT: return ((*gpu.stat | 0x80) & 0xF8) | (gpu.coincidence << 2) | (gpu.mode);
        case IO_LY:
            if (gpu.scanline == 153 && gpu.cycles >= 4) {
                return 0x00;
            } else {
                return gpu.scanline;
            }
        default: return io[address];
    }
}

void WriteIO(uint16_t address, uint8_t data) {
    switch(address) {
        case IO_P1: // JOYPAD
            io[IO_P1] = (data & (3 << 4)) | (io[IO_P1] & ~(3 << 4));
            break;
        case IO_DIV: timer.div = 0x0000; *timer.tima = 0x00; break;
        case IO_TAC: WriteTimerControl(data); io[IO_TAC] = 0xF8 | (data & 0x3); break;
        case IO_IF: io[IO_IF] = data | 0xE0; break;

        case IO_LCDC:
            if ( !(data & 0x80) && LCD_ENABLED) {
                gpu.mode     = HBLANK;
                gpu.cycles   = 0;
                gpu.scanline = 0;
            }
            else if ((data & 0x80) && !LCD_ENABLED) {

                // Next frame after turning on PPU isn't rendered.
                gpu.skipNextFrame = 1;
                UpdateCoincidence();

            }
            io[IO_LCDC] = data;
            break;

        case IO_STAT: *gpu.stat = ((data | 0x80) & 0xF8); break;
        case IO_LY: break;

        case IO_LYC: 
            *gpu.lyc = data; 
            
            // LY - LYC comparison doesn't happen when the PPU is disabled.
            if (LCD_ENABLED) {
                UpdateCoincidence();
            }
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
            *gpu.obp0 = data;
            gpu.obpPalette[0][0] = (data & 0x0C) >> 2;
            gpu.obpPalette[0][1] = (data & 0x30) >> 4;
            gpu.obpPalette[0][2] = (data & 0xC0) >> 6;
            break;
        
        case IO_OBP1: 
            *gpu.obp1 = data;
            gpu.obpPalette[1][0] = (data & 0x0C) >> 2;
            gpu.obpPalette[1][1] = (data & 0x30) >> 4;
            gpu.obpPalette[1][2] = (data & 0xC0) >> 6;
            break;

        default: io[address] = data;
    }
}

void ResetIO(void) {

    memset(io, 0xFF, sizeof(io));
    
    io[IO_P1] = 0xCF;
    
    io[IO_SB] = 0x00;
    io[IO_SC] = 0x7E;

    io[IO_TIMA] = 0x00;
    io[IO_TMA]  = 0x00;
    io[IO_IF]   = 0xE1;

    io[IO_NR10] = 0x80;
    io[IO_NR11] = 0xBF;
    io[IO_NR12] = 0xF3;
    io[IO_NR14] = 0xBF;
    io[IO_NR21] = 0x3F;
    io[IO_NR22] = 0x00;
    io[IO_NR24] = 0xBF;
    io[IO_NR30] = 0x7F;
    io[IO_NR32] = 0x9F;
    io[IO_NR34] = 0xBF;
    io[IO_NR41] = 0xFF;
    io[IO_NR42] = 0x00;
    io[IO_NR43] = 0x00;
    io[IO_NR44] = 0xBF;
    io[IO_NR50] = 0x77;
    io[IO_NR51] = 0xF3;
    io[IO_NR52] = 0xF1;

    io[IO_LCDC] = 0x91;
    io[IO_STAT] = 0x85;
    io[IO_LY]   = 0x99;
    io[IO_LYC]  = 0x00;
    io[IO_SCY]  = 0x00;
    io[IO_SCX]  = 0x00;
    io[IO_WY]   = 0x00;
    io[IO_WX]   = 0x00;
    WriteIO(IO_BGP, 0xFC);
    WriteIO(IO_OBP0, 0xFF);
    WriteIO(IO_OBP1, 0xFF);

}