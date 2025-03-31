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
    bool check = cpu->checkInterrupts();
    if (check == true) {
        cycles = 20;
    }
    else {
        uint8_t opcode = cpu->mmu->read_byte(cpu->PC);
        // std::cout << "PC: "<< std::hex << cpu->PC << "\n" << "Opcode: " << std::hex << opcode << std::endl;

        cycles = cpu->getCycles(opcode);
        std::cout << std::hex << (int)opcode << std::endl;
        //"\n" << cycles << std::endl;
        cpu->executeInstruction(opcode);
    }
    cpu->PC += 1;
    scheduler->increment(cycles);
    ppu->step(cycles);
    renderer->render();
    std::cout << "PC: " << std::hex << cpu->PC << std::endl;  
    cpu->printRegisters();
    mmu->info();
    // scheduler->info();
    getchar();
}