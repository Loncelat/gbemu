#include "interrupts.h"

uint8_t IME = 0;

uint8_t CheckAndHandleInterrupts(void) {

    uint8_t FLAGS = INTERRUPT_FLAGS & INTERRUPT_ENABLE;

    // Controleer of er een interrupt is.
    if (!(FLAGS & 0x1F)) {
        return NO_INTR_DISPATCH;
    }

    cpu.halted = NOT_HALTED;
    
    if (!IME) {
        t_cycles += 4;
        return NO_INTR_DISPATCH;
    }

    // Clear de interrupt flag.
    INTERRUPT_FLAGS = 0xE0;
    WriteStack(registers.pc);
    IME = 0;
    
    // Vertical Blank
    if (FLAGS & VBL_INTERRUPT) {
        registers.pc = 0x0040;
    } // LCD STAT
    else if (FLAGS & STAT_INTERRUPT) {
        registers.pc = 0x0048;
    } // Timer
    else if (FLAGS & TIMER_INTERRUPT) {
        registers.pc = 0x0050;
    } // Serial input
    else if (FLAGS & SERIAL_INTERRUPT) {
        registers.pc = 0x0058;
    } // Joypad input
    else if (FLAGS & JOYPAD_INTERRUPT) {
        registers.pc = 0x0060;
    }

    return INTR_DISPATCH;
}