#pragma once

#include <stdint.h>
#include "memory.h"
#include "interrupts.h"

extern struct timer {
    // Divider Register.
    uint16_t div;

    // Timer Counter.
    uint8_t * const tima;

    // Timer Modulo Register.
    uint8_t * const tma;

    uint8_t enabled;
    uint8_t frequency;
} timer;

void WriteTimerControl(uint8_t data);
void UpdateTimer(uint8_t cycles);