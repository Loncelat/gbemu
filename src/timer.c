#include "timer.h"

struct timer timer = {
    &io[IO_TIMA], &io[IO_TMA], 0x0000, 0x0000, 0, 0
};

const uint16_t timerClocksCount[] = { 1024, 16, 64, 256, };

void WriteTimerControl(uint8_t data) {
    timer.enabled = (data & 0x04);
    timer.frequency = (data & 0x03);
}

void UpdateTimer(uint8_t cycles) {
    
    timer.div += cycles;
    timer.cycles += cycles;
    
    while (timer.enabled && timer.cycles >= timerClocksCount[timer.frequency]) {
        
        timer.cycles -= timerClocksCount[timer.frequency];
        *timer.tima += 1;

        // Als er een overflow was.
        if (*timer.tima == 0x00) {
            *timer.tima = *timer.tma;
            REQ_INTERRUPT(TIMER_INTERRUPT);
        }        
        
    }
}