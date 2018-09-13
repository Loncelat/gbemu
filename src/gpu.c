#include "gpu.h"

void RenderTiles(void);
void RenderSprites(void);
void PushScanLine(void);
void DrawPixelBuffer(void);

static inline void UpdategpuMode(void);

SDL_Window *window = NULL;
SDL_Renderer *LCDRenderer = NULL;
SDL_Texture *LCD = NULL;

uint8_t Window_Scale = LCD_DEFAULT_SIZE;

SDL_Event event;

uint64_t vsyncStartTime;
uint8_t waitForVsync = 1;
uint8_t skipNextFrame = 0;

// LCD van 160px bij 144px.
Colour_t pixelBuffer[144][160];
uint8_t scanLineRow[160];

// Grijstinten-palette.
const Colour_t palette[4] = {
    {232, 232, 232},
    {160, 160, 160},
    {88,  88,  88},
    {16,  16,  16},
};

struct gpu gpu = { 
    &io[IO_LCDC], &io[IO_STAT], &io[IO_SCY], &io[IO_SCX],
    &io[IO_LYC], &io[IO_DMA], &io[IO_BGP],
    &io[IO_OBP0], &io[IO_OBP1], &io[IO_WY], &io[IO_WX], 
    0, 0, 0, 0, 
    {0, 0, 0, 0}, { {3, 3, 3}, {3, 3, 3} }
};

