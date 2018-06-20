#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "memory.h"
#include "registers.h"
#include "interrupts.h"
#include "timer.h"
#include "input.h"
#include "instructions.h"

#define NOT_HALTED 0
#define HALTED 1

struct cpu {
    uint8_t halted : 1;
    uint8_t bugged : 1;
    uint8_t stopped : 1;
} extern cpu;

extern uint8_t enableIMENextClock, t_cycles;

void CPUCycle(void);
void ResetCPU(void);
void PrintRegisters(void);