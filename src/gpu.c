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
uint64_t frequency;

#ifdef BENCH
uint32_t frames = 0;
uint8_t renderCount = 0;
char name[32];
#endif

Colour_t pixelBuffer[144][160];
uint8_t scanLineRow[160];

Colour_t paletteData[2][4] = {
    {
        {232, 232, 232},
        {160, 160, 160},
        {88,  88,  88},
        {16,  16,  16},
    },
    {
        {155, 188, 15},
        {139, 172, 15},
        {48, 98, 48},
        {15, 56, 15},
    },
};

Colour_t (*palette)[4] = &paletteData[0];
uint8_t currentPalette = 0;

struct gpu gpu = { 
    &io[IO_LCDC], &io[IO_STAT], &io[IO_SCY], &io[IO_SCX],
    &io[IO_LYC], &io[IO_DMA], &io[IO_BGP],
    &io[IO_OBP0], &io[IO_OBP1], &io[IO_WY], &io[IO_WX], 
    0, 0, 0, 0, 0, 0,
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

    // SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRenderDrawColor(LCDRenderer, 232, 232, 232, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(LCDRenderer);
    SDL_RenderPresent(LCDRenderer);

    frequency = SDL_GetPerformanceFrequency();

    return 0;
}

void TogglePalette(void) {
    currentPalette = (currentPalette + 1) % PALETTE_COUNT;
    palette = &paletteData[currentPalette];
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

	if (!LCD_ENABLED) {
        return;
    }

	gpu.cycles += cycles;

	if (gpu.cycles > 456)
	{

        if (gpu.scanline <= 143 && !gpu.skipNextFrame) {
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

        UpdateCoincidence();

		gpu.cycles -= 456;
	}

    UpdategpuMode();
}

static inline void UpdategpuMode(void) {

    if (gpu.scanline >= 144) {
        gpu.mode = VBLANK;
    } else {        
        if (gpu.cycles < 80) {
            gpu.mode = SEARCH_OAM_RAM;
        }
        else if (gpu.cycles < 252) {
            gpu.mode = DATA_TO_LCD;
        }
        else {
            gpu.mode = HBLANK;
        }
    }

    UpdateStatusIRQ();
}

uint8_t GetStatusIRQ(void) {
    return (HBLANK_IRQ && gpu.mode == HBLANK) 
        || (VBLANK_IRQ && gpu.mode == VBLANK)
        || (OAM_SEARCH_IRQ && gpu.mode == SEARCH_OAM_RAM)
        || (COINCIDENCE_IRQ && gpu.coincidence);
}

void UpdateStatusIRQ(void) {
    uint8_t irq = gpu.statusirq;
    gpu.statusirq = GetStatusIRQ();

    if (!irq && gpu.statusirq) {
        REQ_INTERRUPT(STAT_INTERRUPT);
    }
}

void UpdateCoincidence(void) {
    if (gpu.scanline == 153 && gpu.cycles >= 4) {
        gpu.coincidence = (*gpu.lyc == 0);
    } else {
        gpu.coincidence = (gpu.scanline == *gpu.lyc);
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

    TileMapOffset += (y / 8) * 32;

    for (uint8_t i = 0; i < 160;) {

        uint8_t xPos = i + *gpu.scx;

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
            pixelBuffer[gpu.scanline][i] = (*palette)[gpu.bgPalette[colourID]];
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

        if (gpu.scanline < sy || gpu.scanline >= sy + verticalsize) {
            continue;
        }

        /*
        When using 8x16 sprites, the least significant bit of
        the sprite's tile number is ignored.
        */
        if (use8x16) {
            sprite.tile &= 0xFE;
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
                pixelBuffer[gpu.scanline][x] = (*palette)[
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

    gpu.skipNextFrame = 0;
    
    #ifdef BENCH
    frames += 1;
    #endif

    if (VSYNC_SHOULD_WAIT) {

        // Pause if framerate is capped.
        if (waitForVsync) {
            VSYNC_WAIT;
        } else {
            gpu.skipNextFrame = 1;
            return;
        }
    }

    vsyncStartTime = SDL_GetPerformanceCounter();
    SDL_UpdateTexture(LCD, NULL, pixelBuffer, LCD_WIDTH * sizeof(Colour_t));
    SDL_RenderCopy(LCDRenderer, LCD, NULL, NULL);
    SDL_RenderPresent(LCDRenderer);

    #ifdef BENCH
    renderCount++;
    if (renderCount == 60) {
        snprintf(name, sizeof(name), "%d", frames);
        SDL_SetWindowTitle(window, name);
        frames = 0;
        renderCount = 0;
    }
    #endif
}