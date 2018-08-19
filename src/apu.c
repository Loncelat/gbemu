#include "apu.h"

SDL_AudioSpec audioSpec, obtainedSpec;
SDL_AudioDeviceID dev;

int32_t queue[2];

int SetupAudio(void) {
    audioSpec.freq = APU_AUDIO_FREQUENCY;
    audioSpec.format = AUDIO_S32SYS;
	audioSpec.channels = 2;
    audioSpec.samples = APU_AUDIO_SAMPLES;

    dev = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &obtainedSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    SDL_PauseAudioDevice(dev, 0);
    return 0;
}

void AudioCycle(uint8_t cycles) {
    
}