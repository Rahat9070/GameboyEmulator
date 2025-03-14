#include "gameboy.h"

Gameboy::Gameboy(Cartridge* cartridge) {
    this->cartridge = cartridge;
    mmu = new MMU(cartridge);
    ppu = new PPU(cpu, mmu);
    scheduler = new Scheduler(mmu);
    cpu = new CPU(*mmu, *scheduler);
    renderer = new Renderer(cpu, ppu, mmu);

    mmu->load_game_rom(cartridge->name);
    renderer->init("Gameboy Emulator", 640, 480);
}

void Gameboy::step() {
    int cycles = 0;
    if (cpu->checkInterrupts()) {
        cpu->handleInterrupts();
        cycles = 20;
    }
    else {
        cycles = cpu->getCycles(cpu->mmu->read_byte(cpu->PC++));
        cpu->executeInstruction(cpu->mmu->read_byte(cpu->PC));
    }
    scheduler->increment(cycles);
    ppu->step(cycles);
    renderer->render();    
}
