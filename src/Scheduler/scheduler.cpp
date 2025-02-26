#include "scheduler.h"

Scheduler::Scheduler(MMU *mmu) {
    this->mmu = mmu;
    DIV = TIMA = TMA = TAC = 0;
    timer_cycles = divider_cycles = 0;
}

void Scheduler::increment(uint8_t cycles) {
    timer_cycles += cycles;
    timer_cycles %= 4194304; // Gameboy Ticks
    while (DIV >= 256) {
        DIV -= 256;
        mmu->DIV++;
    }

    if (mmu->TAC & 0x04) {
        TIMA += cycles;
        int threshold = 0;
        switch (mmu->TAC & 0x03) {
            case 0:
                threshold = 1024;
                break;
            case 1:
                threshold = 16;
                break;
            case 2:
                threshold = 64;
                break;
            case 3:
                threshold = 256;
                break;
        }
        while (TIMA >= threshold) {
            TIMA -= threshold;
            if (mmu->TIMA == 0xFF) {
                mmu->TIMA = mmu->read_byte(0xFF06);
                // interrupts->set_interrupt_flag(INTERRUPT_TIMER);
            } else {
                mmu->TIMA++;
            }
        }
    }
}