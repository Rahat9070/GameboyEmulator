#include "gameboy.h"

Gameboy::Gameboy(Cartridge* cartridge) {
    this->cartridge = cartridge;
    mmu = new MMU(cartridge);
    ppu = new PPU(cpu, mmu);
    scheduler = new Scheduler(mmu);
    cpu = new CPU(mmu, scheduler);
    renderer = new Renderer(cpu, ppu, mmu);

    renderer->init("Gameboy Emulator", 640, 480);
}

void Gameboy::step() {
    int cycles = 0;
    if (cpu->checkInterrupts() == true) {
        cpu->handleInterrupts();
        cycles = 20;
    }
    else {
        uint8_t opcode = cpu->mmu->read_byte(cpu->PC++);
        // std::cout << "PC: "<< std::hex << cpu->PC << "\n" << "Opcode: " << std::hex << opcode << std::endl;

        cycles = cpu->getCycles(opcode);
        cpu->executeInstruction(opcode);
    }

    scheduler->increment(cycles);
    ppu->step(cycles);
    renderer->render();    
    // cpu->printRegisters();
    // mmu->info();
}