#include "gpu.h"

void RenderTiles(void);
void RenderSprites(void);
void UpdategpuMode(void);
void PushScanLine(void);
void DrawPixelBuffer(void);

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
const Colour_t paletteShades[4] = {
    {232, 232, 232},
    {160, 160, 160},
    {88,  88,  88},
    {16,  16,  16},
};

struct gpu gpu = { 
    &io[0x40], &io[0x41], &io[0x42], &io[0x43],
    &io[0x44], &io[0x45], &io[0x46], &io[0x47], 
    &io[0x48], &io[0x49], &io[0x4A], &io[0x4B], 
    0, 0, 0, 
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

	if (gpu.cycles > 456)
	{

        if (*gpu.ly <= 143 && !skipNextFrame) {
            PushScanLine();
        }

        if (*gpu.ly == 143) {
            DrawPixelBuffer();
            REQ_INTERRUPT(VBL_INTERRUPT);
        } 
        else if (*gpu.ly == 153) {
            *gpu.ly = 0xFF;
        }

		*gpu.ly += 1;

        UpdategpuMode();
        Compare_LY_LYC();

		gpu.cycles -= 456;
	}
}

void UpdategpuMode(void) {

    if (!LCD_ENABLED) {
        gpu.mode = HBLANK;
        gpu.ly_equals_lyc = 0;
        gpu.cycles = 0;
        *gpu.ly = 0x00;
        return;
    }

    uint8_t requestInterrupt = 0;
    uint8_t previousMode = gpu.mode;

    if (*gpu.ly >= 144) {
        gpu.mode = VBLANK;
        requestInterrupt = *gpu.stat & (1 << 4);
    } else {
        
        switch (gpu.cycles) {
            
            case 376 ... 456: 
                gpu.mode = SEARCH_OAM_RAM;
                requestInterrupt = *gpu.stat & (1 << 5);
                break;
            
            case 204 ... 375:
                gpu.mode = DATA_TO_LCD;
                break;
            
            default:
                gpu.mode = HBLANK;
                requestInterrupt = *gpu.stat & (1 << 3);
                break;

        }

    }

    if (requestInterrupt && gpu.mode != previousMode) {
        REQ_INTERRUPT(STAT_INTERRUPT);
    }

}

void Compare_LY_LYC(void) {

    gpu.ly_equals_lyc = (*gpu.ly == *gpu.lyc);

    if (gpu.ly_equals_lyc && COINCIDENCE_IRQ) {
        REQ_INTERRUPT(STAT_INTERRUPT);
    }

}

void PushScanLine(void) {

    memset(scanLineRow, 0, sizeof(scanLineRow));

    RenderTiles();

    if (SPRITES_ENABLED) {
        RenderSprites();
    }
}

void RenderTiles(void) {
    uint16_t TileMapOffset;
    uint8_t usingWindow = 0;
    uint8_t signedAddressing = 0;

    if (WINDOW_ENABLED && *gpu.wy <= *gpu.ly) {
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
        y = *gpu.ly - *gpu.wy;
    } else {
        y = *gpu.ly + *gpu.scy;
    }

    TileMapOffset += ((uint8_t) (y / 8)) * 32;

    for (uint8_t i = 0; i < 160;) {

        uint8_t xPos = i + *gpu.scx;

        if (usingWindow && i >= *gpu.wx - 7) {
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

        // Teken de rest van de tile ook.
        do {
            mask = 1 << (7 - tilePos);

            colourID = (((data2 & mask) >> (7 - tilePos)) << 1) | 
                        ((data1 & mask) >> (7 - tilePos));

            scanLineRow[i] = colourID;
            pixelBuffer[*gpu.ly][i] = paletteShades[gpu.bgPalette[colourID]];
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

        // Bit 0 wordt genegeerd bij 8x16.
        if (use8x16) {
            sprite.tile &= 0xFE;
        }

        if (*gpu.ly < sy || *gpu.ly >= sy + verticalsize) {
            continue;
        }

        int16_t line = *gpu.ly - sy;

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
                pixelBuffer[*gpu.ly][x] = paletteShades[
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