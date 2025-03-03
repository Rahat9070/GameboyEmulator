#include "gameboy.h"

Gameboy::Gameboy(Cartridge* cartridge) {
    this->cartridge = cartridge;
    mmu = new MMU(cartridge);
    scheduler = new Scheduler(mmu);
    cpu = new CPU(*mmu, *scheduler);
}

void Gameboy::step() {
    cpu->step();
}