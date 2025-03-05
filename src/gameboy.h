#pragma once

#include "structs.h"

#include "CPU/CPU.h"
#include "MMU/MMU.h"
#include "Scheduler/scheduler.h"
#include "Cartridge/cartridge.h"

class Gameboy {
    Cartridge *cartridge;
    CPU *cpu;
    MMU *mmu;
    Scheduler *scheduler;

    public:
        Gameboy(Cartridge* cartridge);
        void step();
};