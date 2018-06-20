#pragma once

#include <stdio.h>
#include <stdint.h>
#include "registers.h"
#include "memory.h"
#include "cpu.h"
#include "gpu.h"

#define INTERRUPT_FLAGS ReadByte(0xFF0F)
#define INTERRUPT_ENABLE ReadByte(0xFFFF)

#define REQ_INTERRUPT(interrupt) do { \
    WriteByte(IF_LOCATION,  INTERRUPT_FLAGS | (interrupt)); \
} while (0)

#define IF_LOCATION 0xFF0F
#define IE_LOCATION 0xFFFF

#define JOYPAD_INTERRUPT (1 << 4)
#define SERIAL_INTERRUPT (1 << 3)
#define TIMER_INTERRUPT (1 << 2)
#define STAT_INTERRUPT (1 << 1)
#define VBL_INTERRUPT (1 << 0)

extern uint8_t IME;

uint8_t CheckAndHandleInterrupts(void);