#pragma once

#include <stdio.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "memory.h"
#include "main.h"

#define LCD_WIDTH 160
#define LCD_HEIGHT 144
#define LCD_DEFAULT_SIZE 4
#define LCD_MAX_SIZE 10
#define FRAMES_PER_SECOND 60
#define FRAME_FREQUENCY ((double) 1000 / FRAMES_PER_SECOND)

#define HBLANK         0x00
#define VBLANK         0x01
#define SEARCH_OAM_RAM 0x02
#define DATA_TO_LCD    0x03

#define VSYNC_SHOULD_WAIT ((SDL_GetPerformanceCounter() - vsyncStartTime) * 1000.0 / SDL_GetPerformanceFrequency() < FRAME_FREQUENCY)
#define VSYNC_WAIT (SDL_Delay((FRAME_FREQUENCY) - (SDL_GetPerformanceCounter() - vsyncStartTime) * 1000.0 / SDL_GetPerformanceFrequency()))

#define COINCIDENCE_IRQ (*gpu.stat & (1 << 6))
#define OAM_CHECK_IRQ   (*gpu.stat & (1 << 5))
#define VBLANK_IRQ      (*gpu.stat & (1 << 4))
#define HBLANK_IRQ      (*gpu.stat & (1 << 3))

#define LCD_ENABLED         (*gpu.control & (1 << 7))
#define WD_TILEMAP          (*gpu.control & (1 << 6))
#define TILE_ADDR_MODE      (*gpu.control & (1 << 4))
#define WINDOW_ENABLED      (*gpu.control & (1 << 5))
#define BG_TILEMAP          (*gpu.control & (1 << 3))
#define SPRITE_SIZE         (*gpu.control & (1 << 2))
#define SPRITES_ENABLED     (*gpu.control & (1 << 1))
#define BACKGROUND_ENABLED  (*gpu.control & (1 << 0))

#define SPRITE_PRIORITY (sprite.attributes & (1 << 7))
#define SPRITE_YFLIP    (sprite.attributes & (1 << 6))
#define SPRITE_XFLIP    (sprite.attributes & (1 << 5))
#define SPRITE_PALETTE  (sprite.attributes & (1 << 4))

extern SDL_Window *window;
extern SDL_Renderer *LCDRenderer;
extern SDL_Texture *LCD;

extern uint8_t Window_Scale;

extern uint64_t vsyncStartTime;
extern uint8_t waitForVsync;

typedef struct {
    uint8_t r, g, b;
} Colour_t;

struct gpu {
    uint8_t * const control;
    uint8_t * const stat;

    // Scroll X en Y
    uint8_t * const scy;
    uint8_t * const scx;
    
    // LCD current scanline. (144-153 = VBL)
    uint8_t * const ly;
    uint8_t * const lyc;

    uint8_t * const dma;

    // Background palette
    uint8_t * const bgp;
    uint8_t * const obp0;
    uint8_t * const obp1;

    // Window X en Y
    uint8_t * const wy;
    uint8_t * const wx;

    uint16_t cycles;

    // STAT deels ontbonden.
    uint8_t ly_equals_lyc : 1;
    uint8_t mode : 2;

    uint8_t bgPalette[4];
    uint8_t obpPalette[2][3];
} extern gpu;

typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attributes;
} Sprite_t;

int SetupVideo(void);
void ResizeWindow(int8_t factor);
void gpu_quit(void);

void gpuCycle(uint8_t cycles);
void Compare_LY_LYC(void);