#pragma once

#include <stdint.h>
#include "memory.h"
#include "interrupts.h"
#include "io.h"

extern struct timer {
    uint8_t * const tima;
    uint8_t * const tma;
    
    uint16_t div;
    uint16_t cycles;

    uint8_t enabled;
    uint8_t frequency;
} timer;

void WriteTimerControl(uint8_t data);
void UpdateTimer(uint8_t cycles);