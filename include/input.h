#pragma once

#include "SDL2/SDL.h"
#include "memory.h"
#include "gpu.h"

struct {
    uint8_t A      : 1;
    uint8_t B      : 1;
    uint8_t Select : 1;
    uint8_t Start  : 1;

    uint8_t Up     : 1;
    uint8_t Down   : 1;
    uint8_t Left   : 1;
    uint8_t Right  : 1;
} keys;

void HandleInput(SDL_Event *event);