int SetupVideo(void) {

    window = SDL_CreateWindow(
        "gbemu",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        LCD_WIDTH  * LCD_DEFAULT_SIZE,
        LCD_HEIGHT * LCD_DEFAULT_SIZE,
        0);

    if (window == NULL) {
        printf("Venster ging kaduuk: %s\n", SDL_GetError());
        return 1;
    }

    LCDRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (LCDRenderer == NULL) {
        printf("Renderer ging kaduuk: %s\n", SDL_GetError());
        return 1;
    }

    LCD = SDL_CreateTexture(
        LCDRenderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        LCD_WIDTH,
        LCD_HEIGHT);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRenderDrawColor(LCDRenderer, 232, 232, 232, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(LCDRenderer);
    SDL_RenderPresent(LCDRenderer);

    return 0;
}

void ResizeWindow(int8_t factor) {
    if (factor == -1 && Window_Scale > 1) {
        Window_Scale -= 1;
        SDL_SetWindowSize(window, LCD_WIDTH * Window_Scale, LCD_HEIGHT * Window_Scale);
    } 
    else if (factor == 1 && Window_Scale <= LCD_MAX_SIZE) {
        Window_Scale += 1;
        SDL_SetWindowSize(window, LCD_WIDTH * Window_Scale, LCD_HEIGHT * Window_Scale);
    }
}

void gpuCycle(uint8_t cycles) {
    
	UpdategpuMode();

	if (!LCD_ENABLED) {
        return;
    }

	gpu.cycles += cycles;

	if (gpu.cycles >= 456)
	{

        if (gpu.scanline <= 143 && !skipNextFrame) {
            PushScanLine();
        }

        if (gpu.scanline == 143) {
            DrawPixelBuffer();
            REQ_INTERRUPT(VBL_INTERRUPT);
        } 
        else if (gpu.scanline == 153) {
            gpu.scanline = 0xFF;
        }

		gpu.scanline += 1;

        UpdategpuMode();
        Compare_LY_LYC();

		gpu.cycles -= 456;
	}
}

static inline void UpdategpuMode(void) {

    if (!LCD_ENABLED) {
        gpu.mode = HBLANK;
        gpu.ly_equals_lyc = 0;
        return;
    }

    uint8_t requestInterrupt = 0;
    uint8_t previousMode = gpu.mode;

    if (gpu.scanline >= 144) {
        gpu.mode = VBLANK;
        requestInterrupt = *gpu.stat & (1 << 4);
    } else {
        
        if (gpu.cycles < 80) {
            gpu.mode = SEARCH_OAM_RAM;
            requestInterrupt = *gpu.stat & (1 << 5);
        }
        else if (gpu.cycles < 252) {
            gpu.mode = DATA_TO_LCD;
        }
        else {
            gpu.mode = HBLANK;
            requestInterrupt = *gpu.stat & (1 << 3);
        }

    }

    if (requestInterrupt && gpu.mode != previousMode) {
        REQ_INTERRUPT(STAT_INTERRUPT);
    }

}

void Compare_LY_LYC(void) {

    if (gpu.scanline == 153 && gpu.cycles >= 4) {
        gpu.ly_equals_lyc = (*gpu.lyc == 0);
    } else {
        gpu.ly_equals_lyc = (gpu.scanline == *gpu.lyc);
    }

    if (gpu.ly_equals_lyc && COINCIDENCE_IRQ) {
        REQ_INTERRUPT(STAT_INTERRUPT);
    }

}

void PushScanLine(void) {

    RenderTiles();

    if (SPRITES_ENABLED) {
        RenderSprites();
    }
}

void RenderTiles(void) {
    uint16_t TileMapOffset;
    uint8_t usingWindow = 0;
    uint8_t signedAddressing = 0;

    if (WINDOW_ENABLED && *gpu.wy <= gpu.scanline) {
        usingWindow = 1;
    }

    if ( !(TILE_ADDR_MODE) ) {
        signedAddressing = 1;
    }

    if (usingWindow) {
        TileMapOffset = WD_TILEMAP ? 0x1C00 : 0x1800;
    } else {
        TileMapOffset = BG_TILEMAP ? 0x1C00 : 0x1800;
    }

    uint8_t y;

    if (usingWindow) {
        y = gpu.scanline - *gpu.wy;
    } else {
        y = gpu.scanline + *gpu.scy;
    }

    TileMapOffset += ((uint8_t) (y / 8)) * 32;

    for (uint8_t i = 0; i < 160;) {

        uint8_t xPos = i + *gpu.scx;

        // Possible underflow when *gpu.wx < 7.
        if (usingWindow && i >= (*gpu.wx - 7)) {
            xPos = i - (*gpu.wx - 7);
        }

        uint16_t tileCol = xPos / 8;
        uint16_t tileNum = vram[TileMapOffset + tileCol];

        if (signedAddressing && tileNum < 128) { 
            tileNum += 256;
        }

        uint8_t tileRow = (y % 8) * 2;
        uint8_t data1 = vram[tileNum * 16 + tileRow];
        uint8_t data2 = vram[tileNum * 16 + tileRow + 1];

        uint8_t mask, colourID;
        uint8_t tilePos = xPos % 8;

        // Use the same data to draw the rest of the tile's row.
        do {
            mask = 1 << (7 - tilePos);

            colourID = (((data2 & mask) >> (7 - tilePos)) << 1) | 
                        ((data1 & mask) >> (7 - tilePos));

            scanLineRow[i] = colourID;
            pixelBuffer[gpu.scanline][i] = palette[gpu.bgPalette[colourID]];
            ++i;
            ++tilePos;

            if (i >= 160) {
                return;
            }

        } while (tilePos < 8);
    }
}

void RenderSprites(void) {
    uint8_t use8x16 = 0;

    uint8_t verticalsize = 8;

    if (SPRITE_SIZE) {
        verticalsize = 16;
        use8x16 = 1;
    }

    for (int16_t i = 156; i >=0 ; i -= 4) {
        Sprite_t sprite = { oam[i], oam[i + 1], oam[i + 2], oam[i + 3] };

        int16_t sx = sprite.x - 8;
        int16_t sy = sprite.y - 16;
        uint8_t xFlip = SPRITE_XFLIP;
        uint8_t palNo = SPRITE_PALETTE >> 4;
        uint8_t priority = SPRITE_PRIORITY;

        /*
        When using 8x16 sprites, the least significant bit of
        the sprite's tile number is ignored.
        */
        if (use8x16) {
            sprite.tile &= 0xFE;
        }

        if (gpu.scanline < sy || gpu.scanline >= sy + verticalsize) {
            continue;
        }

        int16_t line = gpu.scanline - sy;

        if (SPRITE_YFLIP) {
            line = (verticalsize - 1) - line;
        }

        line *= 2;

        uint8_t data1 = vram[sprite.tile * 16 + line];
        uint8_t data2 = vram[sprite.tile * 16 + line + 1];

        for (uint8_t tilex = 0; tilex < 8; tilex++) {

            int16_t x = sx + tilex;

            if(x < 0 || x >= 160 || (priority && scanLineRow[x])) {
                continue;
            }
            
            uint8_t mask = 1 << (7 - tilex);

            if (xFlip) {
                mask = 1 << tilex;
            }

            uint8_t colourID = ((data1 & mask) ? 1 : 0) +
                               ((data2 & mask) ? 2 : 0);

            if (colourID) {
                pixelBuffer[gpu.scanline][x] = palette[
                    gpu.obpPalette[palNo][colourID - 1]
                ];
            }

        }

    }
}

void DrawPixelBuffer(void) {

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            ShutdownRequested = 1;
        }
        HandleInput(&event);
    }

    skipNextFrame = 0;

    if (waitForVsync) {

        if (VSYNC_SHOULD_WAIT) {
            VSYNC_WAIT;
        }

        vsyncStartTime = SDL_GetPerformanceCounter();
        SDL_UpdateTexture(LCD, NULL, pixelBuffer, LCD_WIDTH * sizeof(Colour_t));
        SDL_RenderCopy(LCDRenderer, LCD, NULL, NULL);
        SDL_RenderPresent(LCDRenderer);
        
    } else if (!VSYNC_SHOULD_WAIT) {

        vsyncStartTime = SDL_GetPerformanceCounter();
        SDL_UpdateTexture(LCD, NULL, pixelBuffer, LCD_WIDTH * sizeof(Colour_t));
        SDL_RenderCopy(LCDRenderer, LCD, NULL, NULL);
        SDL_RenderPresent(LCDRenderer);

    } else {
        skipNextFrame = 1;
    }
}