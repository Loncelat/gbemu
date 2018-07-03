#pragma once

#include <stdio.h>
#include "SDL2/SDL.h"

#include "rom.h"
#include "memory.h"
#include "cpu.h"
#include "gpu.h"
#include "apu.h"
#include "input.h"
#include "timer.h"

#define EMU_NAME "gbemu"

uint8_t ShutdownRequested;