#pragma once

#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "cb.h"
#include "memory.h"
#include "registers.h"
#include "interrupts.h"

extern const uint8_t instuctionCycles[256];
uint8_t ExecuteInstruction(uint8_t opcode);