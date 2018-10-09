#pragma once

#include <stdio.h>
#include <stdint.h>
#include "registers.h"
#include "memory.h"
#include "cpu.h"
#include "gpu.h"

#define INTERRUPT_FLAGS io[IO_IF]
#define INTERRUPT_ENABLE hram[0x7F]

#define REQ_INTERRUPT(interrupt) do { \
    INTERRUPT_FLAGS = INTERRUPT_FLAGS | (interrupt); \
} while (0)

#define NO_INTR_DISPATCH (0)
#define INTR_DISPATCH (1)

#define JOYPAD_INTERRUPT (1 << 4)
#define SERIAL_INTERRUPT (1 << 3)
#define TIMER_INTERRUPT (1 << 2)
#define STAT_INTERRUPT (1 << 1)
#define VBL_INTERRUPT (1 << 0)

extern uint8_t IME;

uint8_t CheckAndHandleInterrupts(void);