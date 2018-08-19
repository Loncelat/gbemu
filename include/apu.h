#pragma once

#include <stdint.h>
#include <stdio.h>
#include "SDL2/SDL.h"

#define APU_AUDIO_FREQUENCY (44100)
#define APU_AUDIO_SAMPLES (1024)
#define APU_SAMPLE_FREQUENCY ((uint32_t) (CPU_FREQUENCY / (APU_FREQUENCY * 4)))

int SetupAudio(void);
void AudioCycle(uint8_t cycles);