#pragma once

#ifdef _WIN32

#include <stdlib.h>
#include <windows.h>

#else

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#endif


#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "memory.h"
#include "mbc.h"
#include "gpu.h"

#define HEADER_LENGTH 0x14F
#define HEADER_NAME_OFFSET 0x134
#define HEADER_CTYPE_OFFSET 0x147
#define HEADER_ROMSIZE_OFFSET 0x148
#define HEADER_RAMSIZE_OFFSET 0x149

uint8_t LoadRom(char **argv);
void InitMemory();
void InitMBCType(uint8_t cartridgeType, char *romname, char **argv);