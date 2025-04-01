#include "gameboy.h"

Gameboy::Gameboy(Cartridge* cartridge) {
    this->cartridge = cartridge;
    mmu = new MMU(cartridge);
    ppu = new PPU(cpu, mmu);
    scheduler = new Scheduler(mmu);
    cpu = new CPU(mmu, scheduler);
    renderer = new Renderer(cpu, ppu, mmu);

    mmu->rom_disabled = true;
    renderer->init("Gameboy Emulator", 640, 480);
}

void Gameboy::step() {
    int cycles = 0;
    bool check = cpu->checkInterrupts();
    if (check == true) {
        std::cout << "Interrupts triggered" << std::endl;
        cycles = 20;
    }
    else {
        if (cpu->halted) {
            cycles = 4;
            return;
        }

        uint8_t opcode = cpu->mmu->read_byte(cpu->PC);
        // std::cout << "PC: "<< std::hex << cpu->PC << "\n" << "Opcode: " << std::hex << opcode << std::endl;
        if (!mmu->trigger_halted) {
            cpu->PC++;
        }
        cycles = cpu->getCycles(opcode);
        std::cout << std::hex << (int)opcode << std::endl;
        //"\n" << cycles << std::endl;
        cpu->executeInstruction(opcode);
    }
    scheduler->increment(cycles);
    ppu->step(cycles);
    renderer->render();
    std::cout << "PC: " << std::hex << cpu->PC << std::endl;  
    cpu->printRegisters();
    mmu->info();
    // scheduler->info();
    getchar();
}