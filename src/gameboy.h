#pragma once

#include "structs.h"

#include "CPU/CPU.h"
#include "PPU/PPU.h"
#include "MMU/MMU.h"
#include "Scheduler/scheduler.h"
#include "Cartridge/cartridge.h"

class Gameboy {
    Cartridge *cartridge;

    public:
        CPU *cpu;
        MMU *mmu;
        Scheduler *scheduler;
        PPU *ppu;
        Gameboy(Cartridge* cartridge);
        void step();
};