#pragma once
#include "MMU/MMU.h"

class Scheduler {
    MMU* mmu;
    int DIV, TIMA, TMA, TAC; // Timer Registers
    int timer_cycles, divider_cycles; // Timer & Divider Cycles
    public:
        Scheduler(MMU *mmu);
        void increment(uint8_t cycles);
        void info();
};