#include "structs.h"

#include "CPU/CPU.h"
#include "PPU/PPU.h"
#include "MMU/MMU.h"
#include "Scheduler/scheduler.h"
#include "Cartridge/cartridge.h"
#include "Render/render.h"

bool isValidROMFile(const std::string& filename) {
    return filename.size() > 3 && filename.substr(filename.size() - 3) == ".gb";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_rom_file>" << std::endl;
        return -1;
    }

    std::string directory = argv[1];
    if (!isValidROMFile(directory)) {
        std::cerr << "Error: The file must have a .gb extension" << std::endl;
        return -1;
    }

    Cartridge cartridge(directory);
    MMU mmu(&cartridge);
    Scheduler scheduler(&mmu);
    CPU cpu(&mmu, &scheduler);
    PPU ppu(&cpu, &mmu);
    Renderer renderer(&cpu, &ppu, &mmu);

    std::cout << "Would you like to have debug mode? Type y if so." << std::endl;
    bool debug = false;
    char debug_check = getchar();
    if (debug_check == 'y') {
        debug = true;
        mmu.set_debug();
        std::cout << "Debug mode activated!" << std::endl;
    } else {
        std::cout << "Debug mode not activated!" << std::endl;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Would you like to have a puase after every instruction? Type y if so. \nIf you do enable pausing, to close the application, use ctrl+c or fn+c depending on system." << std::endl;
    bool pause = false;
    char pause_check = getchar();

    if (pause_check == 'y') {
        pause = true;
        std::cout << "Pausing enabled!" << std::endl;
    }
    else {
        std::cout << "Pausing not enabled!" << std::endl;
    }

    if (debug == true) {
        cartridge.info();
    }

    renderer.init("Gameboy Emulator", 640, 480);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int cycles = 0;
        bool check = cpu.checkInterrupts();
        if (check) {
            cycles = 20;
        } else {
            if (cpu.halted) {
                cycles = 4;
            } else {
                uint8_t opcode = mmu.read_byte(cpu.PC);
                if (debug) {
                    std::cout << "Opcode: " << std::hex << (int)opcode << std::endl;
                }

                if (!mmu.trigger_halted) {
                    cpu.PC++;
                }
                cycles = cpu.getCycles(opcode);
                cpu.executeInstruction(opcode);
            }
        }

        scheduler.increment(cycles);
        ppu.step(cycles);
        renderer.render();
        
        if (debug) {
            cpu.info();
            mmu.info();
        }
        if (pause == true){
            getchar();
        }
    }

    return 0;
}
