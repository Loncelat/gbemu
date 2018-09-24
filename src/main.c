#include "main.h"

void gbemu_cleanup();

uint8_t ShutdownRequested = 0;

/*
    De emulator werkt per m_cycle.
    1 m_cycle = 4 t_cycles. Er zijn 2^20 = 1.048.576 m_cycles per seconde.
*/

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Pass a rom file.\n");
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL ging kaduuk: %s", SDL_GetError());
        return 1;
    }

    if (SetupVideo()) { gbemu_cleanup(); return 1; }
    if (LoadRom(argv[1])) { gbemu_cleanup(); return 1; }
    //SetupAudio();

    ResetCPU();

    vsyncStartTime = SDL_GetPerformanceCounter();
    
    for (;;) {
        
        if (ShutdownRequested) {
            gbemu_cleanup(); return 0;
        }

        if (!cpu.stopped) {
            CPUCycle();
            gpuCycle(t_cycles);
            //AudioCycle(t_cycles);
            UpdateTimer(t_cycles);
        }
        else { gbemu_cleanup(); return 0; }
    }
    return 0;
}

void gbemu_cleanup(void) {

    if (mbc.battery) {
        rewind(mbc.save);
        fwrite(sram, 1, mbc.ramsize, mbc.save);
        fclose(mbc.save);
    }

    free(rom);
    free(sram);

    SDL_DestroyRenderer(LCDRenderer);
    SDL_DestroyTexture(LCD);
    SDL_DestroyWindow(window);
    SDL_Quit();
}