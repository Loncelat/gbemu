#include "timer.h"

uint16_t cpuClocks = 0x0000;
struct timer timer = {
    0x0000, &io[0x05], &io[0x06], 0, 0
};

uint16_t timerClocksCount[] = { 1024, 16, 64, 256, };

void WriteTimerControl(uint8_t data) {
    timer.enabled = (data & 0x04);
    timer.frequency = (data & 0x03);
}

void UpdateTimer(uint8_t cycles) {
    
    timer.div += cycles;
    cpuClocks += cycles;
    
    while (timer.enabled && cpuClocks >= timerClocksCount[timer.frequency]) {
        
        cpuClocks -= timerClocksCount[timer.frequency];
        *timer.tima += 1;

        // Als er een overflow was.
        if (*timer.tima == 0x00) {
            *timer.tima = *timer.tma;
            REQ_INTERRUPT(TIMER_INTERRUPT);
        }        
        
    }
}